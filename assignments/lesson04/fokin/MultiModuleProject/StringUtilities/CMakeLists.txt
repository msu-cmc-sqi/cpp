cmake_minimum_required(VERSION 3.10)	
project(StringUtilities)			
add_library(StringUtilities StringUtilities.cpp)	
target_include_directories(StringUtilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})