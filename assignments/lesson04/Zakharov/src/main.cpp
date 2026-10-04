#include "model_request.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace {

int run(const std::filesystem::path& config_path,
        const std::string& config_path_text,
        const std::string& user_text) {
    std::ifstream config_file(config_path);
    if (!config_file) {
        std::cerr << "File error: cannot open configuration file '"
                  << config_path_text << "'\n";
        return 2;
    }

    std::ostringstream buffer;
    buffer << config_file.rdbuf();
    if (config_file.bad()) {
        std::cerr << "File error: cannot read configuration file '"
                  << config_path_text << "'\n";
        return 2;
    }

    try {
        const auto config = model_request::parse_config_text(buffer.str());
        const auto request = model_request::build_request(config, user_text);
        std::cout << request.dump(2) << '\n';
    } catch (const model_request::ConfigError& error) {
        std::cerr << "Configuration error: " << error.what() << '\n';
        return 3;
    } catch (const std::exception& error) {
        std::cerr << "Unexpected error: " << error.what() << '\n';
        return 4;
    }

    return 0;
}

#ifdef _WIN32
std::string to_utf8(const std::wstring& text) {
    if (text.empty()) {
        return {};
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, text.data(),
                                         static_cast<int>(text.size()), nullptr,
                                         0, nullptr, nullptr);
    if (size == 0) {
        throw std::runtime_error("cannot convert command-line argument to UTF-8");
    }

    std::string result(static_cast<std::size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                        result.data(), size, nullptr, nullptr);
    return result;
}
#endif

}  // namespace

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: model_request <config.json> <user text>\n";
        return 1;
    }

    try {
        return run(argv[1], to_utf8(argv[1]), to_utf8(argv[2]));
    } catch (const std::exception& error) {
        std::cerr << "Command-line error: " << error.what() << '\n';
        return 4;
    }
}
#else
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <config.json> <user text>\n";
        return 1;
    }

    return run(argv[1], argv[1], argv[2]);
}
#endif
