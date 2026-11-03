#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace model_app {

struct Config {
    std::string server_url = "http://127.0.0.1:8080/v1/chat/completions";
    std::string model = "local-model";
    int max_tokens = 64;
    double temperature = 0.7;
    long timeout_seconds = 30;
};

Config load_config(const std::string& config_json_str); // загрузка и проверка конфигурационного json

// формирование json для POST-запроса
nlohmann::json create_chat_request(const Config& config, const std::string& user_prompt);

} // namespace model_app