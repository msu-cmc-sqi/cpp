#include "model_config.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace model_request;

namespace {

void expect_structure_error(const std::string& text) {
    try {
        parse_config(text);
    } catch (const ConfigStructureError&) {
        return;
    }
    throw std::runtime_error("ожидалась ConfigStructureError для: " + text);
}

void expect_syntax_error(const std::string& text) {
    try {
        parse_config(text);
    } catch (const ConfigSyntaxError&) {
        return;
    }
    throw std::runtime_error("ожидалась ConfigSyntaxError для: " + text);
}

} // namespace

int main() {
    // 1) Корректная конфигурация.
    const auto cfg = parse_config(R"({
        "model": "local-model.gguf",
        "context_size": 2048,
        "temperature": 0.7
    })");
    assert(cfg.model == "local-model.gguf");
    assert(cfg.context_size == 2048);
    assert(cfg.temperature == 0.7);

    // 2) Пропущенное поле.
    expect_structure_error(R"({"model":"m","context_size":1024})");
    expect_structure_error(R"({"model":"m","temperature":0.5})");

    // 3) Неверный тип: дробный context_size, null в model, строка вместо числа.
    expect_structure_error(R"({"model":"m","context_size":1024.5,"temperature":0.5})");
    expect_structure_error(R"({"model":null,"context_size":1024,"temperature":0.5})");
    expect_structure_error(R"({"model":"m","context_size":"big","temperature":0.5})");

    // 4) Число вне диапазона.
    expect_structure_error(R"({"model":"m","context_size":0,"temperature":0.5})");
    expect_structure_error(R"({"model":"m","context_size":1024,"temperature":3.0})");
    expect_structure_error(R"({"model":"m","context_size":1024,"temperature":-0.1})");

    // 5) Пустая строка модели.
    expect_structure_error(R"({"model":"","context_size":1024,"temperature":0.5})");

    // 6) Синтаксическая ошибка JSON.
    expect_syntax_error("{ not json }");

    // 7) Структура построенного запроса.
    const auto req = build_request(cfg, "Привет");
    assert(req.is_object());
    assert(req.at("model") == "local-model.gguf");
    assert(req.at("messages").is_array());
    assert(req.at("messages").size() == 1);
    assert(req.at("messages")[0].at("role")    == "user");
    assert(req.at("messages")[0].at("content") == "Привет");

    std::cout << "все тесты прошли\n";
    return 0;
}
