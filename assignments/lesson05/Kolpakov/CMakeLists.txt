cmake_minimum_required(VERSION 3.20)
project(lesson05_chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

include(FetchContent)
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(chat_client STATIC
    src/chat_client.cpp
)
target_include_directories(chat_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(chat_client PUBLIC CURL::libcurl nlohmann_json::nlohmann_json)

add_executable(local_chat src/main.cpp)
target_link_libraries(local_chat PRIVATE chat_client)
