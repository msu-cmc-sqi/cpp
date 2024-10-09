cmake_minimum_required(VERSION 3.30)
project(MultiModuleProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_MATH_FUNCTIONS "Build MathFunctions library" ON)
option(BUILD_STRING_UTILITIES "Build StringUtilities library" ON)

add_subdirectory(MathFunctions)
add_subdirectory(StringUtilities)
add_subdirectory(MainApp)

install(TARGETS MainApp DESTINATION bin)
install(TARGETS MathFunctions DESTINATION lib)
install(TARGETS StringUtilities DESTINATION lib)