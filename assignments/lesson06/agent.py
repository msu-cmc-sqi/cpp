import os
import json
import shlex
import subprocess

from pathlib import Path
from dotenv import load_dotenv
from openai import OpenAI


# Загружаем настройки из .env
load_dotenv()

PROJECT_ROOT = Path.cwd().resolve()

API_KEY = os.environ["OPENAI_API_KEY"]
BASE_URL = os.environ["OPENAI_BASE_URL"]
MODEL = "qwen38_27b"


# Создаём клиента для OpenAI-compatible API
client = OpenAI(
    api_key=API_KEY,
    base_url=BASE_URL,
)


# ----------------------------
# Ограничения безопасности
# ----------------------------

ALLOWED_COMMANDS = {
    "ls",
    "find",
    "cat",
    "cmake",
    "ctest",
    "make",
    "ninja",
    "g++",
    "clang++",
    "gcc",
    "clang",
}

FORBIDDEN_FIND_OPTIONS = {
    "-exec",
    "-execdir",
    "-ok",
    "-okdir",
    "-delete",
}

FORBIDDEN_SHELL_TOKENS = {
    "&&",
    "||",
    "|",
    ";",
    ">",
    "<",
    ">>",
    "2>",
    "2>>",
}

PROTECTED_PATHS = {
    ".env",
    "agent.py",
}


# ----------------------------
# Tool: запись файла
# ----------------------------

def write_file(path, content):
    file_path = (PROJECT_ROOT / path).resolve()

    # Запрещаем выход за пределы проекта
    if PROJECT_ROOT not in file_path.parents and file_path != PROJECT_ROOT:
        return f"ERROR: writing outside project directory is not allowed: {path}"

    if file_path.is_dir():
        return f"ERROR: path is a directory: {path}"

    try:
        relative_path = file_path.relative_to(PROJECT_ROOT)
    except ValueError:
        return f"ERROR: invalid path: {path}"

    # Не даём агенту менять служебные файлы
    if str(relative_path) in PROTECTED_PATHS:
        return f"ERROR: writing '{relative_path}' is not allowed"

    if ".git" in relative_path.parts:
        return "ERROR: writing inside .git is not allowed"

    if ".venv" in relative_path.parts:
        return "ERROR: writing inside .venv is not allowed"

    try:
        file_path.parent.mkdir(parents=True, exist_ok=True)
        file_path.write_text(content, encoding="utf-8")
    except Exception as e:
        return f"ERROR: cannot write file: {e}"

    return f"OK: file '{relative_path}' written successfully"


# ----------------------------
# Tool: выполнение команды
# ----------------------------

def run_command(command):
    try:
        args = shlex.split(command)
    except ValueError as e:
        return f"ERROR: invalid command: {e}"

    if not args:
        return "ERROR: empty command"

    for arg in args:
        if arg in FORBIDDEN_SHELL_TOKENS or \
        arg.startswith(">") or \
        arg.startswith("<") or \
        arg.startswith("2>"):
            return (
                f"ERROR: shell operator '{arg}' is not allowed. "
                "Run commands one at a time."
            )

    program = args[0]

    # Разрешаем запуск исполняемых файлов только из каталога build/
    if program.startswith("./build/") or program.startswith("build/"):
        build_root = (PROJECT_ROOT / "build").resolve()
        executable = (PROJECT_ROOT / program).resolve()

        # Файл обязан находиться внутри build/
        if build_root not in executable.parents:
            return "ERROR: executable must be inside build/"

        if not executable.exists():
            return f"ERROR: executable not found: {program}"

        if not executable.is_file():
            return f"ERROR: not a file: {program}"

        if not os.access(executable, os.X_OK):
            return f"ERROR: file is not executable: {program}"

        try:
            result = subprocess.run(
                [str(executable), *args[1:]],
                cwd=PROJECT_ROOT,
                capture_output=True,
                text=True,
                timeout=30
            )
        except subprocess.TimeoutExpired:
            return "ERROR: program timed out after 30 seconds"
        except Exception as e:
            return f"ERROR: program failed: {e}"

        return (
            f"exit_code={result.returncode}\n"
            f"stdout:\n{result.stdout}\n"
            f"stderr:\n{result.stderr}"
        )

    # Все остальные команды должны быть в white list
    if program not in ALLOWED_COMMANDS:
        return f"ERROR: command '{program}' is not allowed"

    # У find есть опасные параметры
    if program == "find":
        for arg in args[1:]:
            if arg in FORBIDDEN_FIND_OPTIONS:
                return f"ERROR: unsafe find option '{arg}' is not allowed"

    try:
        result = subprocess.run(
            args,
            cwd=PROJECT_ROOT,
            capture_output=True,
            text=True,
            timeout=30
        )
    except subprocess.TimeoutExpired:
        return "ERROR: command timed out after 30 seconds"
    except Exception as e:
        return f"ERROR: command failed: {e}"

    return (
        f"exit_code={result.returncode}\n"
        f"stdout:\n{result.stdout}\n"
        f"stderr:\n{result.stderr}"
    )


