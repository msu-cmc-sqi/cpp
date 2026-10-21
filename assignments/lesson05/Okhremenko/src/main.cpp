#include "chat_types.hpp"
#include <cstdint>
#include <curl/curl.h>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <windows.h>


namespace lesson05 {

using nlohmann::json;
json build_request(const std::vector<Message> &history,
                   const std::string &model) {
    json messages = json::array();

    for (const auto &message : history) {
        messages.push_back(
            {{"role", message.role}, {"content", message.content}});
    }

    return {{"model", model},
            {"messages", messages},
            {"stream", false},
            {"max_tokens", MAX_TOKENS}};
}

// Инициализация libcurl живёт дольше всех HTTP-запросов.
struct CurlRuntime {
    CurlRuntime() {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
            throw std::runtime_error("Не удалось инициализировать libcurl");
        }
    }
    ~CurlRuntime() { curl_global_cleanup(); }
    CurlRuntime(const CurlRuntime &) = delete;
    CurlRuntime &operator=(const CurlRuntime &) = delete;
};

// Callback вызывается из C: исключения не должны выходить за его границу.
std::size_t append_response(char *data, std::size_t size, std::size_t count,
                            void *destination) noexcept {
    if (size != 0 && count > (std::numeric_limits<std::size_t>::max)() / size) {
        return 0;
    }
    const auto bytes = size * count;
    try {
        static_cast<std::string *>(destination)->append(data, bytes);
        return bytes;
    } catch (...) {
        return 0; // libcurl сообщит об ошибке записи ответа.
    }
}

void check_curl(CURLcode result) {
    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Ошибка libcurl: ") +
                                 curl_easy_strerror(result));
    }
}

std::string http_post(const std::string &url, const std::string &payload,
                      long timeout_seconds = 180) {
    // unique_ptr освобождает оба ресурса и при обычном выходе, и при
    // исключении.
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> connection(
        curl_easy_init(), curl_easy_cleanup);
    if (!connection) {
        throw std::runtime_error("Не удалось создать HTTP-соединение");
    }
    std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> headers(
        curl_slist_append(nullptr, "Content-Type: application/json"),
        curl_slist_free_all);
    if (!headers) {
        throw std::runtime_error("Не удалось создать HTTP-заголовок");
    }

    std::string response;
    char details[CURL_ERROR_SIZE] = {};
    auto *curl = connection.get();
    check_curl(curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, details));
    check_curl(curl_easy_setopt(curl, CURLOPT_URL, url.c_str()));
    check_curl(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers.get()));
    check_curl(curl_easy_setopt(curl, CURLOPT_POST, 1L));
    check_curl(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str()));
    check_curl(curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L));
    check_curl(curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_seconds));
    check_curl(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_response));
    check_curl(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response));

    // payload и response остаются живы до завершения запроса.
    const auto result = curl_easy_perform(curl);
    if (result == CURLE_OPERATION_TIMEDOUT) {
        throw std::runtime_error("Истёк тайм-аут HTTP-запроса к серверу");
    }
    if (result == CURLE_COULDNT_CONNECT) {
        throw std::runtime_error(
            "Сервер недоступен. Проверьте запуск llama-server и адрес " + url);
    }
    if (result != CURLE_OK) {
        throw std::runtime_error(
            std::string("Ошибка HTTP-запроса: ") +
            (details[0] != '\0' ? details : curl_easy_strerror(result)));
    }

    long status = 0;
    check_curl(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status));
    if (status != 200) {
        throw std::runtime_error("Сервер вернул HTTP " +
                                 std::to_string(status) + ": " + response);
    }
    return response;
}

std::string extract_text(const std::string &response) {
    json parsed;
    try {
        parsed = json::parse(response);
    } catch (const json::parse_error &) {
        throw std::runtime_error("Ответ сервера не является корректным JSON");
    }

    const char *error =
        "В ответе отсутствует choices[0].message.content или неверен его тип";
    if (!parsed.is_object() || !parsed.contains("choices")) {
        throw std::runtime_error(error);
    }
    const auto &choices = parsed.at("choices");
    if (!choices.is_array() || choices.empty() || !choices.at(0).is_object()) {
        throw std::runtime_error(error);
    }
    const auto &choice = choices.at(0);
    if (!choice.contains("message") || !choice.at("message").is_object()) {
        throw std::runtime_error(error);
    }
    const auto &message = choice.at("message");
    if (!message.contains("content") || !message.at("content").is_string()) {
        throw std::runtime_error(error);
    }
    return message.at("content").get<std::string>();
}

// Проверяем, что поле JSON содержит целое неотрицательное число.
std::uint64_t read_token_number(const json &object, const std::string &field) {
    if (!object.is_object() || !object.contains(field)) {
        throw std::runtime_error("Отсутствует поле JSON: " + field);
    }

    const auto &value = object.at(field);

    if (!value.is_number_integer() || value < 0) {
        throw std::runtime_error("Некорректное число токенов: " + field);
    }

    return value.get<std::uint64_t>();
}

