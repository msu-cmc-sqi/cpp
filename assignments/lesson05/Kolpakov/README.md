# lesson05: локальный чат с контролем контекста

C++17 CLI-клиент к `llama-server` (llama.cpp). Сценарий — **чат с памятью**:
история хранится в `std::vector<Message>` и отправляется на сервер при каждом
ходе. Перед генерацией клиент считает вход через
`POST /v1/chat/completions/input_tokens` и при нехватке места удаляет самые
старые завершённые пары `user`–`assistant`, сохраняя `system` и текущую реплику.

## Ноутбук

- **ОС:** `Arch Linux`
- **CPU:** `AMD Ryzen 5 5600H with Radeon Graphics` 
- **RAM:** `16 Gb`
- **GPU:** `нет` 

## Модель и сервер

- **Модель:** [Qwen2.5-0.5B-Instruct-GGUF](https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF/tree/main)
- **Файл:** `qwen2.5-0.5b-instruct-q4_k_m.gguf`
- **Квантование:** `Q4_K_M` (~491 МБ)
- **Версия сервера (`version: 0.7.0-dev (build 11784, commit 493253fe5)
built with GNU 16.2.1 for Linux x86_64`):** 
- **Коммит llama.cpp (`493253fe5a5af7e800b3c92a2452b5684a99e961`):** 

Запуск сервера (в отдельном терминале, из каталога `llama.cpp`):

```bash
./build/bin/llama-server \
  -m models/qwen2.5-0.5b-instruct-q4_k_m.gguf \
  --alias local-model \
  -c 1024 -ngl 0 -t 2 -np 1 \
  --host 127.0.0.1 --port 8080
