cmake_minimum_required(VERSION 3.10)	
project(MainApp)
add_executable(MainApp main.cpp)
target_link_libraries(MainApp PRIVATE MathFunctions StringUtilities)