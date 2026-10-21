# Домашнее задание 5: Локальная LLM (llama.cpp, Hugging Face, HTTP-клиент C++17)

**Студент:** Куликов Владимир (`yowukk`)  
**Каталог:** `assignments/lesson05/kulikov/`

---

## 1. Характеристики рабочей машины

- **ОС:** Linux (Ubuntu 24.04 LTS, WSL2 x86_64)
- **Процессор:** Intel(R) Core(TM) Ultra 5 125H
- **Оперативная память:** 16 ГБ
- **GPU:** Не используется (только CPU, `-ngl 0`, 2 потока `-t 2`)
- **Компилятор и сборка:** `g++ 13.3.0`, `CMake 3.20+`

---

## 2. Модель и запуск сервера

- **Модель на Hugging Face:** [Qwen/Qwen2.5-0.5B-Instruct-GGUF](https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF)
- **Файл весов:** `qwen2.5-0.5b-instruct-q4_k_m.gguf`
- **Квантование:** `Q4_K_M` (~469 МБ)
- **Версия сервера:** `llama-server` (билд b11785, commit `8baabd53b`)

### Команда запуска сервера
Сервер и модель хранятся в домашней папке пользователя `~/llama.cpp/` (вне каталога репозитория):
```bash
~/llama.cpp/llama-b11785/llama-server \
  -m ~/llama.cpp/models/qwen2.5-0.5b-instruct-q4_k_m.gguf \
  --alias local-model \
  -c 1024 -ngl 0 -t 2 -np 1 \
  --host 127.0.0.1 --port 8080
```

Проверка доступности:
```bash
curl -i http://127.0.0.1:8080/health
# HTTP/1.1 200 OK -> {"status":"ok"}
```

---

## 3. Устройство C++17 клиента

Проект состоит из двух исходных файлов и одного заголовка:
- **`include/client.hpp`** — структуры `Config` и `Response`, объявление RAII-обёртки `CurlRuntime` и функции `send_chat_completion`.
- **`src/client.cpp`** — построение JSON-запроса через `nlohmann::json`, выполнение HTTP POST через `libcurl` с конечным таймаутом, проверка HTTP-статуса и парсинг `choices[0].message.content` и `usage`.
- **`src/main.cpp`** — точка входа CLI: принимает запрос из аргументов командной строки или из `stdin`, выводит результат и статистику токенов.

### Структура запроса к API
В `send_chat_completion` запрос формируется через библиотеку JSON:
```json
{
  "model": "local-model",
  "messages": [
    {"role": "user", "content": "..."}
  ],
  "max_tokens": 64,
  "stream": false
}
```
- `role`: роль участника диалога (`user` — запрос пользователя).
- `content`: текст реплики.
- `choices[0].message.content`: текст сгенерированного ответа модели.
- `usage`: количество токенов (`prompt_tokens`, `completion_tokens`, `total_tokens`).

---

## 4. Сборка и запуск

### Сборка
```bash
cd assignments/lesson05/kulikov
cmake -B build -S .
cmake --build build -j
```

### Запуск и самопроверка
1. **Передача запроса аргументом:**
   ```bash
   ./build/llm_client "Привет! Назови столицу Франции."
   ```
2. **Передача запроса через интерактивный ввод или пайп:**
   ```bash
   ./build/llm_client
   # Ваш запрос: Что такое std::vector в C++ одним предложением?
   ```
3. **С указанием собственного файла конфигурации:**
   ```bash
   ./build/llm_client --config config.example.json "Ваш вопрос"
   ```

---

## 5. Примеры работы

Файлы сохранены в папке `examples/`:

1. **[examples/example1.json](examples/example1.json):**
   - **Запрос:** `Привет! Назови столицу Франции.`
   - **Ответ модели:** `Привет! Столицей Франции является Мюнхен.`
   - **Токены:** `вход: 44, ответ: 20, всего: 64`
   - **Оценка:** Успешный HTTP-запрос и разбор ответа. Из-за малого размера модели (0.5B) возникла фактологическая ошибка при связном русском тексте.

2. **[examples/example2.json](examples/example2.json):**
   - **Запрос:** `Что такое std::vector в C++ одним предложением?`
   - **Ответ модели:** `std::vector в C++ — это класс встроенного набора данных...`
   - **Токены:** `вход: 43, ответ: 64, всего: 107`
   - **Оценка:** Модель определила контекст C++ и STL. Генерация достигла лимита `max_tokens = 64`.

3. **[examples/example3_boundary.json](examples/example3_boundary.json):**
   - **Пограничные случаи и ошибки:**
     - Пустой ввод пользователя -> `Ошибка: Введите непустую строку запроса` (код 1).
     - Остановленный сервер -> `Ошибка: Сервер недоступен. Проверьте запуск llama-server по адресу http://127.0.0.1:8080` (код 1).
