#include "AiAgent.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <iostream>
#include <algorithm>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

using nlohmann::json;


AiAgent::AiAgent() {
    std::string dialog_begin = "Привет! Меня зовут Владимир ai, я могу помочь тебе с уходом за твоими домашними растениями. "
                               "Расскажи, за какими растениями ты ухаживаешь, и я дам краткую сводку об уходе за ними. "
                               "Я также готов ответить на твои вопросы :)\n";

    std::cout<<dialog_begin<<"\n";
    std::string first_message = "садовод-помощник:" + dialog_begin;
    context_.dialog = first_message;
    context_.table.clear();
}

void AiAgent::addMessageToDialog(std::string& message) {
    if (message.empty()) return;

    if (message == "таблица") {
        if (!context_.table.empty()) {
            std::cout << context_.table << std::endl << std::endl;
        } else {
            std::cout << "Таблица ещё не сформирована. Сначала расскажи, пожалуйста, о своих растениях — тогда я составлю план полива." << std::endl << std::endl;
        }
        return;
    }

    if (!message.empty()) {
        context_.dialog += "пользователь: ";
        context_.dialog += message;
        context_.dialog += "\n";
    }

    try {
        json full = createFullJson();
        prompt_ = full.dump();
    } catch (...) {
        prompt_.clear();
    }

    std::string localErr;
    auto resp = ask(&localErr);
    if (!resp) {
        std::cerr << "Request failed: " << localErr << "\n";
        return;
    }

    context_.dialog += "садовод-помощник: ";
    context_.dialog += *resp;
    context_.dialog += "\n";

    const std::string& fullResp = *resp;
    size_t firstStar = fullResp.find('*');
    size_t secondStar = (firstStar != std::string::npos) ? fullResp.find('*', firstStar + 1) : std::string::npos;
    if (firstStar != std::string::npos && secondStar != std::string::npos && secondStar > firstStar) {
        std::string tableStr = fullResp.substr(firstStar, secondStar - firstStar + 1);
        context_.table = tableStr;
    } 
    std::cout << *resp << std::endl;
    std::cout<<std::endl;
}

void AiAgent::conversation(std::string* err) {
    std::string input_buffer;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input_buffer)) {
            break;
        }
        std::cout<<std::endl;
        if (input_buffer == "пока.") break;

        addMessageToDialog(input_buffer);
    }
}

json AiAgent::createFullJson(std::string* err) {
    json result;
    result["prompt"] = prompt_;
    result["dialog"] = context_.dialog;
    return result;
}
