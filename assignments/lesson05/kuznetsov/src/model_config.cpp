#include "model_config.hpp"
#include <stdexcept>

namespace model_app {
    Config load_config(const std::string& config_json_str) {
        auto j = nlohmann::json::parse(config_json_str);
        Config cfg;

        if (j.contains("server_url")) {
            if (!j["server_url"].is_string()){
                throw std::invalid_argument("field 'server_url' is incorrect");
            }

            cfg.server_url = j["server_url"].get<std::string>();
        }

        if (j.contains("model")) {
            if (!j["model"].is_string()){
                throw std::invalid_argument("field 'model' is incorrect");
            }

            cfg.model = j["model"].get<std::string>();
        }

        if (j.contains("max_tokens")) {
            if (!j["max_tokens"].is_number_integer()){
                throw std::invalid_argument("field 'max_tokens' must be integer");
            }

            cfg.max_tokens = j["max_tokens"].get<int>();

            if (cfg.max_tokens < 0){
                throw std::invalid_argument("field 'max_tokens' must be positive");
            }
        }

        if (j.contains("temperature") && j["temperature"].is_number()) {
            if (!j["temperature"].is_number()){
                throw std::invalid_argument("field 'temperature' must be double");
            }

            cfg.temperature = j["temperature"].get<double>();
        }

        if (j.contains("timeout_seconds")) {
            if (!j["timeout_seconds"].is_number_integer()){
                throw std::invalid_argument("field 'timeout_seconds' must be integer");
            }

            cfg.timeout_seconds = j["timeout_seconds"].get<long>();
            
            if (cfg.timeout_seconds < 0){
                throw std::invalid_argument("field 'timeout_seconds' must be positive or 0");
            }
        }

        return cfg;
    }

    nlohmann::json create_chat_request(const Config& config, const std::string& user_prompt) {
        if (user_prompt.empty()) {
            throw std::invalid_argument("User prompt cannot be empty.");
        }

        nlohmann::json messages = nlohmann::json::array();

        // системный промпт: роль - помощник по C++
        messages.push_back({
            { "role", "system" },
            { "content", "You are an expert C++ assistant. Help the user fix compilation errors, explain C++ code snippets concisely, and follow modern C++ best practices." }
            });

        // промпт пользователя
        messages.push_back({
            { "role", "user" },
            { "content", user_prompt }
            });

        nlohmann::json request_body = {
            { "model", config.model },
            { "messages", messages },
            { "max_tokens", config.max_tokens },
            { "temperature", config.temperature },
            { "stream", false }
        };

        return request_body;
    }

} // namespace model_app