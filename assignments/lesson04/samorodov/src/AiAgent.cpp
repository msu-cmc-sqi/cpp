#include "AiAgent.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

using nlohmann::json;

// --------- utils IO ----------
bool AiAgent::readWholeFile(const std::string& path, std::string& out, std::string* err) {
    std::ifstream f(path, std::ios::binary);
    if (!f) { if (err) *err = "Cannot open file: " + path; return false; }
    std::ostringstream ss; ss << f.rdbuf();
    out = std::move(ss).str();
    return true;
}

// --------- JSON loaders ----------
bool AiAgent::loadConfig(const std::string& path, std::string* err) {
    std::string s;
    if (!readWholeFile(path, s, err)) return false;
    try {
        auto j = json::parse(s);
        // обязательные поля через at(); port можно оставить как есть, если отсутствует
        cfg_.host   = j.at("host").get<std::string>();
        if (j.contains("port")) cfg_.port = j.at("port").get<std::string>();
        cfg_.api_key = j.at("api_key").get<std::string>();
        return true;
    } catch (const std::exception& e) {
        if (err) *err = std::string("Config parse error: ") + e.what();
        return false;
    }
}

bool AiAgent::loadPrompt(const std::string& path, std::string* err) {
    std::string s;
    if (!readWholeFile(path, s, err)) return false;
    try {
        // Допускаем, что файл — либо строка JSON, либо объект с ключом "prompt"
        json j = json::parse(s);
        if (j.is_string()) {
            prompt_ = j.get<std::string>();
        } else if (j.is_object()) {
            prompt_ = j.at("prompt").get<std::string>();
        } else {
            if (err) *err = "Prompt JSON must be string or object with key 'prompt'";
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        if (err) *err = std::string("Prompt parse error: ") + e.what();
        return false;
    }
}

// ------- Простейший разбор JSON: ожидаем { "text": "<строка>" } -------
std::string AiAgent::extractTextFromJsonBody(const std::string& body) {
    // Если вместе с HTTP-хедерами — отрежем их
    const auto p = body.find("\r\n\r\n");
    const std::string json_part = (p != std::string::npos) ? body.substr(p + 4) : body;

    try {
        auto j = json::parse(json_part);
        return j.at("text").get<std::string>();  // строго ожидаем поле "text"
    } catch (...) {
        return {};
    }
}

// -------- Низкоуровневый HTTPS POST на /api/generate --------
std::optional<std::string> AiAgent::httpsPostGenerate(
        const AiConfig& cfg, const std::string& jsonBody, std::string* err) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) { if (err) *err = "SSL_CTX_new failed"; return std::nullopt; }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { if (err) *err = "socket failed"; SSL_CTX_free(ctx); return std::nullopt; }

    struct addrinfo hints = {}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(cfg.host.c_str(), cfg.port.c_str(), &hints, &res) != 0) {
        if (err) *err = "getaddrinfo failed";
        close(sock); SSL_CTX_free(ctx); return std::nullopt;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        if (err) *err = "connect failed";
        freeaddrinfo(res); close(sock); SSL_CTX_free(ctx); return std::nullopt;
    }
    freeaddrinfo(res);

    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    if (SSL_connect(ssl) <= 0) {
        if (err) *err = "SSL_connect failed";
        SSL_free(ssl); close(sock); SSL_CTX_free(ctx); return std::nullopt;
    }

    // HTTP запрос
    std::ostringstream req;
    req << "POST /api/generate HTTP/1.1\r\n"
        << "Host: " << cfg.host << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Connection: close\r\n";
    if (!cfg.api_key.empty()) req << "x-api-key: " << cfg.api_key << "\r\n";
    req << "Content-Length: " << jsonBody.size() << "\r\n\r\n"
        << jsonBody;

    const std::string request_str = req.str();
    if (SSL_write(ssl, request_str.c_str(), (int)request_str.size()) <= 0) {
        if (err) *err = "SSL_write failed";
        SSL_free(ssl); close(sock); SSL_CTX_free(ctx); return std::nullopt;
    }

    char buf[4096];
    std::string response;
    int bytes;
    while ((bytes = SSL_read(ssl, buf, sizeof(buf)-1)) > 0) {
        buf[bytes] = '\0';
        response += buf;
    }

    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);

    // ----- Используем nlohmann::json для извлечения "text" -----
    std::string text = extractTextFromJsonBody(response);
    if (text.empty()) {
        if (err) *err = "Cannot extract \"text\" from JSON response";
        return std::nullopt;
    }
    return text;
}

