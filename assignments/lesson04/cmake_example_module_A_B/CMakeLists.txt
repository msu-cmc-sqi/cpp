cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Установка стандартов C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Опция для сборки модуля A
option(BUILD_MODULE_A "Build ModuleA library" ON)

# Опция для сборки модуля B
option(BUILD_MODULE_B "Build ModuleB library" ON)

# Добавляем модуль A, если опция включена
if (BUILD_MODULE_A)
    add_subdirectory(ModuleA)
    add_definitions(-DHAVE_MODULE_A)
endif()

# Добавляем модуль B, если опция включена
if (BUILD_MODULE_B)
    add_subdirectory(ModuleB)
    add_definitions(-DHAVE_MODULE_B)
endif()

# Главный исполняемый файл
add_executable(MyProject main.cpp)

# Условная линковка модулей
if (BUILD_MODULE_A)
    target_link_libraries(MyProject ModuleA)
endif()

if (BUILD_MODULE_B)
    target_link_libraries(MyProject ModuleB)
endif()

# Указываем, что нужно установить главный исполняемый файл
install(TARGETS MyProject DESTINATION bin)

# Если есть библиотеки или другие файлы, которые нужно установить:
if (BUILD_MODULE_A)
    install(TARGETS ModuleA DESTINATION lib)
    install(FILES ModuleA/ModuleA.h DESTINATION include)
endif()

if (BUILD_MODULE_B)
    install(TARGETS ModuleB DESTINATION lib)
    install(FILES ModuleB/ModuleB.h DESTINATION include)
endif()
