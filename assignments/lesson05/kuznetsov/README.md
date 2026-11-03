# C++ LLM Клиент 

Консольное C++17 приложение, которое отправляет структурированный JSON-запрос по протоколу HTTP (с использованием `libcurl`) к локальному серверу `llama-server`, получает ответ от GGUF-модели и выводит его в терминал. Проект выбран по сценарию **«Помощник по C++»**.

---

## 1. Аппаратное и программное обеспечение
* **Операционная система:** macOS 14 Sonoma
* **Процессор (CPU):** Apple M3 CPU 8-core, 4 p-ядра, 4 e-ядра
* **Оперативная память (RAM):**: 8 ГБ
* **Использование GPU:** нет

---

## 2. Информация о модели
* **Название модели:** Qwen2.5-0.5B-Instruct-GGUF
* **Ссылка на Hugging Face:** [https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF](https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF)
* **Имя GGUF-файла:** `qwen2.5-0.5b-instruct-q4_k_m.gguf`
* **Квантование:** `Q4_K_M`
* **Размер файла:** ~491 МБ

---

## 3. Версия сервера и запуск
* **Версия llama-server:**:
  ```
  llama.cpp % ./build/bin/llama-server --version
  version: 0.6.0-dev (build 11606, commit 1b70350c9)
  built with AppleClang 16.0.0.16000026 for Darwin arm64
  ```
* **Команда запуска сервера:**
  ```bash
  ./build/bin/llama-server \
    -m models/qwen2.5-0.5b-instruct-q4_k_m.gguf \
    --alias local-model \
    -c 1024 -ngl 0 -t 2 -np 1 \
    --host 127.0.0.1 --port 8080
    ```
---
## Примеры ответов:
      --- Model Response ---
      Вот упрощенная версия кода, сокращая переменные и определяющие значения:

      ```cpp
      #include <nlohmann/json.hpp>

      int main() {
          std::string config_json_str = "config.json";  // Пример JSON файла
          auto j = nlohmann::json::parse(config_json_str);  // Пример JSON файла
          return 0;
      }
      ```

      ### Описание:
      1. **`auto j = nlohmann::json::parse(config_json_str);`**:
          - `nlohmann::json::parse` — это функция для преобразования JSON-кода в объект `nlohmann::json`.
          - `config_json_str` — это строка с JSON-кодом.
          - `j` — это переменная, которая будет содержать преобразованное JSON-объект.
      2. **`nlohmann::json::parse`**:
          - `nlohmann::json::parse` — это функция, которая принимает строку JSON-кода и возвращает объект `nlohmann::json`.
          - `config_json_str` — это строка, содержащая JSON-код.
      3. **`return 0;`**:
          - Эта строка завершает выполнение скрипта и приводит к завершению функции.

      ### Объяснение:
      - **`nlohmann::json::parse(config_json_str)`**:
          - `nlohmann::json` — это библиотека для работы с JSON-кодами.
          - `json::parse` — это метод, который выполняет преобразование строкового JSON-кода в объект `nlohmann::json`.
          - `config_json_str` — это строка, содержащая JSON-код для преобразования.
      4. **`j`**:
          - `j` — это переменная, которая ссылается на результат преобразования `config_json_str` в `nlohmann::json`.
          - `j` — это переменная, которая содержит JSON-объект, преобразованный в объект `nlohmann::json`.

      Этот пример простой и понятный код для преобразования JSON-кода в `nlohmann::json` объект.
На вот такой промпт (с окном токенов в 512):
```
daniil_kuznetsov@MacBook-Air-Daniil-5 kuznetsov % ./build/llm_cli config.example.json "Что здесь происходит: auto j = nlohmann::json::parse(config_json_str);"
Sending request to http://127.0.0.1:8080/v1/chat/completions...
```

--------------