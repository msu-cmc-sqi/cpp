#pragma once
#include <optional>
#include <string>

struct AiConfig {
    std::string host;
    std::string port = "443";
    std::string api_key;
};

class AiAgent {
public:
    // Load the configuration (host, port, api_key) from a JSON file
    bool loadConfig(const std::string& path, std::string* err = nullptr);

    // Load the prompt from a JSON file (accepts either a string or an object with the key "prompt")
    bool loadPrompt(const std::string& path, std::string* err = nullptr);

    // Execute the request and return the parsed "text" from the response
    // Return std::nullopt on error (description written to outErr if provided)
    std::optional<std::string> ask(std::string* outErr = nullptr) const;

    // Set the prompt explicitly in code (not from a file)
    void setPrompt(std::string p) {
        prompt_ = std::move(p);
    }

private:
    // Low-level HTTPS POST to /api/generate
    static std::optional<std::string> httpsPostGenerate(
        const AiConfig& cfg, const std::string& jsonBody, std::string* err);

    // Simple JSON parsing: expect { "text": "<string>" }
    static std::string extractTextFromJsonBody(const std::string& body);

    // Read the entire file at path into out as binary
    static bool readWholeFile(const std::string& path, std::string& out, std::string* err);

private:
    AiConfig cfg_;
    std::string prompt_;
};
