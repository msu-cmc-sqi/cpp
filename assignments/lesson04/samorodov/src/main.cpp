#include "AiAgent.h"
#include <iostream>
#include <fstream>
#include <string>

void printUsage() {
    std::cout << "Code Analyzer AI Agent\n"
              << "Usage:\n"
              << "  ai_agent analyze <file> [language]  - Analyze code file\n"
              << "  ai_agent code <code_string> [language] - Analyze code from string\n"  
              << "  ai_agent prompt                     - Run original prompt demo\n"
              << "  ai_agent interactive               - Interactive mode\n"
              << "\nLanguages: cpp, python (default: cpp)\n";
}

bool fileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

void runInteractiveMode(AiAgent& agent) {
    std::cout << "=== Interactive Code Analysis ===\n"
              << "Enter code (end with 'EOF' on a new line):\n";
    
    std::string code, line;
    while (std::getline(std::cin, line)) {
        if (line == "EOF") break;
        code += line + "\n";
    }
    
    if (!code.empty()) {
        std::cout << "Enter language (cpp/python): ";
        std::string lang;
        std::getline(std::cin, lang);
        if (lang.empty()) lang = "cpp";
        
        std::string err;
        auto result = agent.analyzeCodeString(code, lang, &err);
        if (!err.empty()) {
            std::cerr << "Error: " << err << "\n";
        } else if (result) {
            std::cout << "\nAnalysis Result:\n" << *result << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    AiAgent agent;
    
    std::string err;
    if (!agent.loadConfig("config.json", &err)) {
        std::cerr << "Config error: " << err << "\n";
        return 1;
    }
    
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    std::string command = argv[1];
    
    if (command == "prompt") {
        if (!agent.loadPrompt("prompt.json", &err)) {
            std::cerr << "Prompt error: " << err << "\n";
            return 1;
        }
        
        auto resp = agent.ask(&err);
        if (!resp) {
            std::cerr << "Request failed: " << err << "\n";
            return 2;
        }
        std::cout << *resp << "\n";
        
    } else if (command == "analyze" && argc >= 3) {
        std::string filename = argv[2];
        std::string language = (argc >= 4) ? argv[3] : "cpp";
        
        if (!fileExists(filename)) {
            std::cerr << "File not found: " << filename << "\n";
            return 1;
        }
        
        auto result = agent.analyzeCodeFile(filename, language, &err);
        if (!err.empty()) {
            std::cerr << "Analysis failed: " << err << "\n";
            return 1;
        }
        
        if (result) {
            std::cout << *result << "\n";
        }
        
    } else if (command == "code" && argc >= 3) {
        std::string code = argv[2];
        std::string language = (argc >= 4) ? argv[3] : "cpp";
        
        auto result = agent.analyzeCodeString(code, language, &err);
        if (!err.empty()) {
            std::cerr << "Analysis failed: " << err << "\n";
            return 1;
        }
        
        if (result) {
            std::cout << *result << "\n";
        }
        
    } else if (command == "interactive") {
        runInteractiveMode(agent);
        
    } else {
        printUsage();
        return 1;
    }
    
    return 0;
}