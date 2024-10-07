add_library(MathFunctions STATIC ${CMAKE_CURRENT_SOURCE_DIR}/MathFunctions.cpp ${CMAKE_CURRENT_SOURCE_DIR}/MathFunctions.h)
target_link_libraries(MathFunctions PUBLIC)
install(TARGETS MathFunctions DESTINATION lib)

