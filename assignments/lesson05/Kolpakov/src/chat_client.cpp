#include "chat_client.hpp"

#include <curl/curl.h>

#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

namespace lesson05 {

namespace {

std::size_t append_response(char* data, std::size_t size,
                            std::size_t count, void* destination) noexcept {
    if (size != 0 && count > std::numeric_limits<std::size_t>::max() / size) {
        return 0;
    }
    const auto bytes = size * count;
    try {
        static_cast<std::string*>(destination)->append(data, bytes);
        return bytes;
    } catch (...) {
        return 0;
    }
}

void check_curl(CURLcode result) {
    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Ошибка libcurl: ") +
                                 curl_easy_strerror(result));
    }
}

} // namespace

CurlRuntime::CurlRuntime() {
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        throw std::runtime_error("Не удалось инициализировать libcurl");
    }
}

CurlRuntime::~CurlRuntime() {
    curl_global_cleanup();
}

Config load_config(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Не удалось открыть конфигурацию: " + path);
    }
    json parsed;
    try {
        in >> parsed;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Ошибка разбора конфигурации: " + std::string(e.what()));
    }
    if (!parsed.is_object()) {
        throw std::runtime_error("Конфигурация должна быть JSON-объектом");
    }

    Config cfg;
    auto read_string = [&](const char* key, std::string& target) {
        if (parsed.contains(key)) {
            if (!parsed.at(key).is_string()) {
                throw std::runtime_error(std::string("Поле ") + key + " должно быть строкой");
            }
            target = parsed.at(key).get<std::string>();
        }
    };
    auto read_int = [&](const char* key, int& target, int min_value) {
        if (parsed.contains(key)) {
            if (!parsed.at(key).is_number_integer()) {
                throw std::runtime_error(std::string("Поле ") + key + " должно быть целым");
            }
            const auto value = parsed.at(key).get<long long>();
            if (value < min_value) {
                throw std::runtime_error(std::string("Поле ") + key +
                                         " должно быть не меньше " +
                                         std::to_string(min_value));
            }
            target = static_cast<int>(value);
        }
    };
    auto read_long = [&](const char* key, long& target, long min_value) {
        if (parsed.contains(key)) {
            if (!parsed.at(key).is_number_integer()) {
                throw std::runtime_error(std::string("Поле ") + key + " должно быть целым");
            }
            const auto value = parsed.at(key).get<long long>();
            if (value < min_value) {
                throw std::runtime_error(std::string("Поле ") + key +
                                         " должно быть не меньше " +
                                         std::to_string(min_value));
            }
            target = static_cast<long>(value);
        }
    };

    read_string("server_url", cfg.server_url);
    read_string("model", cfg.model);
    read_int("context_size", cfg.context_size, 1);
    read_int("max_tokens", cfg.max_tokens, 1);
    read_long("connect_timeout_seconds", cfg.connect_timeout_seconds, 1);
    read_long("request_timeout_seconds", cfg.request_timeout_seconds, 1);
    read_long("token_timeout_seconds", cfg.token_timeout_seconds, 1);
    read_string("system_prompt", cfg.system_prompt);

    if (cfg.max_tokens >= cfg.context_size) {
        throw std::runtime_error(
            "max_tokens должен быть меньше context_size: иначе ответ не поместится");
    }
    return cfg;
}

json build_request(const std::vector<Message>& history, const Config& config) {
    json messages = json::array();
    for (const auto& message : history) {
        messages.push_back({{"role", message.role}, {"content", message.content}});
    }
    return {
        {"model", config.model},
        {"messages", messages},
        {"stream", false},
        {"max_tokens", config.max_tokens}
    };
}

