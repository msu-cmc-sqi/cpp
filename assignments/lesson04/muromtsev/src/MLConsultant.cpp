#include "MLConsultant.hpp"
#include <iostream>
#include <fstream>

void MLC::printInfo()
{
    std::cout << "Здравствуйте! Я ваш персональный консультант в области машинного обучения." << std::endl;
    std::cout << "Я могу помочь вам с выбором метода машинного обучения для решения поставленной задачи." << std::endl;
    std::cout << "Для того, чтобы продолжить, введите ваше имя - возможно, мы с вами уже знакомы." << std::endl;
}
bool MLC::loadHistory(std::string *err) 
{
    if (!cfg_.history_path.has_value()) return false;
    std::string s;
    const std::string full_path = cfg_.history_path.value() + "/" + username_ + ".json";
    if (!readWholeFile(full_path, s, err)) {
        std::ofstream histfile {full_path};
        histfile << R"({ "requests": []})";
        histfile.close();
        history_ = json::parse(R"({ "requests": []})");
        return false;
    }
    try {
        history_ = json::parse(std::move(s));
        return true;
    } catch (const std::exception& e) {
        if (err) *err = std::string("History parse error: ") + e.what();
        return false;
    }
    return false;
}
void MLC::userIntroduction(std::string *err = nullptr)
{
    std::cout << "Введите имя:" << std::endl;
    while (!username_.size()) std::cin >> username_;
    std::cout << "Здравствуйте, " << username_ << "!" << std::endl;
    if (!loadHistory(err)) {
        std::cout << "Приятно познакомиться. Какой у вас вопрос?" << std::endl;
    } else {
        std::cout << "Похоже мы с вами уже работали. У вас остались вопросы по прошлой теме или вы хотите уточнить что-то другое?" << std::endl;
    }
}

std::string MLC::getUserRequest(std::string *err = nullptr)
{
    std::string line;
    std::string request;
    while (std::getline(std::cin, line, '\n')) {
        if (line == "END") break;
        if (line.empty()) continue;
        request += line + '\n';
    }
    return std::move(request);
}

std::string MLC::promptBuilder(std::string request, std::string *err = nullptr)
{
    std::stringstream ss;
    ss << "Ты — консультант в области ML" << std::endl;
    ss << "Ты общаешься с пользователем по имени " << username_ << std::endl;
    ss << "Не здоровайся с пользоваталем, обращайся к нему на вы" << std::endl;
    ss << "Отвечай коротко и по существу" << std::endl;
    ss << "В конце предложи пользователю задать тебе ещё какой-нибудь вопрос" << std::endl;
    auto history = history_.at("requests").get<std::vector<std::vector<std::string>>>();
    if (history.size()) {
        ss << "Вот предыдущие запросы пользователя и твои ответы:" << std::endl;
        for (const auto &request : history) {
            ss << "----------------------" << std::endl;
            ss << "Запрос пользователя: " << request[0] << std::endl << "Твой ответ на него: " << request[1] << std::endl;
        }
    }
    ss << "----------------------" << std::endl;
    ss << "Запрос пользователя следующий:" << std::endl;
    ss << request << std::endl;
    ss << "----------------------" << std::endl;
    prompt_ = ss.str();
    return ss.str();
}

void MLC::endSession()
{
    const std::string full_path = cfg_.history_path.value() + "/" + username_ + ".json";
    std::ofstream histfile {full_path};
    histfile << history_.dump(4);
    histfile.close();
}

void MLC::saveHistory(std::optional<std::string> answer, const std::string &request)
{
    auto history = history_.at("requests").get<std::vector<std::vector<std::string>>>();
    history.push_back({request, answer.value()});
    history_["requests"] = history;
}