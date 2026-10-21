#include "model_config.hpp"
#include <stdexcept>

namespace model_app {
    Config load_config(const std::string& config_json_str) {
        auto j = nlohmann::json::parse(config_json_str);
        Config cfg;

        if (j.contains("server_url") && j["server_url"].is_string()) {
            cfg.server_url = j["server_url"].get<std::string>();
        }

        if (j.contains("model") && j["model"].is_string()) {
            cfg.model = j["model"].get<std::string>();
        }

        if (j.contains("max_tokens") && j["max_tokens"].is_number_integer()) {
            cfg.max_tokens = j["max_tokens"].get<int>();
        }

        if (j.contains("temperature") && j["temperature"].is_number()) {
            cfg.temperature = j["temperature"].get<double>();
        }

        if (j.contains("timeout_seconds") && j["timeout_seconds"].is_number_integer()) {
            cfg.timeout_seconds = j["timeout_seconds"].get<long>();
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