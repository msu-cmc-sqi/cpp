cmake_minimum_required(VERSION 3.14)
project(AiAgentExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# nlohmann/json для чтения config.json/prompt.json и формирования тела запроса
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# OpenSSL для TLS
find_package(OpenSSL REQUIRED)

add_executable(ai_agent
    src/AiAgent.cpp
    src/MyMethods.cpp
    src/main.cpp
)

target_include_directories(ai_agent PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(ai_agent
    PRIVATE
      nlohmann_json::nlohmann_json
      OpenSSL::SSL
      OpenSSL::Crypto
)
