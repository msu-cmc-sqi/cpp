#include "model_config.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

void test_valid_config() {
    std::string valid_json = R"({"model": "test-model", "context_size": 1024, "temperature": 0.5})";
    auto req = model_app::create_api_request(valid_json, "Hello");

    assert(req["model"] == "test-model");
    assert(req["messages"][0]["content"] == "Hello");

    std::cout << "test_valid_config passed!\n";
}

void test_invalid_temperature() {
    std::string invalid_json = R"({"model": "test-model", "context_size": 1024, "temperature": 5.0})";
    bool thrown = false;

    try {
        model_app::create_api_request(invalid_json, "Hello");
    }
    catch (const std::exception& e) {
        thrown = true;
    }

    assert(thrown);

    std::cout << "test_invalid_temperature passed!\n";
}

int main() {
    try {
        test_valid_config();
        test_invalid_temperature();

        std::cout << "All tests passed successfully\n";
    }
    catch (...) {
        std::cerr << "Tests failed\n";

        return 1;
    }
    
    return 0;
}