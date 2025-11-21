# LLM + GGUF

* **Веса LLM** бывают в разных форматах. Для локального быстрого инференса через стек **ggml/llama.cpp** используется **GGUF** — один файл, часто **квантованный** (Q4/Q5…), чтобы занимать меньше памяти и работать быстрее.
* **Квантовка:** `Q4_K_M` — хороший баланс скорость/качество; `Q5_K_M` — лучше качество, больше размер.
* **Заменяемость:** в llama.cpp/llama-cpp-python «сменить модель» = просто поменять **путь к `.gguf`**. Поддерживаются Qwen, Llama, Mistral, Gemma и др., если есть GGUF-сборка.

> **Что значит «квантованный»:** веса нейросети сохранены не в стандартной *плавающей точке* (FP16/FP32), а в **меньшем числе бит** — например, 4/5/8 бит. Это резко уменьшает размер и ускоряет инференс на CPU/Metal/CUDA, с небольшой (зависящей от схемы) потерей качества.

Полезные модели:

* [Hugging Face: Qwen2.5-0.5B Instruct (GGUF)](https://huggingface.co/bartowski/Qwen2.5-0.5B-Instruct-GGUF)
* [Hugging Face: TinyLlama 1.1B Chat](https://huggingface.co/TinyLlama/TinyLlama-1.1B-Chat-v1.0)

---

# Пример 1 — Python (llama-cpp-python)

```python
# file: example01.py
from pathlib import Path
from contextlib import redirect_stdout, redirect_stderr
from llama_cpp import Llama
import sys, os, platform

MODEL_PATH = Path("models/qwen2.5-0.5b-instruct-q4_k_m.gguf")  # путь к модели

def main(prompt: str):
    if not MODEL_PATH.exists():
        raise FileNotFoundError(f"Нет файла модели: {MODEL_PATH.resolve()}")

    # На macOS (Apple Silicon) оффлоадим слои на GPU (Metal)
    n_gpu_layers = 999 if platform.system() == "Darwin" else 0

    # тихо загружаем
    with open(os.devnull, "w") as f, redirect_stdout(f), redirect_stderr(f):
        llm = Llama(
            model_path=str(MODEL_PATH),
            n_ctx=4096,          # можно снизить до 3072/2048 при нехватке RAM
            n_threads=4,         # CPU потоки
            n_gpu_layers=n_gpu_layers,
            verbose=False
        )

    messages = [
        {"role": "system", "content": "You are a helpful coding assistant."},
        {"role": "user",   "content": prompt}
    ]

    res = llm.create_chat_completion(
        messages=messages,
        max_tokens=300,
        temperature=0.7,
        top_p=0.9
    )
    print(res["choices"][0]["message"]["content"].strip())

if __name__ == "__main__":
    # промпт из аргумента или stdin
    if len(sys.argv) > 1:
        user_prompt = " ".join(sys.argv[1:])
    else:
        data = sys.stdin.read().strip()
        if not data:
            print('Usage: python example01.py "your question here"')
            sys.exit(2)
        user_prompt = data
    main(user_prompt)
```

Запуск:

```bash
python example01.py "Create a short C++ RAII example"
```

**Поменять модель:** просто укажите другой `.gguf` в `MODEL_PATH`.

---

# Пример 2 — Локальный CLI (llama-cli)

```bash
# Qwen 2.5 0.5B Instruct (ChatML-подобная разметка)
./llama.cpp/build/bin/llama-cli \
  -m models/qwen2.5-0.5b-instruct-q4_k_m.gguf \
  -c 4096 -ngl 999 \
  -p $'<|im_start|>system
You are a helpful assistant.
<|im_end|>
<|im_start|>user
Привет! Скажи три идеи для проекта на C++.
<|im_end|>
<|im_start|>assistant
'

# Другая модель — просто меняем путь:
./llama.cpp/build/bin/llama-cli \
  -m models/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf \
  -c 2048 -ngl 999 \
  -p "Дай три идеи для проекта на C++."
```

> На не-macOS поставьте `-ngl 0` (или уберите), если нет GPU-оффлоада.

---

# Пример 3 — HTTP-запрос к локальному llama-server (OpenAI-совместимый)

Идея: поднимаем локальный **llama-server** на GGUF-модели и шлём запрос `curl`.

## 3.0 Сборка сервера (один раз)

```bash
git clone https://github.com/ggerganov/llama.cpp
cd llama.cpp
cmake -B build -DCMAKE_BUILD_TYPE=Release -DGGML_METAL=ON -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release -- -j$(sysctl -n hw.ncpu)
```

Сложите модели (GGUF) в `models/`:

```
models/qwen2.5-0.5b-instruct-q4_k_m.gguf
models/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf
```

## 3.1 Запустить сервер (терминал №1)

```bash
./llama.cpp/build/bin/llama-server \
  -m models/qwen2.5-0.5b-instruct-q4_k_m.gguf \
  -c 4096 -ngl 999 \
  --host 127.0.0.1 --port 8080
```

## 3.2 Запрос из `curl` (терминал №2)

```bash
curl -s http://127.0.0.1:8080/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "model": "local-gguf",
    "messages": [
      {"role":"system","content":"You are a helpful coding assistant."},
      {"role":"user","content":"Поясни в двух предложениях, что такое цикл в C++."}
    ],
    "max_tokens": 200,
    "temperature": 0.7,
    "top_p": 0.9
  }' | jq -r '.choices[0].message.content'
```

**Поменять модель:** остановите сервер и запустите его с другим `-m path/to/other.gguf`.

# Домашнее задание

* Перенастроить вашего ИИ-ассистента на работу с **локальной моделью** (GGUF через `llama.cpp`/`llama-cpp-python`).
* Реализовать возможность **переключать тип модели** через **конфигурационный файл**.
* Добавить выбор источника инференса: **локальная модель** или **удалённый (hosted) API** — например, Hurated API.
