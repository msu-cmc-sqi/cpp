/*
 * Для компиляции необходимо добавить ключ -lncurses
 * g++ -o snake main.cpp -lncurses
 */

#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <ncurses.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <unistd.h>

// Направления движения змеи и команда завершения игры
enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME='q'};
// Ограничения и параметры игры
enum {MAX_TAIL_SIZE=1000, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10, SPEED=20000, SEED_NUMBER=3};

// Структура для представления сегмента хвоста змеи
struct Tail {
    int x, y;
};

// Структура для представления еды
struct Food {
    int x, y;             // Координаты еды
    time_t put_time;      // Время появления еды
    char point;           // Символ еды на экране
    bool enable;          // Доступность еды
};

// Класс Snake описывает поведение и характеристики змеи
class Snake {
public:
    int x, y, direction;           // Позиция головы змеи и направление движения
    size_t tsize;                  // Текущий размер хвоста
    char head, body;               // Символы головы и хвоста змеи
    std::vector<Tail> tail;        // Массив сегментов хвоста

    // Конструктор инициализирует параметры змеи
    Snake(int xx, int yy, char c1, char c2, int d) : x(xx), y(yy), direction(d), 
    head(c1), body(c2), tsize(START_TAIL_SIZE+1) {
        tail.resize(MAX_TAIL_SIZE);
    }

    // Перемещение головы змеи
    void move(std::mutex& M) {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x); // Получение размеров окна терминала
    
        M.lock();
        mvprintw(y, x, " "); // Стираем предыдущую позицию головы

        // Перемещаем голову змеи в зависимости от направления
        switch (direction) {
            case LEFT:
                if (x <= 0) x = max_x; // Если вышли за границу слева, перемещаем направо
                x--;
                break;
            case RIGHT:
                if (x >= max_x) x = 0; // Если вышли за границу справа, перемещаем налево
                x++;
                break;
            case UP:
                if (y <= 0) y = max_y; // Если вышли за верхнюю границу, перемещаем вниз
                y--;
                break;
            case DOWN:
                if (y >= max_y) y = 0; // Если вышли за нижнюю границу, перемещаем вверх
                y++;
                break;
        }
        mvprintw(y, x, "%c", head); // Рисуем голову змеи на новой позиции
        refresh();
        M.unlock();
    }

    // Перемещение хвоста змеи
    void moveTail(std::mutex& M) {
        M.lock();
        mvprintw(tail[tsize - 1].y, tail[tsize - 1].x, " "); // Стираем последний сегмент хвоста
        for (size_t i = tsize - 1; i > 0; i--) {
            tail[i] = tail[i - 1]; // Сдвигаем сегменты хвоста
            if (tail[i].y || tail[i].x) {
                mvprintw(tail[i].y, tail[i].x, "%c", body); // Рисуем каждый сегмент хвоста
            }
        }
        M.unlock();
        
        tail[0].x = x; // Голова становится первым сегментом хвоста
        tail[0].y = y;
    }

    // Увеличение длины хвоста
    void addTail() {
        if (tsize < MAX_TAIL_SIZE) {
            tsize++;
        } else {
            mvprintw(0, 0, "Can't add tail"); // Сообщение, если хвост достиг максимальной длины
        }
    }

    // Проверка на столкновение змеи с самой собой
    bool isCrash() {
        for (size_t i = 1; i < tsize; ++i) {
            if (x == tail[i].x && y == tail[i].y) return true;
        }
        return false;
    }
};

// Класс Game описывает игровую логику
class Game {
    std::vector<Food> food;         // Массив еды
    Snake computer, player;        // Компьютерная и пользовательская змеи
    std::mutex M;                  // Мьютекс для синхронизации потоков
    std::atomic<bool> run_cont = true; // Флаг, указывающий, идет ли игра
    int key_pressed;               // Последняя нажатая клавиша

    // Инициализация еды
    void init_food() {
        food.resize(MAX_FOOD_SIZE);
        for (auto& f : food) {
            f = {0, 0, 0, '$', false}; // Устанавливаем начальные параметры еды
        }
    }

