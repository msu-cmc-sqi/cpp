#pragma once

#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

namespace model_request {

struct ModelConfig {
    std::string model;
    int context_size;
    double temperature;
};

class ConfigError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

ModelConfig parse_config(const nlohmann::json& json);
ModelConfig parse_config_text(const std::string& text);

nlohmann::json build_request(const ModelConfig& config,
                             const std::string& user_text);

}  // namespace model_request
