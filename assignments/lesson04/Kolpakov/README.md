# model_request — прототип CLI для подготовки запроса к локальной модели

CLI принимает путь к JSON-конфигурации и текст пользователя, проверяет конфигурацию
и печатает в stdout JSON-запрос с массивом `messages`.

## Требования

- CMake ≥ 3.20
- Компилятор с поддержкой C++17 (GCC, Clang, MSVC)
- Сетевой доступ при **первой** конфигурации (для `FetchContent`)

## Получение JSON-библиотеки

Используется `nlohmann::json` v3.11.3 через `FetchContent`. Версия закреплена тегом,
повторная конфигурация не требует сети, если кеш уже получен.

## Команды

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/model_request config.example.json "Привет"
```

Для Visual Studio / Xcode:

```bash
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
./build/Debug/model_request config.example.json "Привет"
```

## Пример запуска

Вход: `config.example.json` из корня проекта и текст `"Привет"`.

Ожидаемый вывод (stdout):

```json
{
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

При неверном числе аргументов CLI печатает подсказку и возвращает код 2.
Ошибки файла, синтаксиса JSON и структуры конфигурации печатаются в stderr
с различимыми префиксами (`file error`, `syntax error`, `config error`)
и возвращают код 1.

## Граф целей

```
config_core (статическая библиотека)
├── src/model_config.cpp
├── include/model_config.hpp      (PUBLIC include)
└── nlohmann_json::nlohmann_json  (PUBLIC)

model_request (исполняемая цель) — PRIVATE → config_core
config_test   (исполняемая цель) — PRIVATE → config_core
```

Тест `config_test` покрывает: корректную конфигурацию, пропущенное поле,
неверный тип (`context_size` дробный, `model = null`, строка вместо числа),
число вне диапазона (`context_size = 0`, `temperature = 3.0`, `-0.1`),
пустую строку модели, синтаксическую ошибку JSON и структуру запроса.

## Замечания

- Тесты используют `assert` — собирайте в `Debug` (без `NDEBUG`).
- Каталог `build/` в git не добавляется.
