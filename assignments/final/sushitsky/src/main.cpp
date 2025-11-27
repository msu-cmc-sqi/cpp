#include "AiAgent.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    AiAgent agent;

    // Проверяем CLI режим
    bool cli_mode = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--cli") {
            cli_mode = true;
            break;
        }
    }

    std::string err;
    if (!agent.loadConfig("config.json", &err)) {
        std::cerr << "Config error: " << err << "\n";
        return 1;
    }
    if (!agent.loadPrompt("prompt.json", &err)) {
        std::cerr << "Prompt error: " << err << "\n";
        return 1;
    }

    // Если включен CLI режим
    if (cli_mode) {
        auto result = agent.processCLICommand(argc, argv, &err);
        if (!result) {
            std::cerr << "CLI error: " << err << "\n";
            return 2;
        }
        std::cout << *result << "\n";
    }
    else { // Оригинальный режим (совместимость)
        auto resp = agent.ask(&err);
        if (!resp) {
            std::cerr << "Request failed: " << err << "\n";
            return 2;
        }

        std::cout << *resp << "\n";
    }

    return 0;
}
