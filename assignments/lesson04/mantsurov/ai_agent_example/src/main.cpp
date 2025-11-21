#include "AiAgent.hpp"
#include <iostream>

int main() {
    AiAgent agent;

    std::string err;
    if (!agent.loadConfig("../config.json", &err)) {
        std::cerr << "Config error: " << err << "\n";
        return 1;
    }
    if (!agent.loadPrompt("../prompt.json", &err)) {
        std::cerr << "Prompt error: " << err << "\n";
        return 1;
    }

    std::cout<< "Навигация в диалоге:\n";
    std::cout<<  "\"таблица\" - повторить таблицу полива (если она уже представлена)\n";
    std::cout<< "\"пока.\" - завершить диалог\n";
    std::cout<< "любые другие строки расцениваются как сообщение помощнику.\n";

    std::cout<<std::endl;

    agent.conversation(&err);
    return 0;
}

