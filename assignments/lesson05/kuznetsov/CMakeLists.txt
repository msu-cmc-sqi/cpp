cmake_minimum_required(VERSION 3.20)
project(model_http_client LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

find_package(CURL REQUIRED)

# Библиотека с ключевой логикой (конфиг, сборка JSON, HTTP-запросы)
add_library(client_core 
    src/model_config.cpp 
    src/http_client.cpp
)
target_include_directories(client_core PUBLIC include)
target_compile_features(client_core PUBLIC cxx_std_17)
target_link_libraries(client_core 
    PUBLIC 
        nlohmann_json::nlohmann_json
        CURL::libcurl
)

add_executable(llm_cli src/main.cpp) # исполняемый файл
target_link_libraries(llm_cli PRIVATE client_core)
