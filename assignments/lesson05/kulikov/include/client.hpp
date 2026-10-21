#pragma once

#include <string>

namespace lesson05 {

struct Config {
    std::string server_url = "http://127.0.0.1:8080";
    std::string model_alias = "local-model";
    long timeout_seconds = 60;
    int max_tokens = 64;
};

// Загрузка настроек из JSON-файла
Config load_config(const std::string& filepath);

struct Response {
    std::string text;
    int prompt_tokens = 0;
    int completion_tokens = 0;
    int total_tokens = 0;
};

// RAII-инициализация ресурсов libcurl
struct CurlRuntime {
    CurlRuntime();
    ~CurlRuntime();
    CurlRuntime(const CurlRuntime&) = delete;
    CurlRuntime& operator=(const CurlRuntime&) = delete;
};

// Отправка HTTP-запроса к llama-server и получение результата
Response send_chat_completion(const Config& config, const std::string& prompt);

} // namespace lesson05
