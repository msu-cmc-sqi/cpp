#include "AiAgent.h"
#include <iostream>

int main() {
    AiAgent agent;

    std::string err;
    if (!agent.loadConfig("config.json", &err)) {
        std::cerr << "Config error: " << err << "\n";
        return 1;
    }
    if (!agent.loadPrompt("prompt.json", &err)) {
        std::cerr << "Prompt error: " << err << "\n";
        return 1;
    }

    std::cout << "\n\n\nFitness AiAgent запущен!\n\n";
    std::cout << "Чтобы выйти — напиши 'пока'.\n\n";

    auto resp = agent.ask(&err);
    if (!resp) {
        std::cerr << "Request failed: " << err << "\n";
        return 2;
    }

    std::cout << *resp << "\n";
    return 0;
}
