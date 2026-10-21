#include "client.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        std::string config_file = "config.json";
        std::string prompt;

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "-c" || arg == "--config") && i + 1 < argc) {
                config_file = argv[++i];
            } else if (arg == "-h" || arg == "--help") {
                std::cout << "Использование: " << argv[0] << " [ОПЦИИ] [ТЕКСТ_ЗАПРОСА]\n"
                          << "Опции:\n"
                          << "  -c, --config <путь>   Путь к файлу конфигурации (по умолчанию config.example.json)\n"
                          << "  -h, --help            Показать эту справку\n";
                return 0;
            } else if (prompt.empty()) {
                prompt = arg;
            }
        }

        // Если запрос не передан в аргументах командной строки, запрашиваем из stdin
        if (prompt.empty()) {
            std::cout << "Ваш запрос: ";
            if (!std::getline(std::cin, prompt) || prompt.find_first_not_of(" \t\r\n") == std::string::npos) {
                std::cerr << "Ошибка: Введите непустую строку запроса\n";
                return 1;
            }
        }

        const lesson05::CurlRuntime curl_runtime;
        const lesson05::Config config = lesson05::load_config(config_file);

        const auto response = lesson05::send_chat_completion(config, prompt);

        std::cout << "\nОтвет модели:\n" << response.text << "\n";
        if (response.total_tokens > 0) {
            std::cout << "\n[Токены: вход: " << response.prompt_tokens
                      << ", ответ: " << response.completion_tokens
                      << ", всего: " << response.total_tokens << "]\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
