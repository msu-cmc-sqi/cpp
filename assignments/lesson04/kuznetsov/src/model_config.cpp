#include "model_config.hpp"
#include <stdexcept>

namespace model_app {
    nlohmann::json create_api_request(const std::string& config_json_str, const std::string& user_prompt) {
        auto config = nlohmann::json::parse(config_json_str);

        if (!config.is_object()) {
            throw std::runtime_error("Config root must be a JSON object");
        }

        std::string model_name = config.at("model");
        if (model_name.empty()) {
            throw std::runtime_error("Field 'model' cannot be empty");
        }

        if (!config.contains("context_size") || !config.at("context_size").is_number_integer()) {
            throw std::runtime_error("Field 'context_size' must be an integer");
        }

        int context_size = config.at("context_size");
        if (context_size <= 0) {
            throw std::runtime_error("Field 'context_size' must be positive");
        }

        if (!config.contains("temperature") || !config.at("temperature").is_number()) {
            throw std::runtime_error("Field 'temperature' must be a num ");
        }

        double temp = config.at("temperature");
        if (temp < 0.0 || temp > 2.0) {
            throw std::runtime_error("temperature must be in range [0.0, 2.0]");
        }

        nlohmann::json request = {
            {
                "model", model_name },
            {
                "options",
                {
                    { "context_size", context_size },
                    { "temperature", temp }
                }
        },
            {
                "messages", nlohmann::json::array({ { { "role", "user" },{ "content", user_prompt } } })
            }
        };

        return request;
    }
}