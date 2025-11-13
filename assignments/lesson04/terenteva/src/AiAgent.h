#pragma once
#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include "TrackInfo.h"

struct AiConfig {
    std::string host;
    std::string port = "443";
    std::string api_key;
};

class AiAgent {
public:
    // Загрузить конфиг (host, port, api_key) из JSON-файла
    bool loadConfig(const std::string& path, std::string* err = nullptr);

    // Загрузить промпт из JSON-файла (принимает либо строку, либо объект с ключом "prompt")
    bool loadPrompt(const std::string& path, std::string* err = nullptr);

    // Выполнить запрос и вернуть распарсенный "text" из ответа
    // Возвращает std::nullopt при ошибке (описание в outErr, если передан)
    std::optional<std::vector<std::string>> generateMusic(const std::string &mood_description,
                                                            std::string *outErr = nullptr);
    // Явно задать промпт программно (не из файла)
    void setPrompt(std::string p) { prompt_ = std::move(p); }
    std::optional<bool> printResult(const std::vector<TrackInfo> &tracks, const std::string &mood, std::string *outErr);

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
};