std::string http_post(const std::string& url,
                      const std::string& payload,
                      long connect_timeout_seconds,
                      long total_timeout_seconds) {
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
    auto* curl = connection.get();
    check_curl(curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, details));
    check_curl(curl_easy_setopt(curl, CURLOPT_URL, url.c_str()));
    check_curl(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers.get()));
    check_curl(curl_easy_setopt(curl, CURLOPT_POST, 1L));
    check_curl(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str()));
    check_curl(curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,
                                static_cast<long>(payload.size())));
    check_curl(curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout_seconds));
    check_curl(curl_easy_setopt(curl, CURLOPT_TIMEOUT, total_timeout_seconds));
    check_curl(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_response));
    check_curl(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response));

    const auto result = curl_easy_perform(curl);
    if (result == CURLE_OPERATION_TIMEDOUT) {
        throw std::runtime_error("Истёк тайм-аут HTTP-запроса к серверу");
    }
    if (result == CURLE_COULDNT_CONNECT) {
        throw std::runtime_error(
            "Сервер недоступен. Проверьте запуск llama-server и адрес " + url);
    }
    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Ошибка HTTP-запроса: ") +
            (details[0] != '\0' ? details : curl_easy_strerror(result)));
    }

    long status = 0;
    check_curl(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status));
    if (status != 200) {
        throw std::runtime_error("Сервер вернул HTTP " + std::to_string(status) +
                                 ": " + response);
    }
    return response;
}

long count_input_tokens(const Config& config, const json& request) {
    const std::string url = config.server_url + "/v1/chat/completions/input_tokens";
    const std::string body = http_post(url, request.dump(),
                                       config.connect_timeout_seconds,
                                       config.token_timeout_seconds);
    json parsed;
    try {
        parsed = json::parse(body);
    } catch (const json::parse_error&) {
        throw std::runtime_error("Ответ подсчёта токенов не является корректным JSON");
    }
    if (!parsed.is_object() || !parsed.contains("input_tokens")) {
        throw std::runtime_error(
            "В ответе подсчёта токенов отсутствует поле input_tokens");
    }
    const auto& value = parsed.at("input_tokens");
    if (!value.is_number_integer()) {
        throw std::runtime_error("input_tokens не является целым числом");
    }
    const auto raw = value.get<long long>();
    if (raw < 0) {
        throw std::runtime_error("input_tokens отрицателен");
    }
    return static_cast<long>(raw);
}

std::string extract_text(const std::string& response) {
    json parsed;
    try {
        parsed = json::parse(response);
    } catch (const json::parse_error&) {
        throw std::runtime_error("Ответ сервера не является корректным JSON");
    }
    const char* error =
        "В ответе отсутствует choices[0].message.content или неверен его тип";
    if (!parsed.is_object() || !parsed.contains("choices")) {
        throw std::runtime_error(error);
    }
    const auto& choices = parsed.at("choices");
    if (!choices.is_array() || choices.empty() || !choices.at(0).is_object()) {
        throw std::runtime_error(error);
    }
    const auto& choice = choices.at(0);
    if (!choice.contains("message") || !choice.at("message").is_object()) {
        throw std::runtime_error(error);
    }
    const auto& message = choice.at("message");
    if (!message.contains("content") || !message.at("content").is_string()) {
        throw std::runtime_error(error);
    }
    return message.at("content").get<std::string>();
}

Usage extract_usage(const std::string& response) {
    json parsed;
    try {
        parsed = json::parse(response);
    } catch (const json::parse_error&) {
        throw std::runtime_error("Ответ сервера не является корректным JSON");
    }
    if (!parsed.is_object() || !parsed.contains("usage") ||
        !parsed.at("usage").is_object()) {
        throw std::runtime_error("В ответе отсутствует объект usage");
    }
    const auto& usage = parsed.at("usage");
    auto read_field = [&](const char* name) -> long {
        if (!usage.contains(name)) {
            throw std::runtime_error(std::string("В usage отсутствует поле ") + name);
        }
        const auto& value = usage.at(name);
        if (!value.is_number_integer()) {
            throw std::runtime_error(std::string("usage.") + name +
                                     " не является целым числом");
        }
        const auto raw = value.get<long long>();
        if (raw < 0) {
            throw std::runtime_error(std::string("usage.") + name + " отрицателен");
        }
        return static_cast<long>(raw);
    };
    Usage result;
    result.prompt_tokens = read_field("prompt_tokens");
    result.completion_tokens = read_field("completion_tokens");
    result.total_tokens = read_field("total_tokens");
    return result;
}

bool remove_oldest_pair(std::vector<Message>& history) {
    std::size_t start = 0;
    if (!history.empty() && history.front().role == "system") {
        start = 1;
    }
    for (std::size_t i = start; i + 1 < history.size(); ++i) {
        if (history[i].role == "user" && history[i + 1].role == "assistant") {
            const auto first =
                history.begin() + static_cast<std::vector<Message>::difference_type>(i);
            history.erase(first, first + 2);
            return true;
        }
    }
    return false;
}

} // namespace lesson05
