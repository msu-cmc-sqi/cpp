#include "model_request.hpp"

#include <cmath>
#include <cstdint>
#include <string>

namespace model_request {
namespace {

constexpr int kMinContextSize = 1;
constexpr int kMaxContextSize = 131072;
constexpr double kMinTemperature = 0.0;
constexpr double kMaxTemperature = 2.0;

const nlohmann::json& required_field(const nlohmann::json& json,
                                     const char* field_name) {
    const auto iterator = json.find(field_name);
    if (iterator == json.end()) {
        throw ConfigError("missing required field: " + std::string(field_name));
    }
    return *iterator;
}

}  // namespace

ModelConfig parse_config(const nlohmann::json& json) {
    if (!json.is_object()) {
        throw ConfigError("configuration root must be a JSON object");
    }

    const auto& model_json = required_field(json, "model");
    if (!model_json.is_string()) {
        throw ConfigError("field 'model' must be a string");
    }
    const std::string model = model_json.get<std::string>();
    if (model.empty()) {
        throw ConfigError("field 'model' must not be empty");
    }

    const auto& context_size_json = required_field(json, "context_size");
    if (!context_size_json.is_number_integer() &&
        !context_size_json.is_number_unsigned()) {
        throw ConfigError("field 'context_size' must be an integer");
    }

    const bool context_size_in_range = context_size_json.is_number_unsigned()
        ? context_size_json.get<std::uint64_t>() <=
              static_cast<std::uint64_t>(kMaxContextSize)
        : context_size_json.get<std::int64_t>() >= kMinContextSize &&
              context_size_json.get<std::int64_t>() <= kMaxContextSize;
    if (!context_size_in_range) {
        throw ConfigError("field 'context_size' must be in range [1, 131072]");
    }
    const int context_size = context_size_json.get<int>();

    const auto& temperature_json = required_field(json, "temperature");
    if (!temperature_json.is_number()) {
        throw ConfigError("field 'temperature' must be a number");
    }
    const double temperature = temperature_json.get<double>();
    if (!std::isfinite(temperature) || temperature < kMinTemperature ||
        temperature > kMaxTemperature) {
        throw ConfigError("field 'temperature' must be in range [0, 2]");
    }

    return {model, context_size, temperature};
}

ModelConfig parse_config_text(const std::string& text) {
    try {
        return parse_config(nlohmann::json::parse(text));
    } catch (const nlohmann::json::parse_error& error) {
        throw ConfigError("invalid JSON syntax: " + std::string(error.what()));
    }
}

nlohmann::json build_request(const ModelConfig& config,
                             const std::string& user_text) {
    return {
        {"model", config.model},
        {"messages", nlohmann::json::array({
            {{"role", "user"}, {"content", user_text}}
        })},
        {"options", {
            {"num_ctx", config.context_size},
            {"temperature", config.temperature}
        }},
        {"stream", false}
    };
}

}  // namespace model_request
