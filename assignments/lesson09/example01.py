from pathlib import Path
from contextlib import redirect_stdout, redirect_stderr
from llama_cpp import Llama
import sys, os, platform

MODEL_PATH = Path("models/qwen2.5-0.5b-instruct-q4_k_m.gguf")  # путь к вашей модели

def main(prompt: str):
    if not MODEL_PATH.exists():
        raise FileNotFoundError(f"Нет файла модели: {MODEL_PATH.resolve()}")

    # На macOS (Apple Silicon) выгружаем побольше слоёв на GPU
    n_gpu_layers = 999 if platform.system() == "Darwin" else 0

    # тихо загружаем
    with open(os.devnull, "w") as f, redirect_stdout(f), redirect_stderr(f):
        llm = Llama(
            model_path=str(MODEL_PATH),
            n_ctx=4096,          # можно уменьшить до 3072/2048 при нехватке RAM
            n_threads=4,         # CPU потоки (не критично)
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
    # промпт берём из аргументов или stdin
    if len(sys.argv) > 1:
        user_prompt = " ".join(sys.argv[1:])
    else:
        data = sys.stdin.read().strip()
        if not data:
            print("Usage: python run_mistral.py \"your question here\"")
            sys.exit(2)
        user_prompt = data
    main(user_prompt)
