#include "AiAgent.hpp"

#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

#include <netdb.h>
#include <nlohmann/json.hpp>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>

using nlohmann::json;

// Read the entire file at path into out as binary
bool AiAgent::readWholeFile(const std::string& path, std::string& out, std::string* err) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        if (err) {
            *err = "Cannot open file: " + path;
        }
        return false;
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    out = std::move(ss).str();
    return true;
}

// Load the configuration (host, port, api_key) from a JSON file
bool AiAgent::loadConfig(const std::string& path, std::string* err) {
    std::string s;
    if (!readWholeFile(path, s, err)) {
        return false;
    }
    try {
        auto j = json::parse(s);
        // use at() for required fields; leave port as-is if missing
        cfg_.host = j.at("host").get<std::string>();
        if (j.contains("port")) {
            cfg_.port = j.at("port").get<std::string>();
        }
        cfg_.api_key = j.at("api_key").get<std::string>();
        return true;
    } catch (const std::exception& e) {
        if (err) {
            *err = std::string("Config parse error: ") + e.what();
        }
        return false;
    }
}

// Load the prompt from a JSON file (accepts either a string or an object with the key "prompt")
bool AiAgent::loadPrompt(const std::string& path, std::string* err) {
    std::string s;
    if (!readWholeFile(path, s, err)) {
        return false;
    }
    try {
        // Assume the file is either a JSON string or an object with the key "prompt".
        json j = json::parse(s);
        if (j.is_string()) {
            prompt_ = j.get<std::string>();
        } else if (j.is_object()) {
            prompt_ = j.at("prompt").get<std::string>();
        } else {
            if (err) {
                *err = "Prompt JSON must be string or object with key 'prompt'";
            }
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        if (err) {
            *err = std::string("Prompt parse error: ") + e.what();
        }
        return false;
    }
}

// Simple JSON parsing: expect { "text": "<string>" }
std::string AiAgent::extractTextFromJsonBody(const std::string& body) {
    // If it comes with HTTP headers, cut them off
    const auto p = body.find("\r\n\r\n");
    const std::string json_part = (p != std::string::npos) ? body.substr(p + 4) : body;
    try {
        auto j = json::parse(json_part);
        return j.at("text").get<std::string>();  // strictly expect the field "text"
    } catch (...) {
        return {};
    }
}

// Low-level HTTPS POST to /api/generate
std::optional<std::string> AiAgent::httpsPostGenerate(
        const AiConfig& cfg, const std::string& jsonBody, std::string* err) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        if (err) {
            *err = "SSL_CTX_new failed";
        }
        return std::nullopt;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        if (err) {
            *err = "socket failed";
        }
        SSL_CTX_free(ctx);
        return std::nullopt;
    }

    struct addrinfo hints = {};
    struct addrinfo* res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(cfg.host.c_str(), cfg.port.c_str(), &hints, &res) != 0) {
        if (err) {
            *err = "getaddrinfo failed";
        }
        close(sock);
        SSL_CTX_free(ctx);
        return std::nullopt;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        if (err) {
            *err = "connect failed";
        }
        freeaddrinfo(res);
        close(sock);
        SSL_CTX_free(ctx);
        return std::nullopt;
    }
    freeaddrinfo(res);

    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    if (SSL_connect(ssl) <= 0) {
        if (err) {
            *err = "SSL_connect failed";
        }
        SSL_free(ssl);
        close(sock);
        SSL_CTX_free(ctx);
        return std::nullopt;
    }

    // HTTP request
    std::ostringstream req;
    req << "POST /api/generate HTTP/1.1\r\n"
        << "Host: " << cfg.host << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Connection: close\r\n";
    if (!cfg.api_key.empty()) {
        req << "x-api-key: " << cfg.api_key << "\r\n";
    }
    req << "Content-Length: " << jsonBody.size() << "\r\n\r\n" << jsonBody;

    const std::string request_str = req.str();
    if (SSL_write(ssl, request_str.c_str(), (int)request_str.size()) <= 0) {
        if (err) {
            *err = "SSL_write failed";
        }
        SSL_free(ssl);
        close(sock);
        SSL_CTX_free(ctx);
        return std::nullopt;
    }

    char buf[4096];
    std::string response;
    int bytes;
    while ((bytes = SSL_read(ssl, buf, sizeof(buf)-1)) > 0) {
        buf[bytes] = '\0';
        response += buf;
    }

    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);

    // Use nlohmann::json to extract "text"
    std::string text = extractTextFromJsonBody(response);
    if (text.empty()) {
        if (err) {
            *err = "Cannot extract \"text\" from JSON response";
        }
        return std::nullopt;
    }
    return text;
}

// Execute the request and return the parsed "text" from the response
// Return std::nullopt on error (description written to outErr if provided)
std::optional<std::string> AiAgent::ask(std::string* outErr) const {
    if (cfg_.host.empty() || cfg_.api_key.empty()) {
        if (outErr) {
            *outErr = "Config not loaded or api_key/host missing";
        }
        return std::nullopt;
    }
    if (prompt_.empty()) {
        if (outErr) {
            *outErr = "Prompt is empty (load it first)";
        }
        return std::nullopt;
    }

    // Construct the request body as valid JSON using nlohmann::json
    json payload = { {"prompt", prompt_} };
    const std::string body = payload.dump();

    return httpsPostGenerate(cfg_, body, outErr);
}
