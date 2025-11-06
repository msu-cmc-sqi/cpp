#pragma once
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

struct AiConfig {
    std::string host;
    std::string port = "443";
    std::string api_key;
};

struct Context {
    std::string dialog;
    std::string table;
};

class AiAgent {
public:

    AiAgent();

    void addMessageToDialog(std::string& message);

    // Загрузить конфиг (host, port, api_key) из JSON-файла
    bool loadConfig(const std::string& path, std::string* err = nullptr);

    // Загрузить промпт из JSON-файла (принимает либо строку, либо объект с ключом "prompt")
    bool loadPrompt(const std::string& path, std::string* err = nullptr);

    // Выполнить запрос и вернуть распарсенный "text" из ответа
    // Возвращает std::nullopt при ошибке (описание в outErr, если передан)
    std::optional<std::string> ask(std::string* outErr = nullptr) const;

    // Явно задать промпт программно (не из файла)
    void setPrompt(std::string p) { prompt_ = std::move(p); }

    void conversation(std::string* err = nullptr);

    nlohmann::json createFullJson(std::string* err = nullptr);

private:
    // ---- низкоуровневые помощники ----
    static std::optional<std::string> httpsPostGenerate(
        const AiConfig& cfg, const std::string& jsonBody, std::string* err);

    // Простой разбор JSON: ожидаем { "text": "<строка>" }
    static std::string extractTextFromJsonBody(const std::string& body);

    static bool readWholeFile(const std::string& path, std::string& out, std::string* err);


private:
    AiConfig cfg_;
    std::string prompt_;
    Context context_;
};
