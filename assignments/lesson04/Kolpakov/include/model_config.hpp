#pragma once

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace model_request {

struct ModelConfig {
    std::string model;
    int         context_size = 2048;
    double      temperature  = 0.7;
};

// Три различимых класса ошибок на границе приложения.
class ConfigFileError      : public std::runtime_error { using std::runtime_error::runtime_error; };
class ConfigSyntaxError    : public std::runtime_error { using std::runtime_error::runtime_error; };
class ConfigStructureError : public std::runtime_error { using std::runtime_error::runtime_error; };

// Парсит текст JSON и валидирует структуру. Бросает ConfigSyntaxError или ConfigStructureError.
ModelConfig parse_config(const std::string& text);

// Валидирует уже распарсенный объект JSON.
ModelConfig config_from_json(const nlohmann::json& root);

// Строит запрос к chat API по конфигурации и тексту пользователя.
nlohmann::json build_request(const ModelConfig& cfg, const std::string& user_text);

} // namespace model_request
