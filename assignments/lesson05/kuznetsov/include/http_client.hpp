#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "model_config.hpp"

namespace model_app {

// RAII над libcurl
class CurlGlobalGuard {
public:
    CurlGlobalGuard();
    ~CurlGlobalGuard();
    CurlGlobalGuard(const CurlGlobalGuard&) = delete;
    CurlGlobalGuard& operator=(const CurlGlobalGuard&) = delete;
};

// POST-запрос
std::string send_http_post(const std::string& url, const std::string& json_payload, long timeout_seconds);

// choices[0].message.content
std::string extract_response_text(const std::string& raw_json_response);

} // namespace model_app