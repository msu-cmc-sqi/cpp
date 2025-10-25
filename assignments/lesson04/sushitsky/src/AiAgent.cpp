#include "AiAgent.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

#include <iostream> //CLI
#include <algorithm> //CLI

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

using nlohmann::json;

//Конструктор и деструктор
AiAgent::AiAgent() {
#ifdef NO_SQLITE
    context_enabled_ = false;
#else
    db_ = nullptr;
    context_enabled_ = false;
    
    // Устанавливаем абсолютный путь к базе данных в текущей директории
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        db_path_ = std::string(cwd) + "/chat_context.db";
    } else {
        db_path_ = "chat_context.db";  // fallback
    }
    std::cout << "Database path: " << db_path_ << std::endl;
#endif
}

AiAgent::~AiAgent() {
    closeDatabase();
}

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



// ========== НОВЫЕ МЕТОДЫ ДЛЯ РАБОТЫ С КОНТЕКСТОМ ==========



bool AiAgent::initDatabase() {
    if (sqlite3_open(db_path_.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << \
            std::endl;
        return false;
    }
    return createTables();
}

bool AiAgent::createTables() {
    const char* sql = "CREATE TABLE IF NOT EXISTS chat_history ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "session_id TEXT NOT NULL,"
        "role TEXT NOT NULL,"
        "content TEXT NOT NULL,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_session ON chat_history(session_id);"
        "CREATE INDEX IF NOT EXISTS idx_timestamp ON chat_history(timestamp);";
    
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

void AiAgent::closeDatabase() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool AiAgent::enableContext(const std::string& session_id) {
#ifdef NO_SQLITE
    std::cout << "Context features disabled: SQLite3 not available" << std::endl;
    return false;
#else
    current_session_ = session_id.empty() ? "default" : session_id;
    
    // Закрываем предыдущее соединение если было
    closeDatabase();
    
    if (!initDatabase()) {
        std::cerr << "Failed to initialize database for context" << std::endl;
        return false;
    }
    
    context_enabled_ = true;
    std::cout << "✓ Context enabled for session: " << current_session_ << std::endl;
    
    // Проверяем, есть ли предыдущая история
    auto history = getContextHistory(5);
    if (!history.empty()) {
        std::cout << "✓ Loaded " << history.size() << " previous messages" << std::endl;
    }
    
    return true;
#endif
}

bool AiAgent::disableContext() {
    context_enabled_ = false;
    closeDatabase();
    std::cout << "Context disabled" << std::endl;
    return true;
}

bool AiAgent::saveToContext(const std::string& role, const std::string& content) {
#ifdef NO_SQLITE
    return false;
#else
    if (!context_enabled_ || !db_) {
        std::cerr << "Context not enabled or database not initialized" << std::endl;
        return false;
    }

    const char* sql = "INSERT INTO chat_history (session_id, role, content) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, current_session_.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, role.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_STATIC);
    
    int result = sqlite3_step(stmt);
    bool success = (result == SQLITE_DONE);
    
    if (!success) {
        std::cerr << "Failed to save to context: " << sqlite3_errmsg(db_) << std::endl;
    }
    
    sqlite3_finalize(stmt);
    return success;
#endif
}

std::vector<ChatMessage> AiAgent::getContextHistory(int limit) const {
    std::vector<ChatMessage> history;
#ifdef NO_SQLITE
    return history;
#else
    if (!context_enabled_ || !db_) return history;

    const char* sql = "SELECT role, content, timestamp FROM chat_history WHERE session_id = ? ORDER BY timestamp DESC LIMIT ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return history;
    }
    
    sqlite3_bind_text(stmt, 1, current_session_.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, limit);
    
    int step_result;
    while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW) {
        ChatMessage msg;
        const char* role_ptr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* content_ptr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* timestamp_ptr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        
        if (role_ptr) msg.role = role_ptr;
        if (content_ptr) msg.content = content_ptr;
        if (timestamp_ptr) msg.timestamp = timestamp_ptr;
        
        history.push_back(msg);
    }
    
    if (step_result != SQLITE_DONE) {
        std::cerr << "Error reading context: " << sqlite3_errmsg(db_) << std::endl;
    }
    
    sqlite3_finalize(stmt);
    
    // Переворачиваем чтобы получить в хронологическом порядке
    std::reverse(history.begin(), history.end());
    return history;
#endif
}

bool AiAgent::clearContext() {
    if (!context_enabled_ || !db_) return false;

    const char* sql = "DELETE FROM chat_history WHERE session_id = ?";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << \
            sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, current_session_.c_str(), -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    if (success) {
        std::cout << "Context cleared for session: " << current_session_ << \
            std::endl;
    }

    return success;
}

// ============== CLI ==================



std::string AiAgent::modeToString(CLIMode mode) {
    switch (mode) {
        case CLIMode::HELP: return "help";
        case CLIMode::TODO: return "todo";
        case CLIMode::TIMER: return "timer";
        case CLIMode::SUMMARY: return "summary";
        case CLIMode::IDEAS: return "ideas";
        case CLIMode::PLANNER: return "planner";
        default: return "default";
    }
}

