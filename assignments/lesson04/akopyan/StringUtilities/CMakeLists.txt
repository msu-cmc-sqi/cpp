if (BUILD_STRING_UTILITIES)
    add_library(StringUtilities StringUtilities.cpp)
    target_include_directories(StringUtilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif()
