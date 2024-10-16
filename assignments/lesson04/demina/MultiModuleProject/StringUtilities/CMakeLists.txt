add_library(StringUtilities StringUtilities.cpp)

install(TARGETS StringUtilities
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)
install(FILES StringUtilities.h DESTINATION include)
