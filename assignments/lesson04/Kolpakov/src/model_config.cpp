#include "model_config.hpp"

namespace model_request {

namespace {
constexpr int    kContextSizeMin   = 1;
constexpr int    kContextSizeMax   = 131072;
constexpr double kTemperatureMin   = 0.0;
constexpr double kTemperatureMax   = 2.0;
} // namespace

ModelConfig config_from_json(const nlohmann::json& root) {
    if (!root.is_object()) {
        throw ConfigStructureError("корень конфигурации должен быть JSON-объектом");
    }

    for (const char* key : {"model", "context_size", "temperature"}) {
        if (!root.contains(key)) {
            throw ConfigStructureError(std::string("отсутствует обязательное поле: ") + key);
        }
    }

    if (!root.at("model").is_string()) {
        throw ConfigStructureError("поле 'model' должно быть строкой");
    }
    const std::string model = root.at("model").get<std::string>();
    if (model.empty()) {
        throw ConfigStructureError("поле 'model' не должно быть пустым");
    }

    if (!root.at("context_size").is_number_integer()) {
        throw ConfigStructureError("поле 'context_size' должно быть целым числом");
    }
    const int context_size = root.at("context_size").get<int>();
    if (context_size < kContextSizeMin || context_size > kContextSizeMax) {
        throw ConfigStructureError("поле 'context_size' вне диапазона [" +
                                   std::to_string(kContextSizeMin) + ", " +
                                   std::to_string(kContextSizeMax) + "]");
    }

    if (!root.at("temperature").is_number()) {
        throw ConfigStructureError("поле 'temperature' должно быть числом");
    }
    const double temperature = root.at("temperature").get<double>();
    if (temperature < kTemperatureMin || temperature > kTemperatureMax) {
        throw ConfigStructureError("поле 'temperature' вне диапазона [0, 2]");
    }

    ModelConfig cfg;
    cfg.model        = model;
    cfg.context_size = context_size;
    cfg.temperature  = temperature;
    return cfg;
}

ModelConfig parse_config(const std::string& text) {
    nlohmann::json root;
    try {
        root = nlohmann::json::parse(text);
    } catch (const nlohmann::json::parse_error& e) {
        throw ConfigSyntaxError(std::string("некорректный синтаксис JSON: ") + e.what());
    }
    return config_from_json(root);
}

nlohmann::json build_request(const ModelConfig& cfg, const std::string& user_text) {
    return nlohmann::json{
        {"model",       cfg.model},
        {"temperature", cfg.temperature},
        {"messages", nlohmann::json::array({
            {{"role", "user"}, {"content", user_text}}
        })}
    };
}

} // namespace model_request
