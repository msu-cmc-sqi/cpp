# CLI-прототип подготовки запроса к локальной модели

Приложение для валидации JSON-конфигурации локальной модели и формирования JSON-запроса к API.

## Структура проекта

```text
kuznetsov/
├── CMakeLists.txt         # Описание целей сборки и зависимостей
├── README.md              # Документация и инструкция
├── config.example.json    # Пример валидной конфигурации
├── include/
│   └── model_config.hpp   # Публичный интерфейс
├── src/
│   ├── model_config.cpp   # Логика проверки JSON и сборки запроса
│   └── main.cpp           # CLI-интерфейс
└── tests/
    └── config_test.cpp    # Автоматические тесты
```

## Зависимости и сборка
Внешняя библиотека nlohmann_json подключается автоматически во время конфигурации с помощью механизма CMake FetchContent (закрепена версия v3.11.3).

## Требования
* Компилятор с поддержкой C++17
* CMake версии 3.20 или выше

## Команды для сборки
* Конфигурация проекта:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```
* Сборка:
```bash
cmake --build build
```
* Запуск автоматических тестов:
```bash
ctest --test-dir build --output-on-failure
``` 
или 
```bash
./build/config_test
```

## Использование
Запуск приложения требует передачи пути к JSON-конфигурации и текста пользовательского промпта:
```bash
./build/model_request config.example.json "Привет"
```

## Пример входных данных (config.example.json):
```json
{
  "model": "local-model.gguf",
  "context_size": 2048,
  "temperature": 0.7
}
```
## Ожидаемый результат (стандартный вывод):
```json
{
    "messages": [
        {
            "content": "Привет",
            "role": "user"
        }
    ],
    "model": "local-model.gguf",
    "options": {
        "context_size": 2048,
        "temperature": 0.7
    }
}
```