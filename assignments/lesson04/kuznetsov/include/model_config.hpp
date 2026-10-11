#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace model_app {
    // объявляем функцию для проверки текста конфига
    nlohmann::json create_api_request(const std::string& config_json_str, const std::string& user_prompt);
}