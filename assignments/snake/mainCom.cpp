#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <ncurses.h>
#include <unistd.h>
#include <mutex>
#include <atomic>
#include <thread>

// Направления для движения змейки
enum { LEFT = 1, UP, RIGHT, DOWN, STOP_GAME = 'q' };
// Константы для игры (максимальный размер хвоста, начальный размер, максимальный размер еды, время жизни еды, скорость игры)
enum { MAX_TAIL_SIZE = 1000, START_TAIL_SIZE = 3, MAX_FOOD_SIZE = 20, FOOD_EXPIRE_SECONDS = 10, SPEED = 20000, SEED_NUMBER = 3 };

// Структура для представления части хвоста змейки
struct Tail {
    int x, y; // Координаты части хвоста
};

// Структура для еды, которая появляется на экране
struct Food {
    int x, y;       // Координаты еды
    time_t put_time; // Время, когда еда была поставлена
    char point;      // Символ, представляющий еду
    bool enable;     // Флаг, активна ли еда
};

// Класс змейки
class Snake {
public:
    int x, y, direction;  // Координаты головы змейки и направление движения
    size_t tsize;         // Размер хвоста
    char head, body;      // Символы для головы и тела змейки
    std::vector<Tail> tail; // Вектор частей хвоста змейки

    // Конструктор для инициализации змейки
    Snake(int xx, int yy, char c1, char c2, int d) : x(xx), y(yy), direction(d),
                                                     head(c1), body(c2), tsize(START_TAIL_SIZE + 1) {
        tail.resize(MAX_TAIL_SIZE); // Инициализация вектора хвоста
    }

    // Функция для перемещения змейки
    void move(std::mutex& M) {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x); // Получаем размеры окна

        M.lock(); // Захват мьютекса для синхронизации

        mvprintw(y, x, " "); // Убираем старое положение головы змейки

        // Перемещаем змейку в зависимости от направления
        switch (direction) {
            case LEFT: x = (x <= 0) ? max_x : x - 1; break;
            case RIGHT: x = (x >= max_x) ? 0 : x + 1; break;
            case UP: y = (y <= 0) ? max_y : y - 1; break;
            case DOWN: y = (y >= max_y) ? 0 : y + 1; break;
        }

        mvprintw(y, x, "%c", head); // Отображаем голову змейки
        refresh();
        M.unlock(); // Освобождаем мьютекс
    }

    // Функция для перемещения хвоста змейки
    void moveTail(std::mutex& M) {
        M.lock();
        mvprintw(tail[tsize - 1].y, tail[tsize - 1].x, " "); // Убираем последнюю часть хвоста
        for (size_t i = tsize - 1; i > 0; i--) {
            tail[i] = tail[i - 1]; // Сдвигаем хвост на одну позицию
            if (tail[i].y || tail[i].x) {
                mvprintw(tail[i].y, tail[i].x, "%c", body); // Отображаем тело змейки
            }
        }
        M.unlock();

        tail[0].x = x; // Перемещаем первую часть хвоста на позицию головы
        tail[0].y = y;
    }

    // Функция для добавления части хвоста
    void addTail() {
        if (tsize < MAX_TAIL_SIZE) {
            tsize++; // Увеличиваем размер хвоста, если это возможно
        }
    }

    // Функция для проверки, не врезалась ли змейка в себя
    bool isCrash() {
        for (size_t i = 1; i < tsize; ++i) {
            if (x == tail[i].x && y == tail[i].y) return true; // Если голова сталкивается с частью хвоста
        }
        return false;
    }

    // Функция для проверки, не столкнулась ли змейка с барьером
    bool hitBarrier(const std::vector<std::pair<int, int>>& barriers) {
        for (const auto& b : barriers) {
            if (x == b.first && y == b.second) return true; // Если голова сталкивается с барьером
        }
        return false;
    }
};

// Класс игры
class Game {
    std::vector<Food> food; // Вектор еды
    std::vector<std::pair<int, int>> barriers; // Вектор барьеров
    Snake computer, player; // Объекты змейки для компьютера и игрока
    std::mutex M; // Мьютекс для синхронизации
    std::atomic<bool> run_cont = true; // Флаг продолжения игры
    int key_pressed; // Нажатая клавиша

    // Инициализация еды
    void init_food() {
        food.resize(MAX_FOOD_SIZE);
        int max_y = 0, max_x = 0;
        getmaxyx(stdscr, max_y, max_x);
        for (auto& f : food) {
            f = {0, 0, 0, '$', false}; // Инициализация каждого элемента еды
        }
    }

    // Установка еды в случайное место
    void putFoodSeed(Food& fp) {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        M.lock();
        mvprintw(fp.y, fp.x, " "); // Убираем старое положение еды
        fp.x = rand() % (max_x - 1); // Генерируем новые координаты
        fp.y = rand() % (max_y - 3) + 2;
        fp.put_time = time(nullptr); // Записываем текущее время
        fp.enable = true; // Активируем еду
        mvprintw(fp.y, fp.x, "%c", fp.point); // Отображаем еду
        M.unlock();
    }

