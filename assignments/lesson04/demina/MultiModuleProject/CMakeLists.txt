cmake_minimum_required(VERSION 3.10)

project(MultiModuleProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_MATH_FUNCTIONS "Build MathFunctions library" ON)
option(BUILD_STRING_UTILITIES "Build StringUtilities library" ON)

if(BUILD_MATH_FUNCTIONS)
	    add_subdirectory(MathFunctions)
endif()

if(BUILD_STRING_UTILITIES)
	    add_subdirectory(StringUtilities)
endif()

add_subdirectory(MainApp)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

if(BUILD_MATH_FUNCTIONS)
	install(TARGETS MathFunctions
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		RUNTIME DESTINATION bin
	)
	install(FILES MathFunctions/MathFunctions.h DESTINATION include)
endif()

if(BUILD_STRING_UTILITIES)
	install(TARGETS StringUtilities
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		RUNTIME DESTINATION bin
	)
	install(FILES StringUtilities/StringUtilities.h DESTINATION include)
endif()

install(TARGETS MainApp
	RUNTIME DESTINATION bin
)
