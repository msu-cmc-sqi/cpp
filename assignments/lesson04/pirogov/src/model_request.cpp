#include "model_request.hpp"

#include <stdexcept>

namespace pirogov {

ModelConfig parse_config(const nlohmann::json& data) {
    if (!data.is_object()) {
        throw std::runtime_error("корень конфигурации должен быть объектом");
    }

    if (!data.contains("model")) {
        throw std::runtime_error("нет поля model");
    }
    if (!data.at("model").is_string()) {
        throw std::runtime_error("поле model должно быть строкой");
    }

    std::string model = data.at("model").get<std::string>();
    if (model.empty()) {
        throw std::runtime_error("поле model не должно быть пустым");
    }

    if (!data.contains("context_size")) {
        throw std::runtime_error("нет поля context_size");
    }
    if (!data.at("context_size").is_number_integer()) {
        throw std::runtime_error("поле context_size должно быть целым числом");
    }

    int context_size = 0;
    if (data.at("context_size").is_number_unsigned()) {
        unsigned long long value = data.at("context_size").get<unsigned long long>();
        if (value < 1 || value > 131072) {
            throw std::runtime_error("поле context_size должно быть от 1 до 131072");
        }
        context_size = static_cast<int>(value);
    } else {
        long long value = data.at("context_size").get<long long>();
        if (value < 1 || value > 131072) {
            throw std::runtime_error("поле context_size должно быть от 1 до 131072");
        }
        context_size = static_cast<int>(value);
    }

    if (!data.contains("temperature")) {
        throw std::runtime_error("нет поля temperature");
    }
    if (!data.at("temperature").is_number()) {
        throw std::runtime_error("поле temperature должно быть числом");
    }

    double temperature = data.at("temperature").get<double>();
    if (temperature < 0.0 || temperature > 2.0) {
        throw std::runtime_error("поле temperature должно быть от 0 до 2");
    }

    ModelConfig config;
    config.model = model;
    config.context_size = context_size;
    config.temperature = temperature;
    return config;
}

nlohmann::json make_request(
    const ModelConfig& config,
    const std::string& user_text
) {
    nlohmann::json message;
    message["role"] = "user";
    message["content"] = user_text;

    nlohmann::json request;
    request["model"] = config.model;
    request["context_size"] = config.context_size;
    request["temperature"] = config.temperature;
    request["messages"] = nlohmann::json::array();
    request["messages"].push_back(message);

    return request;
}

} // namespace pirogov
