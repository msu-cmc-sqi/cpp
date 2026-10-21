#include "client.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

namespace lesson05 {

using nlohmann::json;

Config load_config(const std::string& filepath) {
    Config config;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        if (filepath != "config.example.json") {
            std::ifstream fallback("config.example.json");
            if (fallback.is_open()) {
                return load_config("config.example.json");
            }
        }
        return config;
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& err) {
        throw std::runtime_error("Ошибка разбора JSON-конфигурации: " + std::string(err.what()));
    }

    if (j.contains("server_url") && j["server_url"].is_string()) {
        config.server_url = j["server_url"].get<std::string>();
    }
    if (j.contains("model_alias") && j["model_alias"].is_string()) {
        config.model_alias = j["model_alias"].get<std::string>();
    }
    if (j.contains("timeout_seconds") && j["timeout_seconds"].is_number_integer()) {
        config.timeout_seconds = j["timeout_seconds"].get<long>();
    }
    if (j.contains("max_tokens") && j["max_tokens"].is_number_integer()) {
        config.max_tokens = j["max_tokens"].get<int>();
    }

    return config;
}

CurlRuntime::CurlRuntime() {
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        throw std::runtime_error("Не удалось инициализировать libcurl");
    }
}

CurlRuntime::~CurlRuntime() {
    curl_global_cleanup();
}

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

} // namespace

Response send_chat_completion(const Config& config, const std::string& prompt) {
    // Сериализация через JSON-библиотеку защищает от переносов строк и кавычек
    json request_body = {
        {"model", config.model_alias},
        {"messages", json::array({
            {{"role", "user"}, {"content", prompt}}
        })},
        {"max_tokens", config.max_tokens},
        {"stream", false}
    };
    std::string payload = request_body.dump();

    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> connection(
        curl_easy_init(), curl_easy_cleanup);
    if (!connection) {
        throw std::runtime_error("Не удалось создать HTTP-клиент curl");
    }

    std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> headers(
        curl_slist_append(nullptr, "Content-Type: application/json"),
        curl_slist_free_all);
    if (!headers) {
        throw std::runtime_error("Не удалось сформировать HTTP-заголовки");
    }

    std::string response_body;
    char details[CURL_ERROR_SIZE] = {};
    auto* curl = connection.get();
    std::string url = config.server_url + "/v1/chat/completions";

    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, details);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers.get());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, config.timeout_seconds);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

    const auto result = curl_easy_perform(curl);
    if (result == CURLE_OPERATION_TIMEDOUT) {
        throw std::runtime_error("Истёк тайм-аут HTTP-запроса (" +
                                 std::to_string(config.timeout_seconds) + " сек.)");
    }
    if (result == CURLE_COULDNT_CONNECT) {
        throw std::runtime_error("Сервер недоступен. Проверьте запуск llama-server по адресу " +
                                 config.server_url);
    }
    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Ошибка HTTP-запроса: ") +
                                 (details[0] != '\0' ? details : curl_easy_strerror(result)));
    }

    long status_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
    if (status_code != 200) {
        throw std::runtime_error("Сервер вернул HTTP " + std::to_string(status_code) +
                                 ": " + response_body);
    }

    json parsed;
    try {
        parsed = json::parse(response_body);
    } catch (const json::parse_error& err) {
        throw std::runtime_error("Ответ сервера не является валидным JSON: " + std::string(err.what()));
    }

    const char* error_msg = "В ответе отсутствует choices[0].message.content или неверен его тип";
    if (!parsed.is_object() || !parsed.contains("choices") || !parsed["choices"].is_array() ||
        parsed["choices"].empty()) {
        throw std::runtime_error(error_msg);
    }

    const auto& first_choice = parsed["choices"].at(0);
    if (!first_choice.is_object() || !first_choice.contains("message") ||
        !first_choice["message"].is_object() || !first_choice["message"].contains("content") ||
        !first_choice["message"]["content"].is_string()) {
        throw std::runtime_error(error_msg);
    }

    Response response;
    response.text = first_choice["message"]["content"].get<std::string>();

    if (parsed.contains("usage") && parsed["usage"].is_object()) {
        const auto& usage = parsed["usage"];
        if (usage.contains("prompt_tokens") && usage["prompt_tokens"].is_number_integer()) {
            response.prompt_tokens = usage["prompt_tokens"].get<int>();
        }
        if (usage.contains("completion_tokens") && usage["completion_tokens"].is_number_integer()) {
            response.completion_tokens = usage["completion_tokens"].get<int>();
        }
        if (usage.contains("total_tokens") && usage["total_tokens"].is_number_integer()) {
            response.total_tokens = usage["total_tokens"].get<int>();
        }
    }

    return response;
}

} // namespace lesson05
