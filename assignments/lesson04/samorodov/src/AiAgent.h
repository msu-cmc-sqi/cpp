#pragma once
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

struct AiConfig {
    std::string host;
    std::string port = "443";
    std::string api_key;
};

struct CodeIssue {
    std::string type;       // "error", "warning", "suggestion"
    std::string message;
    int line = -1;          // -1 если неизвестно
    std::string context;    // фрагмент кода
};

class AiAgent {
public:
    // Загрузить конфиг (host, port, api_key) из JSON-файла
    bool loadConfig(const std::string& path, std::string* err = nullptr);

    // Загрузить промпт из JSON-файла (принимает либо строку, либо объект с ключом "prompt")
    bool loadPrompt(const std::string& path, std::string* err = nullptr);

    // Выполнить запрос и вернуть распарсенный "text" из ответа
    // Возвращает std::nullopt при ошибке (описание в outErr, если передан)
    std::optional<std::string> ask(std::string* outErr = nullptr) const;

    // Явно задать промпт программно (не из файла)
    void setPrompt(std::string p) { prompt_ = std::move(p); }
    
    // Анализ кода из файла
    std::optional<std::string> analyzeCodeFile(const std::string& filepath, 
                                              const std::string& language = "cpp",
                                              std::string* err = nullptr);
    
    // Анализ кода из строки
    std::optional<std::string> analyzeCodeString(const std::string& code,
                                                const std::string& language = "cpp",
                                                std::string* err = nullptr);
    
    // Установить язык по умолчанию для анализа
    void setDefaultLanguage(const std::string& lang) { default_language_ = lang; }

private:
    // ---- низкоуровневые помощники ----
    static std::optional<std::string> httpsPostGenerate(
        const AiConfig& cfg, const std::string& jsonBody, std::string* err);

    // Простой разбор JSON: ожидаем { "text": "<строка>" }
    static std::string extractTextFromJsonBody(const std::string& body);

    static bool readWholeFile(const std::string& path, std::string& out, std::string* err);

    std::string createAnalysisPrompt(const std::string& code, const std::string& language) const;
    std::string formatAnalysisReport(const std::vector<CodeIssue>& issues) const;
    std::vector<CodeIssue> parseAnalysisResponse(const std::string& response) const;

private:
    AiConfig cfg_;
    std::string prompt_;
    std::string default_language_ = "cpp";
};

