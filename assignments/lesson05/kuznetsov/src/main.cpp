#include "model_config.hpp"
#include "http_client.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./llm_cli <config_path> \"<prompt_text>\"\n";

        return 1;
    }

    std::string config_path = argv[1];
    std::string user_prompt = argv[2];

    try {
        model_app::CurlGlobalGuard curl_guard;

        std::ifstream file(config_path); // чтение файла конфигурации
        if (!file.is_open()) {
            std::cerr << "Error: Could not open config file: " << config_path << "\n";

            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        // парсинг и формирование json-запроса
        auto config = model_app::load_config(buffer.str());
        auto request_json = model_app::create_chat_request(config, user_prompt);

        std::cout << "Sending request to " << config.server_url << "...\n";

        // POST
        std::string raw_response = model_app::send_http_post(
            config.server_url,
            request_json.dump(),
            config.timeout_seconds
        );

        // извлечение вывода
        std::string result_text = model_app::extract_response_text(raw_response);

        std::cout << "\n--- Model Response ---\n";
        std::cout << result_text << "\n";
        std::cout << "----------------------\n";

    }
    catch (const std::exception& e) {
        std::cerr << "\n[Error]: " << e.what() << "\n";

        return 1;
    }

    return 0;
}