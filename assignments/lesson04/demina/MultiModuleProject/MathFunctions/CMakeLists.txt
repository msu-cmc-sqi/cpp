add_library(MathFunctions MathFunctions.cpp)

install(TARGETS MathFunctions
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)
install(FILES MathFunctions.h DESTINATION include)
