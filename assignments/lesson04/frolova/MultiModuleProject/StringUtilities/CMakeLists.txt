add_library(StringUtilities StringUtilities.cpp)

install(TARGETS StringUtilities DESTINATION lib)

target_include_directories(StringUtilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})