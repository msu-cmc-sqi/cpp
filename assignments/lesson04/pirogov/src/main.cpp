#include "model_request.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Использование: model_request <конфигурация.json> <текст>\n";
        return 1;
    }

    std::ifstream input(argv[1]);
    if (!input.is_open()) {
        std::cerr << "Ошибка файла: не удалось открыть " << argv[1] << '\n';
        return 2;
    }

    nlohmann::json data;
    try {
        input >> data;
    } catch (const nlohmann::json::parse_error& error) {
        std::cerr << "Ошибка синтаксиса JSON: " << error.what() << '\n';
        return 3;
    }

    try {
        pirogov::ModelConfig config = pirogov::parse_config(data);
        nlohmann::json request = pirogov::make_request(config, argv[2]);
        std::cout << request.dump(2) << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Ошибка конфигурации: " << error.what() << '\n';
        return 4;
    }

    return 0;
}
