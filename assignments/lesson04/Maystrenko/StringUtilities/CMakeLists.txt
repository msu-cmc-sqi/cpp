add_library(StringUtilities STATIC ${CMAKE_CURRENT_SOURCE_DIR}/StringUtilities.cpp ${CMAKE_CURRENT_SOURCE_DIR}/StringUtilities.h)
target_link_libraries(StringUtilities PUBLIC)
install(TARGETS StringUtilities DESTINATION lib)

