cmake_minimum_required(VERSION 3.20)
project(model_request_app LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(nlohmann_json
GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(config_core src/model_config.cpp) # создаёт библиотечную цель-таргет с именем config_core
target_include_directories(config_core PUBLIC include) # значит, что include нужна. не только библиотеке при сборке, но и всем остальным модулям
target_compile_features(config_core PUBLIC cxx_std_17) # требует использовать С++17 для компиляции этой библиотеки и тем кто от неё зависит, так как public
target_link_libraries(config_core PUBLIC nlohmann_json::nlohmann_json)

add_executable(model_request src/main.cpp) # создание цели для исп файла с именем config_cli
target_link_libraries(model_request PRIVATE config_core) # связывание библиотеки с исполняемым файлом config_cli
# PRIVATE означает, что библиотека используется только внутри этой программы

include(CTest)
if(BUILD_TESTING)
    add_executable(config_test tests/config_test.cpp)
    target_link_libraries(config_test PRIVATE config_core)
    add_test(NAME config_test COMMAND config_test) # регистрирует тест в системе ctest, и при вызове этой комманды cmake будет знать, что нужно запустить config_test
endif()