    // Установка всей еды на экране
    void putFood() {
        for (auto& f : food) {
            putFoodSeed(f); // Размещение еды
        }
        refresh();
    }

    // Обновление еды (время жизни еды)
    void refreshFood() {
        for (auto& f : food) {
            if (f.put_time && (!f.enable || (time(nullptr) - f.put_time) > FOOD_EXPIRE_SECONDS)) {
                putFoodSeed(f); // Если еда просрочена или неактивна, то перемещаем её
            }
        }
    }

    // Размещение барьеров на экране
    void placeBarriers() {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);

        barriers.clear(); // Очищаем список барьеров
        for (int i = 0; i < 10; ++i) {
            int bx = rand() % (max_x - 1);
            int by = rand() % (max_y - 3) + 2;
            barriers.emplace_back(bx, by); // Размещаем новые барьеры
        }

        // Отображаем барьеры
        for (const auto& b : barriers) {
            mvprintw(b.second, b.first, "#");
        }
        refresh();
    }

    // Случайная смена направления для компьютерной змейки
    void randomizeDirection(Snake& snake) {
        while (run_cont) {
            std::this_thread::sleep_for(std::chrono::seconds(5)); // Ожидание 5 секунд
            int new_direction;
            do {
                new_direction = rand() % 4 + 1; // Генерация нового направления
            } while ((snake.direction == LEFT && new_direction == RIGHT) ||
                     (snake.direction == RIGHT && new_direction == LEFT) ||
                     (snake.direction == UP && new_direction == DOWN) ||
                     (snake.direction == DOWN && new_direction == UP));

            M.lock();
            snake.direction = new_direction; // Обновление направления
            M.unlock();
        }
    }

    // Изменение направления змейки по клавише
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

    // Отображение уровня змейки
    void printLevel() {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        M.lock();
        mvprintw(0, max_x - 17, "COMPUTER LEVEL: %zu", computer.tsize);
        mvprintw(1, max_x - 15, "PLAYER LEVEL: %zu", player.tsize);
        refresh();
        M.unlock();
    }

    // Завершение игры
    void end_game() {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        mvprintw(max_y / 2, max_x / 2 - 10, "Game Over!"); // Сообщение о завершении игры
        mvprintw(max_y / 2 + 2, max_x / 2 - 10, "Press any key to exit...");
        timeout(SPEED); // Установка задержки
        getch(); // Ожидание нажатия клавиши
        endwin(); // Завершаем ncurses
    }

public:
    // Конструктор игры
    Game() : computer(0, 3, '@', '*', RIGHT), player(10, 15, '*', '+', LEFT) {
        key_pressed = 0;
        srand(time(nullptr)); // Инициализация случайного числа
        init_food(); // Инициализация еды
        initscr(); // Инициализация ncurses
        keypad(stdscr, TRUE); // Включение обработки специальных клавиш
        raw(); // Отключение буферизации ввода
        noecho(); // Отключение отображения вводимых символов
        curs_set(FALSE); // Отключение курсора
        mvprintw(0, 0, "Use arrows to control. Press 'q' to exit."); // Инструкция для игрока
        refresh();
        timeout(10); // Установка таймера
    }

    // Основной цикл игры для змейки
    void snake_run(Snake& snake) {
        while (run_cont) {
            if (snake.isCrash() || snake.hitBarrier(barriers)) {
                run_cont = false;
                break;
            }

            snake.move(M); // Перемещение змейки
            snake.moveTail(M); // Перемещение хвоста

            if (haveEat(snake)) {
                snake.addTail(); // Если змейка съела еду, добавляем новый сегмент хвоста
            }

            refreshFood(); // Обновление еды
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Задержка
        }
    }

    // Проверка, съела ли змейка еду
    bool haveEat(Snake& snake) {
        for (auto& f : food) {
            if (f.enable && snake.x == f.x && snake.y == f.y) {
                M.lock();
                f.enable = false; // Делаем еду неактивной
                M.unlock();
                return true; // Змейка съела еду
            }
        }
        return false;
    }

    // Основной цикл игры
    void run() {
        putFood(); // Устанавливаем еду
        placeBarriers(); // Устанавливаем барьеры
        printLevel(); // Печатаем уровень

        // Создаём потоки для игры
        std::thread player_thread(&Game::snake_run, this, std::ref(player));
        std::thread computer_thread(&Game::snake_run, this, std::ref(computer));
        std::thread randomizer(&Game::randomizeDirection, this, std::ref(computer));

        // Основной цикл игры
        while (run_cont) {
            key_pressed = getch(); // Получаем нажатую клавишу

            if (key_pressed == STOP_GAME) {
                run_cont = false; // Если нажата клавиша выхода, завершаем игру
            } else {
                changeDirection(key_pressed); // Изменяем направление змейки
            }

            printLevel(); // Обновляем отображение уровня
        }

        player_thread.join(); // Ожидаем завершения потоков
        computer_thread.join();
        randomizer.join();
        end_game(); // Завершаем игру
    }
};

int main() {
    Game game; // Создание объекта игры
    game.run(); // Запуск игры
    return 0;
}
