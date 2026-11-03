#include "http_client.hpp"
#include <curl/curl.h>
#include <stdexcept>
#include <iostream>

namespace model_app {
    CurlGlobalGuard::CurlGlobalGuard() {
        if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
            throw std::runtime_error("Failed to initialize libcurl globally.");
        }
    }

    CurlGlobalGuard::~CurlGlobalGuard() {
        curl_global_cleanup();
    }

    // Callback-функция libcurl для записи ответа во внутренний буфер std::string
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t total_size = size * nmemb;
        std::string* response = static_cast<std::string*>(userp);

        response->append(static_cast<char*>(contents), total_size);

        return total_size;
    }

    std::string send_http_post(const std::string& url, const std::string& json_payload, long timeout_seconds) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL easy handle.");
        }

        std::string response_string;
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_seconds);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            throw std::runtime_error(std::string("HTTP Request failed: ") + curl_easy_strerror(res));
        }

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (http_code != 200) {
            throw std::runtime_error("Server returned non-200 HTTP status code: " + std::to_string(http_code) +
                "\nBody: " + response_string);
        }

        return response_string;
    }

    std::string extract_response_text(const std::string& raw_json_response) {
        try {
            auto j = nlohmann::json::parse(raw_json_response);

            if (j.contains("choices") && j["choices"].is_array() && !j["choices"].empty()) {
                auto first_choice = j["choices"][0];

                if (first_choice.contains("message") && first_choice["message"].contains("content")) {
                    return first_choice["message"]["content"].get<std::string>();
                }
            }

            throw std::runtime_error("Invalid response JSON structure: missing choices[0].message.content");
        }
        catch (const nlohmann::json::exception& e) {
            throw std::runtime_error(std::string("JSON parse error: ") + e.what());
        }
    }

} // namespace model_app