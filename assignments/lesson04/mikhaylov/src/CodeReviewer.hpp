#pragma once
#include "AiAgent.hpp"
#include <string>
#include <optional>

// CodeReviewer wraps AiAgent and provides specialised prompt construction
// and a simple run() method to execute a code-review request.
class CodeReviewer {
public:
    CodeReviewer() = default;

    // Load agent config and initial prompt file (prompt.json)
    bool init(const std::string& configPath, const std::string& promptPath, std::string* err = nullptr);

    // Set user's code to be reviewed (raw source as string)
    void setSourceCode(const std::string& src) { source_code_ = src; }

    // Optional: set the language explicitly ("cpp" or "python"); if empty, CodeReviewer will try to detect
    void setLanguage(const std::string& lang) { language_ = lang; }

    // Run the review: constructs final prompt (base prompt + user code),
    // sets it on the inner agent and calls ask().
    std::optional<std::string> run(std::string* err = nullptr) const;

private:
    // Build the final prompt by combining the loaded base prompt and the source code.
    // Always include a short structured template so the model returns a list of issues.
    std::string buildPrompt() const;

private:
    AiAgent agent_;
    std::string base_prompt_;   // loaded from prompt.json
    std::string source_code_;
    std::string language_;
};
