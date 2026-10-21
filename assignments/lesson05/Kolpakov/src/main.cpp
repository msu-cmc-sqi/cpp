#include "chat_client.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::string trim(const std::string& s) {
    const auto start = s.find_first_not_of(" \t\r");
    if (start == std::string::npos) {
        return {};
    }
    const auto end = s.find_last_not_of(" \t\r");
    return s.substr(start, end - start + 1);
}

void print_help(const lesson05::Config& config) {
    std::cerr
        << "Чат с локальной моделью через llama-server.\n"
        << "  сервер:    " << config.server_url << "\n"
        << "  модель:    " << config.model << "\n"
        << "  контекст:  " << config.context_size << " токенов\n"
        << "  ответ до:  " << config.max_tokens << " токенов\n"
        << "Команды: /reset — очистить историю, /quit — выход.\n";
}

// Один ход диалога. При ошибке бросает исключение,
// но перед этим гарантирует, что в истории не останется
// неподтверждённой текущей реплики пользователя.
void process_turn(const lesson05::Config& config,
                  std::vector<lesson05::Message>& history,
                  const std::string& input) {
    history.push_back({"user", input});

    // 1. Подсчёт токенов и при необходимости — сокращение истории.
    long input_tokens = 0;
    int removed_pairs = 0;
    try {
        auto request = lesson05::build_request(history, config);
        input_tokens = lesson05::count_input_tokens(config, request);
        while (input_tokens + config.max_tokens > config.context_size) {
            if (!lesson05::remove_oldest_pair(history)) {
                throw std::runtime_error(
                    "Запрос не помещается в контекст: вход "
                    + std::to_string(input_tokens) + ", ответ до "
                    + std::to_string(config.max_tokens) + ", контекст "
                    + std::to_string(config.context_size)
                    + ". Сократите ввод.");
            }
            ++removed_pairs;
            request = lesson05::build_request(history, config);
            input_tokens = lesson05::count_input_tokens(config, request);
        }
    } catch (...) {
        if (!history.empty() && history.back().role == "user") {
            history.pop_back();
        }
        throw;
    }

    if (removed_pairs > 0) {
        std::cerr << "Сокращена история: удалено пар — " << removed_pairs << "\n";
    }

    // 2. Генерация.
    const auto request = lesson05::build_request(history, config);
    const std::string gen_url = config.server_url + "/v1/chat/completions";
    std::string body;
    try {
        body = lesson05::http_post(gen_url, request.dump(),
                                   config.connect_timeout_seconds,
                                   config.request_timeout_seconds);
    } catch (...) {
        if (!history.empty() && history.back().role == "user") {
            history.pop_back();
        }
        throw;
    }

    // 3. Разбор ответа.
    std::string answer;
    lesson05::Usage usage;
    try {
        answer = lesson05::extract_text(body);
        usage = lesson05::extract_usage(body);
    } catch (...) {
        if (!history.empty() && history.back().role == "user") {
            history.pop_back();
        }
        throw;
    }

    history.push_back({"assistant", answer});

    std::cout << answer << "\n";
    std::cerr << "Вход: " << usage.prompt_tokens
              << ", ответ: " << usage.completion_tokens
              << ", всего: " << usage.total_tokens
              << " / " << config.context_size << "\n";
}

} // namespace

int main(int argc, char** argv) {
    std::string config_path = (argc > 1) ? argv[1] : "config.json";

    lesson05::Config config;
    try {
        config = lesson05::load_config(config_path);
    } catch (const std::exception& error) {
        std::cerr << "Ошибка конфигурации: " << error.what() << "\n";
        return 1;
    }

    std::vector<lesson05::Message> history;
    if (!config.system_prompt.empty()) {
        history.push_back({"system", config.system_prompt});
    }

    try {
        const lesson05::CurlRuntime runtime;

        print_help(config);

        std::string line;
        while (true) {
            std::cerr << "> " << std::flush;
            if (!std::getline(std::cin, line)) {
                std::cerr << "\n";
                break;
            }
            const auto input = trim(line);
            if (input.empty()) {
                continue;
            }
            if (input == "/quit" || input == "/exit") {
                break;
            }
            if (input == "/reset") {
                history.clear();
                if (!config.system_prompt.empty()) {
                    history.push_back({"system", config.system_prompt});
                }
                std::cerr << "История очищена.\n";
                continue;
            }

            // Ошибка одного хода не должна ломать сессию:
            // сообщение выводится, история остаётся согласованной.
            try {
                process_turn(config, history, input);
            } catch (const std::exception& error) {
                std::cerr << "Ошибка: " << error.what() << "\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Критическая ошибка: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
