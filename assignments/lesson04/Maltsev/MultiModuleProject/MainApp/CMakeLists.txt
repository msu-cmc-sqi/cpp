# Создаем исполняемый файл main из main.cpp
add_executable(MainApp main.cpp)

# Указываем, что исполняемый файл MainApp зависит от библиотек MathFunctions и StringUtilities
target_link_libraries(MainApp PRIVATE MathFunctions StringUtilities)
