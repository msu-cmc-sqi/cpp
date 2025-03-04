cmake_minimum_required(VERSION 3.10)

project(MainProject)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(MainApp)
set(MAIN_PROJECT_SRC_LIST MainApp/main.cpp)
add_subdirectory(MathFunctions)
add_subdirectory(StringUtilities)
option(BUILD_MATH_FUNCTIONS "Build MathFunctions library" ON)
option(BUILD_STRING_UTILITIES "Build StringUtilities library" ON)
