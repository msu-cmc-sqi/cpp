#pragma once
#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include "AiAgent.h"

using json = nlohmann::json;

class MLC final : public AiAgent {
public:
void printInfo();
void userIntroduction(std::string *err);
std::string promptBuilder(std::string request, std::string *err);
std::string getUserRequest(std::string *err);
void endSession();
void saveHistory(std::optional<std::string> answer, const std::string &request);
private:

bool loadHistory(std::string *err);

private:
std::string username_;
json history_;
};