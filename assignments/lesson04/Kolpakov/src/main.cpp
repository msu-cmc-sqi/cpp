#include "model_config.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string read_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("не удалось открыть файл конфигурации: " + path);
    }
    std::ostringstream buf;
    buf << in.rdbuf();
    return buf.str();
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0]
                  << " <config.json> \"<user text>\"\n";
        return 2;
    }

    const std::string config_path = argv[1];
    const std::string user_text   = argv[2];

    std::string text;
    try {
        text = read_file(config_path);
    } catch (const std::exception& e) {
        std::cerr << "file error: " << e.what() << "\n";
        return 1;
    }

    model_request::ModelConfig cfg;
    try {
        cfg = model_request::parse_config(text);
    } catch (const model_request::ConfigSyntaxError& e) {
        std::cerr << "syntax error: " << e.what() << "\n";
        return 1;
    } catch (const model_request::ConfigStructureError& e) {
        std::cerr << "config error: " << e.what() << "\n";
        return 1;
    }

    const auto request = model_request::build_request(cfg, user_text);
    std::cout << request.dump(2) << "\n";
    return 0;
}