AiAgent::CLIMode AiAgent::stringToMode(const std::string& mode_str) {
    std::string lower_str = mode_str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), \
    ::tolower);

    if (lower_str == "help") return CLIMode::HELP;
    if (lower_str == "todo") return CLIMode::TODO;
    if (lower_str == "timer") return CLIMode::TIMER;
    if (lower_str == "summary") return CLIMode::SUMMARY;
    if (lower_str == "ideas") return CLIMode::IDEAS;
    if (lower_str == "planner") return CLIMode::PLANNER;
    return CLIMode::DEFAULT;
}

void AiAgent::printCLIUsage() const {
    std::cout << "AI Agent CLI Helper - Универсальный командный помощник\n\n";
    std::cout << "Использование:\n";
    std::cout << "  ./ai_agent [команда]                    - обычный режим\n";
    std::cout << "  ./ai_agent --cli [команда]              - CLI режим с командой\n";
    std::cout << "  ./ai_agent --cli --mode <режим> [команда] - CLI с выбором режима\n";
    std::cout << "  ./ai_agent --cli                        - интерактивный CLI режим\n\n";
    std::cout << "Режимы:\n";
    std::cout << "  help    - справка по командам\n";
    std::cout << "  todo    - управление задачами\n";
    std::cout << "  timer   - таймеры и напоминания\n";
    std::cout << "  summary - суммаризация текста\n";
    std::cout << "  ideas   - генерация идей\n";
    std::cout << "  planner - планирование задач\n\n";
    std::cout << "Примеры:\n";
    std::cout << "  ./ai_agent --cli \"помоги настроить git\"\n";
    std::cout << "  ./ai_agent --cli --mode todo \"добавь задачу: подготовить отчет\"\n";
    std::cout << "  ./ai_agent --cli --mode summary --file document.txt\n";
}

std::string AiAgent::buildPromptForCommand(const std::string& command, CLIMode mode) const {
    std::string base_prompt = "Ты — универсальный CLI-помощник. Отвечай кратко, информативно и по делу. Адаптируй стиль под режим работы.";
    
    std::string mode_prompt;
    switch (mode) {
        case CLIMode::HELP: mode_prompt = "Объясни команды и их использование."; break;
        case CLIMode::TODO: mode_prompt = "Помоги управлять задачами: добавлять, удалять, просматривать."; break;
        case CLIMode::TIMER: mode_prompt = "Помоги с таймерами и напоминаниями."; break;
        case CLIMode::SUMMARY: mode_prompt = "Суммаризируй текст, выдели основные мысли."; break;
        case CLIMode::IDEAS: mode_prompt = "Предложи творческие идеи и решения."; break;
        case CLIMode::PLANNER: mode_prompt = "Помоги составить план действий."; break;
        default: mode_prompt = "Ответь полезно и информативно."; break;
    }
    
    // УЛУЧШЕННЫЙ формат контекста
    std::string context_str;
    if (context_enabled_) {
        auto history = getContextHistory(3); // берем только последние 3 сообщения для лучшего понимания
        if (!history.empty()) {
            context_str = "\n\nВАЖНО: Учитывай историю предыдущего разговора:\n";
            for (const auto& msg : history) {
                context_str += (msg.role == "user" ? "ПОЛЬЗОВАТЕЛЬ: " : "АССИСТЕНТ: ") + msg.content + "\n";
            }
            context_str += "\nТекущий запрос должен учитывать эту историю. Если пользователь ссылается на предыдущие сообщения, используй контекст для ответа.";
        }
    }
    
    return base_prompt + " " + mode_prompt + context_str + "\n\nТЕКУЩИЙ ЗАПРОС: " + command;
}

