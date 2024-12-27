#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <ncurses.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

// Направления движения змейки
enum Direction { LEFT = 1, UP, RIGHT, DOWN };

// Константы игры
const int MAX_TAIL_SIZE = 1000; // Максимальный размер хвоста змеи
const int START_TAIL_SIZE = 3;  // Начальный размер хвоста змеи
const int MAX_FOOD_COUNT = 30;  // Максимальное количество еды на карте
const int FOOD_EXPIRE_SECONDS = 10; // Срок действия еды (в секундах)
const int GAME_SPEED = 100;    // Скорость игры (в миллисекундах на шаг)
const char QUIT_KEY = 'q';     // Клавиша выхода
const int WALL_COUNT = 10;      // Количество стен

// Позиция на карте
struct Position {
    int x, y;
};

// Структура еды
struct Food {
    Position pos;  // Позиция еды
    time_t timestamp;  // Время размещения еды
    bool active;  // Активна ли еда
    char symbol = '$';  // Символ еды ('$' или '%')
};

// Класс змейки
class Snake {
private:
    std::vector<Position> tail; // Хвост змеи
    Direction direction; // Направление движения
    size_t size; // Текущий размер хвоста
    Position head; // Позиция головы змеи

public:
    // Конструктор змеи
    Snake(Position startPos, Direction startDir) : direction(startDir), size(START_TAIL_SIZE), head(startPos) {
        tail.resize(MAX_TAIL_SIZE);
    }

    // Получение текущей позиции головы
    Position getHeadPosition() const { return head; }

    // Получение текущего размера хвоста
    size_t getSize() const { return size; }

    // Получение хвоста змеи
    const std::vector<Position>& getTail() const { return tail; }

    // Изменение направления движения
    void changeDirection(Direction newDirection) {
        if ((direction == LEFT && newDirection != RIGHT) ||
            (direction == RIGHT && newDirection != LEFT) ||
            (direction == UP && newDirection != DOWN) ||
            (direction == DOWN && newDirection != UP)) {
            direction = newDirection;
        }
    }

    // Перемещение змеи на карте
    void move(int maxX, int maxY) {
        tailUpdate(); // Обновление хвоста
        switch (direction) {
            case LEFT:
                head.x = (head.x > 0) ? head.x - 1 : maxX - 1;
                break;
            case RIGHT:
                head.x = (head.x + 1) % maxX;
                break;
            case UP:
                head.y = (head.y > 0) ? head.y - 1 : maxY - 1;
                break;
            case DOWN:
                head.y = (head.y + 1) % maxY;
                break;
        }
    }

    // Увеличение хвоста змеи
    void grow() {
        if (size < MAX_TAIL_SIZE) {
            size++;
        }
    }

    // Проверка столкновения головы с хвостом
    bool isCollision() const {
        for (size_t i = 1; i < size; ++i) {
            if (head.x == tail[i].x && head.y == tail[i].y) {
                return true;
            }
        }
        return false;
    }

    // Отображение змеи на экране
    void render() const {
        mvprintw(head.y, head.x, "@"); // Голова змеи
        for (size_t i = 0; i < size; ++i) {
            mvprintw(tail[i].y, tail[i].x, "*"); // Хвост змеи
        }
    }

private:
    // Обновление позиции хвоста змеи
    void tailUpdate() {
        for (size_t i = size; i > 0; --i) {
            tail[i] = tail[i - 1];
        }
        tail[0] = head;
    }
};

// Класс управления стенами
class WallManager {
private:
    std::vector<Position> walls; // Позиции стен

public:
    // Конструктор
    WallManager() {
        walls.resize(WALL_COUNT);
    }

    // Инициализация стен
    void initialize(int maxX, int maxY) {
        for (auto& w : walls) {
            w.x = rand() % maxX;
            w.y = rand() % maxY;
        }
    }

    // Проверка столкновения змеи со стенами
    bool checkCollision(Position snakeHead) const {
        for (const auto& w : walls) {
            if (w.x == snakeHead.x && w.y == snakeHead.y) {
                return true;
            }
        }
        return false;
    }

    // Отображение стен
    void render() const {
        for (const auto& w : walls) {
            mvprintw(w.y, w.x, "!");
        }
    }
};

// Класс управления едой
class FoodManager {
private:
    std::vector<Food> food; // Список еды

public:
    // Конструктор
    FoodManager() {
        food.resize(MAX_FOOD_COUNT);
    }

    // Инициализация еды
    void initialize(int maxX, int maxY) {
        for (auto& f : food) {
            placeFood(f, maxX, maxY);
        }
    }

    // Обновление еды на карте
    void refresh(int maxX, int maxY) {
        for (auto& f : food) {
            if (!f.active || (time(nullptr) - f.timestamp) > FOOD_EXPIRE_SECONDS) {
                placeFood(f, maxX, maxY);
            }
        }
    }

    // Проверка столкновения змеи с едой
    bool checkCollision(Position snakeHead, char& foodSymbol) {
        for (auto& f : food) {
            if (f.active && f.pos.x == snakeHead.x && f.pos.y == snakeHead.y) {
                foodSymbol = f.symbol;
                f.active = false;
                return true;
            }
        }
        return false;
    }

    // Отображение еды на карте
    void render() const {
        for (const auto& f : food) {
            if (f.active) {
                mvprintw(f.pos.y, f.pos.x, "%c", f.symbol);
            }
        }
    }

private:
    // Размещение еды на карте
    void placeFood(Food& f, int maxX, int maxY) {
        f.pos.x = rand() % maxX;
        f.pos.y = rand() % maxY;
        f.timestamp = time(nullptr);
        // 20% вероятность появления еды '%', иначе '$'
        f.symbol = (rand() % 5 == 0) ? '%' : '$';
        f.active = true;
    }
};

