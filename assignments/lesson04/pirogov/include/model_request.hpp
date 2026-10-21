#pragma once

#include <string>

#include <nlohmann/json.hpp>

namespace pirogov {

struct ModelConfig {
    std::string model;
    int context_size;
    double temperature;
};

ModelConfig parse_config(const nlohmann::json& data);

nlohmann::json make_request(
    const ModelConfig& config,
    const std::string& user_text
);

} // namespace pirogov
