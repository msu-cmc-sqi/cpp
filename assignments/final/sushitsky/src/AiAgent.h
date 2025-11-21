#pragma once
#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include <sqlite3.h>
#include <vector>

struct AiConfig {
    std::string model_type = "remote"; // "remote", "local_http", "local_lib"
    std::string host;
    std::string port = "443";
    std::string api_key;

    std::string local_http_host = "127.0.0.1";
    std::string local_http_port = "8080";
    std::string local_model_path;
    int local_model_n_ctx = 4096;
};

//Структура для хранения истории сообщений
struct ChatMessage {
    std::string role;    //"user" или "assistant"
    std::string content;
    std::string timestamp;
};

class AiAgent {
public:

    enum class CLIMode {   //Режимы работы CLI
        DEFAULT,
        HELP,
        TODO,
        TIMER,
        SUMMARY,
        IDEAS,
        PLANNER
    };

    AiAgent();
    ~AiAgent();

    // Загрузить конфиг (host, port, api_key) из JSON-файла
    bool loadConfig(const std::string& path, std::string* err = nullptr);

    // Загрузить промпт из JSON-файла (принимает либо строку, либо объект с ключом "prompt")
    bool loadPrompt(const std::string& path, std::string* err = nullptr);

    // Выполнить запрос и вернуть распарсенный "text" из ответа
    // Возвращает std::nullopt при ошибке (описание в outErr, если передан)
    std::optional<std::string> ask(std::string* outErr = nullptr) const;

    // Явно задать промпт программно (не из файла)
    void setPrompt(std::string p) { prompt_ = std::move(p); }

    //Новые методы для CLI
    std::optional<std::string> processCLICommand(int argc, char* argv[], \
    std::string* outErr = nullptr);
    void setCLIMode(CLIMode mode) {
        cli_mode_ = mode;
    }
    CLIMode getCLIMode() const {
        return cli_mode_;
    }
    static std::string modeToString(CLIMode mode);
    static CLIMode stringToMode(const std::string& mode_str);
    void printCLIUsage() const;
    void runInteractiveMode();

    //Методы для работы с контекстом/историей
    bool enableContext(const std::string& session_id = "default");
    bool disableContext();
    bool saveToContext(const std::string& role, const std::string& content);
    std::vector<ChatMessage> getContextHistory(int limit = 10) const;
    bool clearContext();
    std::string getCurrentSession() const { return current_session_; }

private:
    // ---- низкоуровневые помощники ----
    static std::optional<std::string> httpsPostGenerate(
        const AiConfig& cfg, const std::string& jsonBody, std::string* err);

    // Простой разбор JSON: ожидаем { "text": "<строка>" }
    static std::string extractTextFromJsonBody(const std::string& body);

    static bool readWholeFile(const std::string& path, std::string& out, std::string* err);

    //CLI
    std::string buildPromptForCommand(const std::string& command, \
        CLIMode mode) const;
    std::string readInputFile(const std::string& filepath) const;
    std::optional<std::string> executeCLICommand(const std::string& command, \
        std::string* outErr);

    //Методы для работы с SQLite
    bool initDatabase();
    bool createTables();
    void closeDatabase();

    //Local model
    static std::optional<std::string> localHttpPostGenerate(const AiConfig& cfg, const std::string& jsonBody, std::string* err);

private:
    AiConfig cfg_;
    std::string prompt_;

    //CLI
    CLIMode cli_mode_ = CLIMode::DEFAULT;
    std::string original_prompt_;

    //Контекст и база данных
    sqlite3* db_ = nullptr;
    bool context_enabled_ = false;
    std::string current_session_;
    std::string db_path_ = "chat_context.db";
};