// Основной класс игры
class Game {
private:
    Snake snake1; // Первая змея
    Snake snake2; // Вторая змея
    FoodManager foodManager; // Управление едой
    WallManager wallManager; // Управление стенами
    std::mutex renderMutex; // Мьютекс для синхронизации рендеринга
    std::atomic<bool> running; // Флаг состояния игры
    int maxX, maxY; // Размеры экрана
    std::atomic<int> snake1Speed; // Скорость первой змеи
    std::atomic<int> snake2Speed; // Скорость второй змеи

public:
    // Конструктор игры
    Game() : snake1({0, 2}, RIGHT), snake2({10, 10}, LEFT), running(true), snake1Speed(GAME_SPEED), snake2Speed(GAME_SPEED) {}

    // Запуск игры
    void run() {
        initialize();

        std::thread snake2Thread(&Game::runSnake2, this);

        int key = 0;
        while (running) {
            key = getch();
            if (key == QUIT_KEY) {
                running = false;
                break;
            }
            handleInput(key);

            {
                std::lock_guard<std::mutex> lock(renderMutex);

                snake1.move(maxX, maxY);

                if (checkCollisions()) {
                    running = false;
                    break;
                }

                char symbol = '\0';
                if (foodManager.checkCollision(snake1.getHeadPosition(), symbol)) {
                    if (symbol == '$') {
                        snake1.grow();
                    } else if (symbol == '%') {
                        snake1.grow();
                        speedUpSnake(snake1Speed);
                    }
                }

                foodManager.refresh(maxX, maxY);

                render();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(snake1Speed));
        }

        snake2Thread.join();
        gameOver();
    }

private:
    // Инициализация игры
    void initialize() {
        srand(time(nullptr));

        initscr();
        keypad(stdscr, TRUE);
        noecho();
        curs_set(FALSE);

        getmaxyx(stdscr, maxY, maxX);

        foodManager.initialize(maxX, maxY);
        wallManager.initialize(maxX, maxY);

        mvprintw(0, 0, "Player 1: Arrows. Player 2: WASD. Press 'q' to quit.");
        timeout(0);
    }

    // Обработка ввода
    void handleInput(int key) {
        switch (key) {
            case KEY_UP:
                snake1.changeDirection(UP);
                break;
            case KEY_DOWN:
                snake1.changeDirection(DOWN);
                break;
            case KEY_LEFT:
                snake1.changeDirection(LEFT);
                break;
            case KEY_RIGHT:
                snake1.changeDirection(RIGHT);
                break;
            case 'w':
                snake2.changeDirection(UP);
                break;
            case 's':
                snake2.changeDirection(DOWN);
                break;
            case 'a':
                snake2.changeDirection(LEFT);
                break;
            case 'd':
                snake2.changeDirection(RIGHT);
                break;
        }
    }

    // Проверка столкновений
    bool checkCollisions() {
        Position head1 = snake1.getHeadPosition();
        Position head2 = snake2.getHeadPosition();

        if (wallManager.checkCollision(head1) || wallManager.checkCollision(head2)) {
            return true;
        }

        if (snake1.isCollision() || snake2.isCollision()) {
            return true;
        }

        for (size_t i = 0; i < snake2.getSize(); ++i) {
            if (head1.x == snake2.getTail()[i].x && head1.y == snake2.getTail()[i].y) {
                return true;
            }
        }

        for (size_t i = 0; i < snake1.getSize(); ++i) {
            if (head2.x == snake1.getTail()[i].x && head2.y == snake1.getTail()[i].y) {
                return true;
            }
        }

        return head1.x == head2.x && head1.y == head2.y;
    }

    // Работа второй змеи в отдельном потоке
    void runSnake2() {
        while (running) {
            {
                std::lock_guard<std::mutex> lock(renderMutex);
                snake2.move(maxX, maxY);

                char symbol = '\0';
                if (foodManager.checkCollision(snake2.getHeadPosition(), symbol)) {
                    if (symbol == '$') {
                        snake2.grow();
                    } else if (symbol == '%') {
                        snake2.grow();
                        speedUpSnake(snake2Speed);
                    }
                }

                if (checkCollisions()) {
                    running = false;
                    break;
                }

                render();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(snake2Speed));
        }
    }

    // Отображение игры
    void render() {
        clear();
        mvprintw(0, 0, "Player 1 LEVEL: %zu   Player 2 LEVEL: %zu",
                 snake1.getSize() - START_TAIL_SIZE,
                 snake2.getSize() - START_TAIL_SIZE);
        snake1.render();
        snake2.render();
        foodManager.render();
        wallManager.render();
        refresh();
    }

    // Окончание игры
    void gameOver() {
        clear();
        mvprintw(maxY / 2, maxX / 2 - 10, "GAME OVER!");
        mvprintw(maxY / 2 + 1, maxX / 2 - 10, "Player 1 LEVEL: %zu", snake1.getSize() - START_TAIL_SIZE);
        mvprintw(maxY / 2 + 2, maxX / 2 - 10, "Player 2 LEVEL: %zu", snake2.getSize() - START_TAIL_SIZE);
        refresh();
        timeout(-1);
        getch();
        endwin();
    }

    // Ускорение змеи
    void speedUpSnake(std::atomic<int>& speedVar) {
        speedVar = GAME_SPEED / 3;
        std::thread([&speedVar]() {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            speedVar = GAME_SPEED;
        }).detach();
    }
};


int main() {
    Game game;
    game.run();
    return 0;
}