    // Генерация новой еды в случайной позиции
    void putFoodSeed(Food& fp) {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        M.lock();
        mvprintw(fp.y, fp.x, " "); // Убираем старую еду
        fp.x = rand() % (max_x - 1);
        fp.y = rand() % (max_y - 3) + 2;
        fp.put_time = time(nullptr);
        fp.enable = true;
        mvprintw(fp.y, fp.x, "%c", fp.point); // Рисуем новую еду
        M.unlock();
    }

    // Размещение всей еды на поле
    void putFood() {
        for (auto& f : food) {
            putFoodSeed(f);
        }
        refresh();
    }

    // Обновление состояния еды
    void refreshFood() {
        for (auto& f : food) {
            if (f.put_time && (!f.enable || (time(nullptr) - f.put_time) > FOOD_EXPIRE_SECONDS)) {
                putFoodSeed(f);
            }
        }
    }

    // Проверка, съела ли змея еду
    bool haveEat(Snake& snake) {
        for (auto& f : food) {
            if (f.enable && snake.x == f.x && snake.y == f.y) {
                M.lock();
                f.enable = false; // Убираем еду после съедания
                M.unlock();
                return true;
            }
        }
        return false;
    }

    // Исправление еды, которая пересекается с телом змеи
    void repairSeed(Snake& snake) {
        for (size_t i = 0; i < snake.tsize; i++) {
            for (auto& f : food) {
                if (f.x == snake.tail[i].x && f.y == snake.tail[i].y && f.enable) {
                    putFoodSeed(f);
                }
            }
        }
    }

    // Проверка столкновения змей
    bool snakes_collided(Snake& snake) {
        // Проверка столкновения двух змей по сегментам их хвостов
        for (size_t i = 1; i < (snake.head == '*' ? computer.tsize : player.tsize); ++i) {
            if (snake.x == (snake.head == '*' ? computer.tail[i].x : player.tail[i].x) && 
                snake.y == (snake.head == '*' ? computer.tail[i].y : player.tail[i].y)) {
                return true;
            }
        }
        return false;
    }

    // Изменение направления игрока на основе нажатой клавиши
    void changeDirection(int key) {
        M.lock();
        switch (key) {
            case KEY_DOWN: if (player.direction != UP) player.direction = DOWN; break;
            case KEY_UP: if (player.direction != DOWN) player.direction = UP; break;
            case KEY_LEFT: if (player.direction != RIGHT) player.direction = LEFT; break;
            case KEY_RIGHT: if (player.direction != LEFT) player.direction = RIGHT; break;
        }
        M.unlock();
    }

    // Вывод уровней игроков
    void printLevel() {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        M.lock();
        mvprintw(0, max_x - 17, "COMPUTER LEVEL: %zu", computer.tsize);
        mvprintw(1, max_x - 15, "PLAYER LEVEL: %zu", player.tsize);
        refresh();
        M.unlock();
    }

    // Сообщение об окончании игры
    void printExit() {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        mvprintw(max_y / 2, max_x / 2 - 7, "Game Over");
        refresh();
        usleep(2000000);
    }

public:
    // Конструктор инициализация игры
    Game() : key_pressed(UP), 
             computer(20, 10, '*', '+', RIGHT), 
             player(10, 10, '#', '-', UP) {
        init_food();
    }

    // Запуск игры
    void run() {
        srand(static_cast<unsigned>(time(nullptr))); // Инициализация генератора случайных чисел
        initscr(); // Инициализация окна ncurses
        keypad(stdscr, TRUE);
        timeout(0); // Установка немедленного отклика на нажатие клавиш
        noecho(); // Отключение отображения символов ввода

        putFood();

        while (run_cont.load()) {
            usleep(SPEED); // Задержка скорости
            refreshFood(); // Обновление еды
            repairSeed(computer);
            repairSeed(player);

            if (key_pressed == STOP_GAME || computer.isCrash() || player.isCrash() ||
                snakes_collided(computer) || snakes_collided(player)) {
                run_cont.store(false);
                break;
            }

            printLevel();

            computer.move(M);
            computer.moveTail(M);
            if (haveEat(computer)) computer.addTail();

            player.move(M);
            player.moveTail(M);
            if (haveEat(player)) player.addTail();

            key_pressed = getch(); // Считывание нажатой клавиши
            if (key_pressed != ERR) changeDirection(key_pressed);
        }

        printExit();
        endwin();
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