// Узнаём размер всего запроса без генерации ответа.
std::uint64_t count_input_tokens(const json &request,
                                 const std::string &server_url) {
    const std::string response = http_post(
        server_url + "/v1/chat/completions/input_tokens", request.dump(), 10);

    const json result = json::parse(response);

    return read_token_number(result, "input_tokens");
}

// Статистика после генерации.
struct TokenUsage {
    std::uint64_t prompt;
    std::uint64_t completion;
    std::uint64_t total;
};

TokenUsage extract_usage(const json &response) {
    if (!response.is_object() || !response.contains("usage") ||
        !response.at("usage").is_object()) {
        throw std::runtime_error("В ответе сервера отсутствует объект usage");
    }

    const auto &usage = response.at("usage");

    return {read_token_number(usage, "prompt_tokens"),
            read_token_number(usage, "completion_tokens"),
            read_token_number(usage, "total_tokens")};
}
} // namespace lesson05

int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc > 3) {
        std::cerr << "Использование: local_chat.exe [server_url] [model]\n";
        return 1;
    }

    std::string server_url = argc >= 2 ? argv[1] : "http://127.0.0.1:8080";

    std::string model = argc >= 3 ? argv[2] : "local-model";

    if (server_url.empty() || model.empty()) {
        std::cerr << "Адрес сервера и модель не должны быть пустыми\n";
        return 1;
    }

    // Удаляем конечный / из адреса, если пользователь его указал.
    while (!server_url.empty() && server_url.back() == '/') {
        server_url.pop_back();
    }

    if (server_url.empty()) {
        std::cerr << "Некорректный адрес сервера\n";
        return 1;
    }
    try {
        const lesson05::CurlRuntime runtime;

        std::vector<lesson05::Message> history;

        std::cout << "Чат с локальной нейросетью\n";
        std::cout << "/reset - очистить историю\n";
        std::cout << "/exit  - выйти\n";

        while (true) {
            std::cout << "\nВы: " << std::flush;

            std::string text;

            if (!std::getline(std::cin, text)) {
                break;
            }

            if (text == "/exit") {
                break;
            }

            if (text == "/reset") {
                history.clear();
                std::cout << "История очищена.\n";
                continue;
            }

            if (text.find_first_not_of(" \t\r\n") == std::string::npos) {
                std::cout << "Введите непустое сообщение.\n";
                continue;
            }

            // Добавляем текущий вопрос.
            history.push_back({"user", text});

            std::size_t removed_pairs = 0;
            std::uint64_t input_tokens = 0;
            std::string answer;
            lesson05::TokenUsage usage{};

            try {
                // Проверяем размер истории перед генерацией.
                while (true) {
                    const auto request =
                        lesson05::build_request(history, model);

                    input_tokens =
                        lesson05::count_input_tokens(request, server_url);

                    if (input_tokens <=
                        lesson05::CONTEXT_LIMIT - lesson05::MAX_TOKENS) {
                        break;
                    }

                    // Остался только текущий вопрос.
                    if (history.size() == 1) {
                        break;
                    }

                    // Удаляем самую старую завершённую пару.
                    if (history.size() < 3 || history[0].role != "user" ||
                        history[1].role != "assistant") {
                        throw std::runtime_error("Нарушена структура истории");
                    }

                    history.erase(history.begin(), history.begin() + 2);

                    ++removed_pairs;
                }

                if (removed_pairs > 0) {
                    std::cout << "Удалено старых пар: " << removed_pairs
                              << '\n';
                }

                // Даже один вопрос может оказаться слишком длинным.
                if (input_tokens >
                    lesson05::CONTEXT_LIMIT - lesson05::MAX_TOKENS) {

                    history.pop_back();

                    std::cout << "Сообщение слишком длинное. "
                              << "Сократите его.\n";

                    continue;
                }

                // Запрос уже точно помещается в контекст.
                const auto request = lesson05::build_request(history, model);

                const auto response = lesson05::http_post(
                    server_url + "/v1/chat/completions", request.dump());
                const auto parsed = lesson05::json::parse(response);

                answer = lesson05::extract_text(response);
                usage = lesson05::extract_usage(parsed);

                // Сохраняем ответ только после успешной проверки.
                history.push_back({"assistant", answer});

            } catch (...) {
                // Убираем вопрос, на который не получили ответ.
                history.pop_back();
                throw;
            }

            std::cout << "Модель: " << answer << '\n';

            std::cout << "Вход: " << usage.prompt
                      << ", ответ: " << usage.completion
                      << ", всего: " << usage.total << " / "
                      << lesson05::CONTEXT_LIMIT << '\n';
        }

    } catch (const std::exception &error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }

    return 0;
}