cmake_minimum_required(VERSION 3.20)

project(model_request LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(model_request_core src/model_request.cpp)
target_include_directories(model_request_core PUBLIC include)
target_compile_features(model_request_core PUBLIC cxx_std_17)
target_link_libraries(model_request_core PUBLIC nlohmann_json::nlohmann_json)

add_executable(model_request src/main.cpp)
target_link_libraries(model_request PRIVATE model_request_core)
if(MSVC)
    target_compile_options(model_request PRIVATE /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(model_request_test tests/model_request_test.cpp)
    target_link_libraries(model_request_test PRIVATE model_request_core)
    add_test(NAME model_request_test COMMAND model_request_test)
endif()
