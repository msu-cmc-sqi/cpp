#include "MLConsultant.hpp"
#include <iostream>

int main() {
    MLC agent;
    std::string err;
    if (!agent.loadConfig("../config.json", &err)) {
        std::cerr << "Config error: " << err << "\n";
        return 1;
    }
    agent.printInfo();
    agent.userIntroduction(&err);
    std::string request = agent.getUserRequest(&err);
    while (request.size()) {
        std::cout << agent.promptBuilder(request, &err) << std::endl;
        auto resp = agent.ask(&err);
        if (!resp) {
            std::cerr << "Request failed: " << err << "\n";
            return 2;
        }
        std::cout << *resp << "\n";
        agent.saveHistory(resp, request);
        request = agent.getUserRequest(&err);
    }
    agent.endSession();
    return 0;
}
