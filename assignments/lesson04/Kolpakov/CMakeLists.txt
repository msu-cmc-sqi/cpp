cmake_minimum_required(VERSION 3.20)
project(model_request LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Способ получения JSON-библиотеки — FetchContent.
include(FetchContent)
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# Библиотечная цель с логикой конфигурации и построения запроса.
add_library(config_core
    src/model_config.cpp
)
target_include_directories(config_core PUBLIC include)
target_compile_features(config_core PUBLIC cxx_std_17)
# Публичный заголовок включает <nlohmann/json.hpp> → зависимость PUBLIC.
target_link_libraries(config_core PUBLIC nlohmann_json::nlohmann_json)

# Исполняемая цель CLI.
add_executable(model_request src/main.cpp)
target_link_libraries(model_request PRIVATE config_core)

# Тестовая цель — использует ту же библиотеку.
include(CTest)
if(BUILD_TESTING)
    add_executable(config_test tests/config_test.cpp)
    target_link_libraries(config_test PRIVATE config_core)
    add_test(NAME config_test COMMAND config_test)
endif()
