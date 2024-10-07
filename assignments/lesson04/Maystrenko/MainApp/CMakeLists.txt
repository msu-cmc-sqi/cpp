add_executable(MainApp ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(MainApp PRIVATE MathFunctions StringUtilities)
install(TARGETS MainApp DESTINATION bin)