std::optional<std::string> AiAgent::analyzeCodeFile(const std::string& filepath, 
                                                   const std::string& language,
                                                   std::string* err) {
    std::string code;
    if (!readWholeFile(filepath, code, err)) {
        return std::nullopt;
    }
    
    if (code.empty()) {
        if (err) *err = "File is empty: " + filepath;
        return std::nullopt;
    }
    
    return analyzeCodeString(code, language, err);
}

std::optional<std::string> AiAgent::analyzeCodeString(const std::string& code,
                                                     const std::string& language,
                                                     std::string* err) {
    if (cfg_.host.empty() || cfg_.api_key.empty()) {
        if (err) *err = "Config not loaded or api_key/host missing";
        return std::nullopt;
    }
    
    if (code.empty()) {
        if (err) *err = "Code string is empty";
        return std::nullopt;
    }
    
    std::string analysis_prompt = createAnalysisPrompt(code, language);
    
    // Формируем JSON запрос с промптом для анализа
    json payload = { {"prompt", analysis_prompt} };
    const std::string body = payload.dump();
    
    auto response = httpsPostGenerate(cfg_, body, err);
    if (!response) {
        return std::nullopt;
    }
    
    // Парсим и форматируем ответ
    auto issues = parseAnalysisResponse(*response);
    return formatAnalysisReport(issues);
}

std::string AiAgent::createAnalysisPrompt(const std::string& code, const std::string& language) const {
    std::ostringstream prompt;
    
    prompt << R"(Ты — опытный программист-аналитик. 
Твоя задача: проанализировать код на языке )" << language << R"( и выявить потенциальные проблеммы.
Формат ответа: строго в формате JSON массив объектов с полями:
- "type": "error"|"warning"|"suggestion"
- "message": описание проблемы/рекомендации
- "line": номер строки (если можно определить)
- "context": фрагмент кода (если уместно)

Контекст: )" << language << R"( код для анализа.
Ограничения: 
- Только конкретные технические проблемы (синтаксис, логика, безопасность, стиль)
- Без общих фраз и комплиментов
- Максимально кратко и по делу
- Приоритет критическим ошибкам

Код для анализа:
```)" << language << "\n" << code << "\n```";

    return prompt.str();
}

std::vector<CodeIssue> AiAgent::parseAnalysisResponse(const std::string& response) const {
    std::vector<CodeIssue> issues;
    
    try {
        // Пытаемся распарсить JSON ответ
        auto j = json::parse(response);
        
        if (j.is_array()) {
            for (const auto& item : j) {
                CodeIssue issue;
                issue.type = item.value("type", "");
                issue.message = item.value("message", "");
                issue.line = item.value("line", -1);
                issue.context = item.value("context", "");
                
                if (!issue.type.empty() && !issue.message.empty()) {
                    issues.push_back(issue);
                }
            }
        }
    } catch (const std::exception& e) {
        // Если не удалось распарсить JSON
        CodeIssue issue;
        issue.type = "info";
        issue.message = "JSON parsing failed" + response;
        issues.push_back(issue);
    }
    
    return issues;
}

std::string AiAgent::formatAnalysisReport(const std::vector<CodeIssue>& issues) const {
    if (issues.empty()) {
        return "✓ No issues found. Code looks good!\n";
    }
    
    std::ostringstream report;
    report << "Code Analysis Report:\n";
    report << "=====================\n\n";
    
    int error_count = 0, warning_count = 0, suggestion_count = 0;
    
    for (const auto& issue : issues) {
        if (issue.type == "error") error_count++;
        else if (issue.type == "warning") warning_count++;
        else if (issue.type == "suggestion") suggestion_count++;
        
        report << "[" << issue.type << "]";
        if (issue.line > 0) report << " Line " << issue.line;
        report << "\n";
        report << "Message: " << issue.message << "\n";
        if (!issue.context.empty()) {
            report << "Context: " << issue.context << "\n";
        }
        report << "---\n";
    }
    
    report << "\nSummary: " << error_count << " errors, " 
           << warning_count << " warnings, " << suggestion_count << " suggestions\n";
    
    return report.str();
}

std::optional<std::string> AiAgent::ask(std::string* outErr) const {
    if (cfg_.host.empty() || cfg_.api_key.empty()) {
        if (outErr) *outErr = "Config not loaded or api_key/host missing";
        return std::nullopt;
    }
    if (prompt_.empty()) {
        if (outErr) *outErr = "Prompt is empty (load it first)";
        return std::nullopt;
    }

    // Формируем корректный JSON тела через nlohmann/json
    json payload = { {"prompt", prompt_} };
    const std::string body = payload.dump();

    return httpsPostGenerate(cfg_, body, outErr);
}
