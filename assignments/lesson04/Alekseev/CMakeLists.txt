cmake_minimum_required(VERSION 3.10)
project(Myproject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(MathFunctions)
add_subdirectory(StringUtilities)
add_subdirectory(MainApp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MathFunctions)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/StringUtilities)

option(BUILD_MATH_FUNCTIONS "Build MathFunctions library" ON)
option(BUILD_STRING_UTILITIES "Build StringUtilities library" ON)
