cmake_minimum_required(VERSION 3.20)
project(cmake_demo LANGUAGES CXX)

add_library(name_core src/normalize_name.cpp)
target_include_directories(name_core PUBLIC include)
target_compile_features(name_core PUBLIC cxx_std_17)

add_executable(name_cli src/main.cpp)
target_link_libraries(name_cli PRIVATE name_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(name_test tests/normalize_name_test.cpp)
    target_link_libraries(name_test PRIVATE name_core)
    add_test(NAME normalize_name COMMAND name_test)
endif()
