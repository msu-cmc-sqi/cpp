# Model Request

CLI-приложение читает JSON-конфигурацию локальной модели, проверяет её и
печатает подготовленный JSON-запрос с сообщением пользователя. Сеть и
сама модель не нужны.

## Требования

- CMake 3.20 или новее;
- компилятор с поддержкой C++17;
- Git и доступ к интернету при первой конфигурации.

Библиотека `nlohmann_json` версии 3.11.3 загружается CMake через `FetchContent`.
Версия закреплена тегом `v3.11.3`.

## Формат конфигурации

Корень JSON должен быть объектом с тремя обязательными полями:

- `model` — непустая строка;
- `context_size` — целое число от 1 до 131072;
- `temperature` — число от 0 до 2 включительно.

Пример находится в `config.example.json`.

## Сборка и тесты

Из этого каталога:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

`--config Debug` и `-C Debug` нужны многоконфигурационным генераторам, например Visual Studio;
для Make и Ninja они безовредны.

## Запуск

Linux, macOS, Make или Ninja:

```bash
./build/model_request config.example.json "Привет"
```

Windows с Visual Studio:

```powershell
.\build\Debug\model_request.exe config.example.json "Привет"
```

Ожидаемый результат:

```json
{
  "messages": [
    {
      "content": "Привет",
      "role": "user"
    }
  ],
  "model": "qwen2.5:3b",
  "options": {
    "num_ctx": 4096,
    "temperature": 0.7
  },
  "stream": false
}
```

При ошибке программа печатает понятное сообщение в `stderr` и возвращает
ненулевой код. Ошибки открытия/чтения файла, JSON-синтаксиса и структуры конфигурации
различаются в тексте сообщения.

## Цели и файлы

```text
model_request_core (src/model_request.cpp, include/model_request.hpp)
        ↑                           ↑
model_request (src/main.cpp)   model_request_test (tests/model_request_test.cpp)
```

`model_request_core` хранит валидацию и сборку JSON-запроса. CLI отвечает только за аргументы,
чтение файла, вывод и обработку ошибок. Тест связан с той же библиотекой и не использует сеть
или модель.