std::string AiAgent::readInputFile(const std::string& filepath) const {
    std::ifstream file(filepath);
    if (!file.is_open()) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::optional<std::string> AiAgent::executeCLICommand(const std::string& \
command, std::string* outErr) {
    if (command.empty()) {
        if (outErr) {
            *outErr = "Empty command";
        }
        return std::nullopt;
    }

    // Обработка файлового ввода для summary режима
    std::string final_command = command;
    if (cli_mode_ == CLIMode::SUMMARY && command.substr(0, 6) == "--file") {
        std::string filepath = command.substr(7);
        std::string file_content = readInputFile(filepath);
        if (!file_content.empty()) {
            final_command = "Суммаризируй текст:\n" + file_content; 
            std::cout << "Reading file: " << filepath << " (" << \
            file_content.length() << " chars)\n";
        }
        else {
            if (outErr) {
                *outErr = "Cannot read file or file is empty: " + filepath;
                return std::nullopt;
            }
        }
    }
 
    // Сохраняем оригинальный промпт
    std::string saved_prompt = prompt_;

    // Строим специализированный промпт
    prompt_ = buildPromptForCommand(final_command, cli_mode_);

    // Выполняем запрос
    auto result = ask(outErr);

    //Сохраняем в контекст если включено
    if (context_enabled_ && result) {
        saveToContext("user", final_command);
        saveToContext("assistant", *result);
    }

    // Восстанавливаем промпт
    prompt_ = saved_prompt;

    return result;
}

std::optional<std::string> AiAgent::processCLICommand(int argc, char* argv[], std::string* outErr) {
    // Обрабатываем флаги помощи
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printCLIUsage();
            return "Help displayed";
        }
    }

    // Если нет аргументов кроме --cli, переходим в интерактивный режим
    if (argc < 3) {
        runInteractiveMode();
        return "Interactive mode finished";
    }
    
    std::string command;
    std::string file_for_summary;
    
    // Парсим аргументы
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--mode" && i + 1 < argc) {
            setCLIMode(stringToMode(argv[i + 1]));
            i++; // пропускаем следующий аргумент
        } else if (arg == "--file" && i + 1 < argc) {
            file_for_summary = argv[i + 1];
            i++; // пропускаем следующий аргумент
        } else if (arg == "--enable-context") {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                enableContext(argv[i + 1]);
                i++;
            } else {
                enableContext();
            }
        } else if (arg == "--disable-context") {
            disableContext();
        } else if (arg == "--clear-context") {
            clearContext();
            return "Context cleared";
        } else if (arg == "--show-context") {
            auto history = getContextHistory();
            if (history.empty()) {
                return "No context history available";
            }
            std::string result = "Context history for session '" + current_session_ + "':\n";
            for (const auto& msg : history) {
                result += "[" + msg.timestamp + "] " + 
                         (msg.role == "user" ? "Пользователь" : "Ассистент") + 
                         ": " + msg.content + "\n";
            }
            return result;
        } else if (arg != "--cli" && arg != "--help" && arg != "-h") {
            if (!command.empty()) command += " ";
            command += arg;
        }
    }
    
    // Если есть файл для суммаризации, обрабатываем его
    if (!file_for_summary.empty() && cli_mode_ == CLIMode::SUMMARY) {
        std::string file_content = readInputFile(file_for_summary);
        if (!file_content.empty()) {
            command = "Суммаризируй следующий текст:\n" + file_content;
        } else {
            if (outErr) *outErr = "Cannot read file or file is empty: " + file_for_summary;
            return std::nullopt;
        }
    }
    
    if (!command.empty()) {
        return executeCLICommand(command, outErr);
    } else {
        runInteractiveMode();
        return "Interactive mode finished";
    }
}

void AiAgent::runInteractiveMode() {
    std::cout << "AI Agent CLI - Интерактивный режим\n";
    std::cout << "Команды: 'quit' - выход, 'help' - справка, 'mode <режим>' - смена режима\n";
    if (context_enabled_) {
        auto history = getContextHistory(5);
        std::cout << "✓ Контекст включен для сессии: " << current_session_;
        if (!history.empty()) {
            std::cout << " (" << history.size() << " сообщений в истории)";
        }
        std::cout << "\n";
    } else {
        std::cout << "ℹ Контекст отключен. Используйте 'enable-context' для включения.\n";
    }
    std::cout << std::endl;
    
    std::string input;
    while (true) {
        std::cout << "[" << modeToString(cli_mode_) << "] > ";
        std::getline(std::cin, input);
        
        if (input.empty()) continue;
        if (input == "quit" || input == "exit") break;
        if (input == "help") { 
            printCLIUsage(); 
            continue; 
        }
        if (input == "clear-context") {
            if (clearContext()) {
                std::cout << "✓ Контекст очищен\n";
            } else {
                std::cout << "✗ Ошибка очистки контекста\n";
            }
            continue;
        }
        if (input == "show-context") {
            auto history = getContextHistory();
            if (history.empty()) {
                std::cout << "История контекста пуста\n";
            } else {
                std::cout << "История контекста (" << history.size() << " сообщений):\n";
                for (const auto& msg : history) {
                    std::cout << "[" << msg.timestamp << "] " 
                             << (msg.role == "user" ? "Пользователь" : "Ассистент") 
                             << ": " << msg.content << "\n";
                }
            }
            continue;
        }
        if (input == "enable-context") {
            if (enableContext()) {
                std::cout << "✓ Контекст включен\n";
            } else {
                std::cout << "✗ Не удалось включить контекст\n";
            }
            continue;
        }
        if (input == "disable-context") {
            disableContext();
            std::cout << "✓ Контекст отключен\n";
            continue;
        }
        if (input.substr(0, 5) == "mode ") {
            setCLIMode(stringToMode(input.substr(5)));
            std::cout << "✓ Режим изменен на: " << modeToString(cli_mode_) << "\n";
            continue;
        }
        
        auto result = executeCLICommand(input, nullptr);
        if (result) {
            std::cout << *result << "\n";
        } else {
            std::cout << "✗ Ошибка выполнения команды\n";
        }
    }
    
    std::cout << "Интерактивный режим завершен.\n";
}