add_library(MathFunctions MathFunctions.cpp)

install(TARGETS MathFunctions DESTINATION lib)

target_include_directories(MathFunctions PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})