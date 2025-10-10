#pragma once
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

struct AiConfig {
    std::string host;
    std::string port = "443";
    std::string api_key;
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

private:
    AiConfig cfg_;
    std::string prompt_;

    //CLI
    CLIMode cli_mode_ = CLIMode::DEFAULT;
    std::string original_prompt_;
};
