#include "model_request.hpp"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void require_config_error(const std::function<void()>& action,
                          const std::string& test_name) {
    try {
        action();
    } catch (const model_request::ConfigError&) {
        return;
    }
    throw std::runtime_error(test_name + ": expected ConfigError");
}

void test_valid_config() {
    const auto config = model_request::parse_config({
        {"model", "qwen2.5:3b"},
        {"context_size", 4096},
        {"temperature", 0.7}
    });

    require(config.model == "qwen2.5:3b", "model was parsed incorrectly");
    require(config.context_size == 4096, "context_size was parsed incorrectly");
    require(config.temperature == 0.7, "temperature was parsed incorrectly");
}

void test_missing_field() {
    require_config_error([] {
        model_request::parse_config({
            {"model", "qwen2.5:3b"},
            {"context_size", 4096}
        });
    }, "missing field");
}

void test_wrong_type() {
    require_config_error([] {
        model_request::parse_config({
            {"model", "qwen2.5:3b"},
            {"context_size", "4096"},
            {"temperature", 0.7}
        });
    }, "wrong type");
}

void test_value_out_of_range() {
    require_config_error([] {
        model_request::parse_config({
            {"model", "qwen2.5:3b"},
            {"context_size", 0},
            {"temperature", 0.7}
        });
    }, "context_size out of range");

    require_config_error([] {
        model_request::parse_config({
            {"model", "qwen2.5:3b"},
            {"context_size", 4096},
            {"temperature", 2.1}
        });
    }, "temperature out of range");
}

void test_invalid_json_syntax() {
    require_config_error([] {
        model_request::parse_config_text(R"({"model": "qwen2.5:3b",})");
    }, "invalid JSON syntax");
}

void test_request_structure() {
    const model_request::ModelConfig config{"qwen2.5:3b", 4096, 0.7};
    const nlohmann::json request =
        model_request::build_request(config, "Hello!");

    require(request.is_object(), "request must be an object");
    require(request.at("model") == "qwen2.5:3b", "wrong request model");
    require(request.at("messages").is_array(), "messages must be an array");
    require(request.at("messages").size() == 1, "expected one message");
    require(request.at("messages").at(0).at("role") == "user",
            "wrong message role");
    require(request.at("messages").at(0).at("content") == "Hello!",
            "wrong message content");
    require(request.at("options").at("num_ctx") == 4096,
            "wrong context size in request");
    require(request.at("options").at("temperature") == 0.7,
            "wrong temperature in request");
    require(request.at("stream") == false, "stream must be disabled");
}

}  // namespace

int main() {
    try {
        test_valid_config();
        test_missing_field();
        test_wrong_type();
        test_value_out_of_range();
        test_invalid_json_syntax();
        test_request_structure();
    } catch (const std::exception& error) {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All tests passed\n";
    return 0;
}
