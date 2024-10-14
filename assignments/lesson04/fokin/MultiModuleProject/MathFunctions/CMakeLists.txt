cmake_minimum_required(VERSION 3.10)	
project(MathFunctions)			
add_library(MathFunctions MathFunctions.cpp)	
target_include_directories(MathFunctions PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})