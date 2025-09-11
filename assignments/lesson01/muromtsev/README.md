# Настройка git

## 1. Создание и  добавление нового ssh ключа в профиль github
Для этого воспользовался инструкцией с этого раздела github: 
```
https://docs.github.com/en/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent
```
## 2. Сделал fork репозитория курса
## 3. Клонировал форк репозитория себе локально на компьютер с помощью команды:
```
git clone git@github.com:OlezhaMuromtsev/cpp.git
```
## 4. Создал ветку и перешел на нее:
```
git checkout -b add-hello-world-branch
```
## 5. Создал необходимые файлы локально в папке репозитория
## 6. Внес все изменения в индекс:
```
git add *
```
## 7. Закомиттил изменения:
```
git commit -m "добавил hello.cpp"
```
## 8. Запушил изменения в удаленный репозиторий
```
git push
```
## 9. Создал pull request на github.com