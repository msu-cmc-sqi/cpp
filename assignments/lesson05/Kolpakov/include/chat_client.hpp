#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace lesson05 {

using json = nlohmann::json;

struct Message {
    std::string role;
    std::string content;
};

struct Config {
    std::string server_url = "http://127.0.0.1:8080";
    std::string model = "local-model";
    int context_size = 1024;
    int max_tokens = 64;
    long connect_timeout_seconds = 5;
    long request_timeout_seconds = 180;
    long token_timeout_seconds = 10;
    std::string system_prompt;
};

struct Usage {
    long prompt_tokens = 0;
    long completion_tokens = 0;
    long total_tokens = 0;
};

// RAII: глобальная инициализация libcurl живёт дольше всех запросов.
class CurlRuntime {
public:
    CurlRuntime();
    ~CurlRuntime();
    CurlRuntime(const CurlRuntime&) = delete;
    CurlRuntime& operator=(const CurlRuntime&) = delete;
};

Config load_config(const std::string& path);

// Сериализация истории через JSON-библиотеку — кавычки и переводы строк не ломают запрос.
json build_request(const std::vector<Message>& history, const Config& config);

// HTTP POST + проверка статуса. Бросает std::runtime_error с понятным сообщением.
std::string http_post(const std::string& url,
                      const std::string& payload,
                      long connect_timeout_seconds,
                      long total_timeout_seconds);

// POST /v1/chat/completions/input_tokens -> input_tokens (неотрицательное целое).
long count_input_tokens(const Config& config, const json& request);

// Извлечение choices[0].message.content с проверкой типов.
std::string extract_text(const std::string& response);

// Извлечение usage.{prompt,completion,total}_tokens с проверкой типов.
Usage extract_usage(const std::string& response);

// Удаляет самую старую завершённую пару user–assistant (system сохраняется).
// Возвращает true, если пара была удалена, и false, если завершённых пар нет.
bool remove_oldest_pair(std::vector<Message>& history);

} // namespace lesson05
