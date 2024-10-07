# Создаем библиотеку StringUtilities
add_library(StringUtilities StringUtilities.cpp)

# Указываем, что заголовочные файлы находятся в текущей директории
target_include_directories(StringUtilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
