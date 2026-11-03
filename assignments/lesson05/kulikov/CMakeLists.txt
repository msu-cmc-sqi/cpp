cmake_minimum_required(VERSION 3.20...3.31)
project(lesson05_kulikov LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

include(FetchContent)
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(llm_client
    src/main.cpp
    src/client.cpp
)

target_include_directories(llm_client PRIVATE include)
target_link_libraries(llm_client PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(llm_client PRIVATE -Wall -Wextra -Wpedantic)
endif()
