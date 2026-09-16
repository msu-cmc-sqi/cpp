# Lesson 01 — Хаммербек

## Настройка окружения
- Установлены git, g++ (или clang++), редактор/IDE.
- Настроены `user.name`, `user.email`, SSH-ключ добавлен в GitHub.

## Работа с Git
1. Создан репозиторий `git-practice` с README.md, склонирован локально.
2. Добавлен `.gitignore` для C++ (build-артефакты, файлы IDE).
3. Добавлены файлы, выполнен коммит и `git push`.
4. Воспроизведён конфликт: изменение `notes.txt` в `main` и `feature-conflict`, `git merge`, ручное разрешение, коммит.
5. Создана ветка `feature-readme`, отправлена на GitHub, открыт Pull Request.
6. Сделан fork `msu-cmc-sqi/cpp`, создана ветка `lesson01-Хаммербек`, добавлены README.md и hello.cpp, открыт PR.

## Сборка
g++ -std=c++17 hello.cpp -o hello && ./hello
