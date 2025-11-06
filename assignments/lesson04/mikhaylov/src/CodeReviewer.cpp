#include "CodeReviewer.hpp"
#include <nlohmann/json.hpp>
#include <sstream>
#include <fstream>

// Local helper: read entire file into string (same behaviour as AiAgent::readWholeFile)
static bool readWholeFileLocal(const std::string& path, std::string& out, std::string* err) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        if (err) *err = "Cannot open file: " + path;
        return false;
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

// Initialize: load config into inner agent and load base prompt from file
bool CodeReviewer::init(const std::string& configPath, const std::string& promptPath, std::string* err) {
    // Load config into agent (AiAgent::loadConfig)
    if (!agent_.loadConfig(configPath, err)) {
        return false;
    }
    // Load base prompt into local string (do not set agent prompt yet)
    std::string tmpErr;
    std::string promptFileContents;
    if (!readWholeFileLocal(promptPath, promptFileContents, &tmpErr)) {
        if (err) *err = tmpErr;
        return false;
    }
    try {
        // parse JSON similar to AiAgent::loadPrompt
        nlohmann::json j = nlohmann::json::parse(promptFileContents);
        if (j.is_string()) {
            base_prompt_ = j.get<std::string>();
        } else if (j.is_object()) {
            base_prompt_ = j.at("prompt").get<std::string>();
        } else {
            if (err) *err = "Prompt JSON must be string or object with key 'prompt'";
            return false;
        }
    } catch (const std::exception& e) {
        if (err) *err = std::string("Prompt parse error: ") + e.what();
        return false;
    }
    return true;
}

// Build the final prompt combining base prompt and the source code and small instruction template.
// This ensures user only needs to change prompt.json or source file content between runs.
std::string CodeReviewer::buildPrompt() const {
    std::ostringstream out;
    out << base_prompt_ << "\n\n";
    out << "Instruction: Analyze the following source code and provide a numbered list of potential issues,\n";
    out << "each with: (1) short title, (2) brief explanation, (3) severity (Low/Medium/High), (4) suggested fix,\n";
    out << "(5) code example of fix, if requested.\n";
    out << "If code is correct, reply with: \"No issues found.\" Keep answers concise and in Russian.\n\n";
    if (!language_.empty()) {
        out << "Language: " << language_ << "\n\n";
    } else {
        out << "Language: (auto-detect)\n\n";
    }
    out << "----BEGIN SOURCE----\n";
    out << source_code_ << "\n";
    out << "----END SOURCE----\n";
    return out.str();
}

// Run the review: set prompt on agent and call ask()
std::optional<std::string> CodeReviewer::run(std::string* err) const {
    if (base_prompt_.empty()) {
        if (err) *err = "Base prompt is empty (did you init with prompt.json?)";
        return std::nullopt;
    }
    if (source_code_.empty()) {
        if (err) *err = "Source code not set";
        return std::nullopt;
    }

    std::string finalPrompt = buildPrompt();
    // create a copy of agent to set prompt (AiAgent::setPrompt is non-const)
    AiAgent a = agent_; // relies on AiAgent being copyable
    a.setPrompt(finalPrompt);
    return a.ask(err);
}