# ----------------------------
# Описание tools для модели
# ----------------------------

tools = [
    {
        "type": "function",
        "function": {
            "name": "run_command",
            "description": (
                "Run one allowed command in the current project directory. "
                "Call only one command at a time. "
                "Do not use shell operators such as &&, ;, |, >, <."
            ),
            "parameters": {
                "type": "object",
                "properties": {
                    "command": {
                        "type": "string",
                        "description": "One command to execute"
                    }
                },
                "required": ["command"]
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "write_file",
            "description": (
                "Write complete contents to a file inside the current project. "
                "Use this tool when source code or project files need to be changed."
            ),
            "parameters": {
                "type": "object",
                "properties": {
                    "path": {
                        "type": "string",
                        "description": "Path relative to the project directory"
                    },
                    "content": {
                        "type": "string",
                        "description": "Complete new contents of the file"
                    }
                },
                "required": ["path", "content"]
            }
        }
    }
]


# ----------------------------
# Начальная история диалога
# ----------------------------

user_prompt = input("Что нужно сделать? ").strip()

if not user_prompt:
    raise RuntimeError("Задача не задана")

messages = [
    {
        "role": "system",
        "content": (
            "You are a coding agent. "
            "Use tools to inspect, modify, build and test the project. "
            "Use run_command for allowed commands. "
            "Use write_file when you need to modify a file. "
            "Call commands one at a time. "
            "Do not use shell operators such as &&, ;, |, > or <. "
            "Executable programs may only be run from the build/ directory. "
            "Always verify changes by rebuilding or running tests."
        )
    },
    {
        "role": "user",
        "content": user_prompt
    }
]


# Чтобы агент не ушёл в бесконечный цикл
max_steps = 10


# ----------------------------
# Agent loop
# ----------------------------

for step in range(max_steps):
    print(f"\n=== STEP {step + 1} ===")

    response = client.chat.completions.create(
        model=MODEL,
        messages=messages,
        tools=tools,
        tool_choice="auto",
    )

    message = response.choices[0].message

    # Добавляем ответ модели в историю
    messages.append(message)

    # Если tool call нет, агент закончил работу
    if not message.tool_calls:
        print("\nFINAL RESPONSE:\n")
        print(message.content)
        break

    # Модель может запросить несколько tools
    for tool_call in message.tool_calls:

        try:
            args = json.loads(tool_call.function.arguments)
        except json.JSONDecodeError as e:
            result = f"ERROR: invalid tool arguments: {e}"

            messages.append({
                "role": "tool",
                "tool_call_id": tool_call.id,
                "content": result
            })
            continue

        if tool_call.function.name == "run_command":
            command = args["command"]

            print(f"\nRUNNING: {command}\n")

            result = run_command(command)

        elif tool_call.function.name == "write_file":
            path = args["path"]
            content = args["content"]

            print(f"\nWRITING FILE: {path}\n")

            result = write_file(path, content)

        else:
            result = f"ERROR: unknown tool '{tool_call.function.name}'"

        print(result)

        # Возвращаем результат tool обратно модели
        messages.append({
            "role": "tool",
            "tool_call_id": tool_call.id,
            "content": result
        })

else:
    print("\nAgent stopped: maximum number of steps reached.")