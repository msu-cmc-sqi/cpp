cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_library(MathFunctions STATIC ${CMAKE_CURRENT_SOURCE_DIR}/MathFunctions.h ${CMAKE_CURRENT_SOURCE_DIR}/MathFunctions.cpp)

target_link_libraries(MathFunctions PUBLIC)

install(TARGETS MathFunctions DESTINATION lib)
