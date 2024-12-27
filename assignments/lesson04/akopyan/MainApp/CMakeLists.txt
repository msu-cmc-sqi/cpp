add_executable(MainApp main.cpp)

if (BUILD_MATH_FUNCTIONS)
    target_link_libraries(MainApp MathFunctions)
endif()

if (BUILD_STRING_UTILITIES)
    target_link_libraries(MainApp StringUtilities)
endif()
