#include "model_config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "USAGE: ./model_request <config_path> <prompt_text>\n";
        return 1;
    }

    std::string config_path{ argv[1] };
    std::string user_prompt{ argv[2] };

    // чтение конфига
    std::ifstream file{ config_path };
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file: " << config_path << "\n";
        return 1;
    }

    std::stringstream buf;
    buf << file.rdbuf();

    try {
        auto request = model_app::create_api_request(buf.str(), user_prompt);
        std::cout << request.dump(4) << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Validation error: " << e.what() << "\n";
    }

    return 0;
}