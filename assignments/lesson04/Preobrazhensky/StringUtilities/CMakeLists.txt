cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(StringUtilities STATIC ${CMAKE_CURRENT_SOURCE_DIR}/StringUtilities.h ${CMAKE_CURRENT_SOURCE_DIR}/StringUtilities.cpp)

target_link_libraries(StringUtilities PUBLIC)

install(TARGETS StringUtilities DESTINATION lib)
