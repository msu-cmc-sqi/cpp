cmake_minimum_required(VERSION 3.20)
project(pirogov_model_request LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(request_core src/model_request.cpp)
target_include_directories(request_core PUBLIC include)
target_compile_features(request_core PUBLIC cxx_std_17)
target_link_libraries(request_core PUBLIC nlohmann_json::nlohmann_json)

add_executable(model_request src/main.cpp)
target_link_libraries(model_request PRIVATE request_core)

if(WIN32)
    target_sources(model_request PRIVATE src/utf8.manifest)
endif()

if(MSVC)
    target_compile_options(request_core PRIVATE /utf-8)
    target_compile_options(model_request PRIVATE /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(request_test tests/model_request_test.cpp)
    target_link_libraries(request_test PRIVATE request_core)
    if(MSVC)
        target_compile_options(request_test PRIVATE /utf-8)
    endif()
    add_test(NAME request_test COMMAND request_test)
endif()
