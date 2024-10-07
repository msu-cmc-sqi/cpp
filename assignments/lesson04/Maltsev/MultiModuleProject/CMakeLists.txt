cmake_minimum_required(VERSION 3.10)

# Название проекта
project(MultiModuleProject)

# Указываем стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Подключаем подмодули проекта
add_subdirectory(MathFunctions)
add_subdirectory(StringUtilities)
add_subdirectory(MainApp)
