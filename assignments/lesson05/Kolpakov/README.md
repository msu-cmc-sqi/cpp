# lesson05: локальный чат с контролем контекста

C++17 CLI-клиент к `llama-server` (llama.cpp). Сценарий — **чат с памятью**:
история хранится в `std::vector<Message>` и отправляется на сервер при каждом
ходе. Перед генерацией клиент считает вход через
`POST /v1/chat/completions/input_tokens` и при нехватке места удаляет самые
старые завершённые пары `user`–`assistant`, сохраняя `system` и текущую реплику.

## Ноутбук

- **ОС:** `<заполнить>` (например, Ubuntu 24.04)
- **CPU:** `<заполнить>` (например, Intel Core i5-1135G7)
- **RAM:** `<заполнить>` (например, 16 ГБ)
- **GPU:** `<заполнить>` (например, нет / Intel Iris Xe, используется CPU-only запуск, `-ngl 0`)

## Модель и сервер

- **Модель:** [Qwen2.5-0.5B-Instruct-GGUF](https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF/tree/main)
- **Файл:** `qwen2.5-0.5b-instruct-q4_k_m.gguf`
- **Квантование:** `Q4_K_M` (~491 МБ)
- **Версия сервера (`llama-server --version`):** `<заполнить>`
- **Коммит llama.cpp (`git rev-parse HEAD`):** `<заполнить>`

Запуск сервера (в отдельном терминале, из каталога `llama.cpp`):

```bash
./build/bin/llama-server \
  -m models/qwen2.5-0.5b-instruct-q4_k_m.gguf \
  --alias local-model \
  -c 1024 -ngl 0 -t 2 -np 1 \
  --host 127.0.0.1 --port 8080
