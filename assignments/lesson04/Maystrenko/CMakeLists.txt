cmake_minimum_required(VERSION 3.0.0)
project(MultiModuleProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MathFunctions)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/StringUtilities)

add_subdirectory(MathFunctions)
add_subdirectory(StringUtilities)
add_subdirectory(MainApp)

option(BUILD_MATH_FUNCTIONS "Build MathFunctions library" ON)
option(BUILD_STRING_UTILITIES "Build StringUtilities library" ON)
