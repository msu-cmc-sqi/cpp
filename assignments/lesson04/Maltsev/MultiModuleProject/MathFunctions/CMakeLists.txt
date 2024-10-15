# Создаем библиотеку MathFunctions
add_library(MathFunctions MathFunctions.cpp)

# Указываем, что заголовочные файлы находятся в текущей директории
target_include_directories(MathFunctions PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
