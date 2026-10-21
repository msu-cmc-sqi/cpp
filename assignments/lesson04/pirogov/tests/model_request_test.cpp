#include "model_request.hpp"

#include <exception>
#include <iostream>
#include <string>

void check(bool condition, const std::string& test_name, int& errors) {
    if (!condition) {
        std::cerr << "Тест не пройден: " << test_name << '\n';
        errors++;
    }
}

bool config_has_error(const nlohmann::json& data) {
    try {
        pirogov::parse_config(data);
    } catch (const std::exception&) {
        return true;
    }
    return false;
}

int main() {
    int errors = 0;

    nlohmann::json valid = {
        {"model", "local-model.gguf"},
        {"context_size", 2048},
        {"temperature", 0.7}
    };

    pirogov::ModelConfig config = pirogov::parse_config(valid);
    check(config.model == "local-model.gguf", "корректная модель", errors);
    check(config.context_size == 2048, "корректный размер контекста", errors);
    check(config.temperature == 0.7, "корректная температура", errors);

    nlohmann::json missing_field = valid;
    missing_field.erase("model");
    check(config_has_error(missing_field), "пропущенное поле", errors);

    nlohmann::json wrong_type = valid;
    wrong_type["context_size"] = "2048";
    check(config_has_error(wrong_type), "неверный тип", errors);

    nlohmann::json wrong_range = valid;
    wrong_range["temperature"] = 3.0;
    check(config_has_error(wrong_range), "число вне диапазона", errors);

    nlohmann::json request = pirogov::make_request(config, "Привет");
    check(request["model"] == "local-model.gguf", "модель в запросе", errors);
    check(request["messages"].is_array(), "messages является массивом", errors);
    check(request["messages"].size() == 1, "одно сообщение", errors);
    check(request["messages"][0]["role"] == "user", "роль пользователя", errors);
    check(request["messages"][0]["content"] == "Привет", "текст пользователя", errors);

    if (errors == 0) {
        std::cout << "Все тесты пройдены\n";
    }

    return errors;
}
