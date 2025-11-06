#include "AiAgent.h"
#include "MusicAgent.h"
#include <iostream>
#include <string>

void printUsage() {
    std::cout << "Как пользоваться:\n"
              << " ./music_ai_agent --suggest - Советует подборку треков из Яндекс.Музыки, подходящих вашему текущему настроению\n"
              << " ./music_ai_agent --find - Может найти любой доступный в Яндекс.Музыке трек\n"
              << " ./music_ai_agent -help - Помощь в использовании\n";
}

int main(int argc, char *argv[])
{
    AiAgent aiagent;
    MusicAgent musicagent;

    std::cout << "Привет! Я - AI-ассистент для подбора музыки!\n";
    std::string err;
    if (!aiagent.loadConfig("config.json", &err)) {
        std::cerr << "Ai Config error: " << err << std::endl;
        return 1;
    }
    if (argc < 2) {
        printUsage();
        return 1;
    }
    std::string command = argv[1];
    if (command == "--suggest") {
        if (!aiagent.loadPrompt("prompt.json", &err)) {
            std::cerr << "Ai Prompt error: " << err << std::endl;
            return 1;
        }
        if (!musicagent.loadConfig("config.json", &err)) {
            std::cerr << "Music Config error: " << err << std::endl;
            return 1;
        }
        std::cout << "Расскажи о своем настроении и состоянии, и я подберу подходящую музыку\n";
        std::cout << "> ";
        std::string mood;
        std::getline(std::cin, mood);
        if (mood.empty()) {
            std::cerr << "Ты не описал свое настроение:(\n";
            return 1;
        }

        std::cout << "Размышляю...\n";
        auto search = aiagent.generateMusic(mood, &err);
        if (!search) {
            std::cerr << "Ошибка генерации ответа: " << err << "\n";
            return 2;
        }

        auto recommendations = musicagent.getMusicRecommendation(*search, &err);
        if (!recommendations) {
            std::cerr << "Ошибка поиска:  " << err << "\n";
            return 3;
        }
        aiagent.printResult(*recommendations, mood, &err);
    } else if (command == "--find") {
        if (!aiagent.loadPrompt("prompt.json", &err)) {
            std::cerr << "Ai Prompt error: " << err << std::endl;
            return 1;
        }
        if (!musicagent.loadConfig("config.json", &err)) {
            std::cerr << "Music Config error: " << err << std::endl;
            return 1;
        }   
        std::cout << "Какой бы трек ты хотел найти?\n";
        std::cout << "Пожалуйста напиши желаемый трек в следующем формате: <Название трека> - <Исполнитель>\n";
        std::cout << "Например: That's Life - Frank Sinatra\n";
        std::cout << ">";
        std::string search;
        std::getline(std::cin, search);
        if (search.empty()) {
            std::cerr << "Ты не ввел никакой трек:(\n";
            return 1;
        }
        auto recommendations = musicagent.findTrack(search, &err);
        if (!recommendations) {
            std::cerr << "Ошибка поиска: " << err << "\n";
            return 3;
        }
        aiagent.printResult(*recommendations, "", &err);
    } else if (command == "-help") {
        printUsage();
        return 1;
    } else {
        printUsage();
        return 1;
    }

    return 0;
}