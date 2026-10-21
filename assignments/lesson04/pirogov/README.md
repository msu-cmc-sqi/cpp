# Урок 4 — Pirogov

Программа читает конфигурацию локальной модели из JSON-файла, проверяет её и создаёт JSON-запрос с сообщением пользователя. Подключение к модели и интернет для запуска программы не нужны.

## Требования

- CMake 3.20 или новее;
- компилятор с поддержкой C++17;
- Git и доступ к интернету при первой настройке проекта.

Библиотека `nlohmann_json` версии 3.11.3 загружается через `FetchContent` при первом запуске CMake.

## Структура проекта

- `request_core` — библиотека из `src/model_request.cpp`;
- `model_request` — программа из `src/main.cpp`;
- `request_test` — тест из `tests/model_request_test.cpp`.

В `include/model_request.hpp` находятся объявления структуры конфигурации и функций. В `src/model_request.cpp` находятся проверка конфигурации и создание запроса. Файл `src/main.cpp` работает с аргументами командной строки, читает файл и выводит ошибки.

Файл `src/utf8.manifest` включает UTF-8 для аргументов командной строки в Windows. В других операционных системах он не используется.

## Сборка и тестирование

Команды нужно выполнять из каталога `assignments/lesson04/pirogov`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Для Visual Studio:

```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```



## Запуск

Linux, macOS, MinGW или Ninja:

```bash
./build/model_request config.example.json "Привет"
```

Visual Studio:

```powershell
.\build\Debug\model_request.exe config.example.json "Привет"
```

Ожидаемый результат:

```json
{
  "context_size": 2048,
  "messages": [
    {
      "content": "Привет",
      "role": "user"
    }
  ],
  "model": "local-model.gguf",
  "temperature": 0.7
}
```

Размер контекста должен быть от 1 до 131072. Температура должна быть от 0 до 2. Имя модели не может быть пустым.

Пример ошибочного запуска:

```bash
./build/model_request missing.json "Привет"
```

Программа напечатает понятное сообщение об ошибке и завершится с ненулевым кодом.