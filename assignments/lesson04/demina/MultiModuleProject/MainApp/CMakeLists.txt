add_executable(MainApp main.cpp)
target_link_libraries(MainApp PRIVATE MathFunctions StringUtilities)

target_include_directories(MainApp PRIVATE
	${CMAKE_SOURCE_DIR}/MathFunctions
	${CMAKE_SOURCE_DIR}/StringUtilities
)
