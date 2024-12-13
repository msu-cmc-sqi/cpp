#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <ncurses.h>
#include <thread>
#include <atomic>
#include <chrono>

enum {LEFT=3, UP = 0, RIGHT = 1, DOWN = 2, STOP_GAME='q'};
enum {MAX_TAIL_SIZE=1000, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10, SPEED=20000, SEED_NUMBER=3};

struct Tail {
    int x, y;
};

struct Food {
    int x, y;
    time_t put_time;
    char point; // каким значком отображаем еду
    bool enable;
};

class Snake {
public:
    int x, y, direction;
    size_t tsize;
    std::vector<Tail> tail;

    Snake(int init_x = 0, int init_y = 2, int init_dir = RIGHT)
        : x(init_x), y(init_y), direction(init_dir), tsize(START_TAIL_SIZE+1) { 
        tail.resize(MAX_TAIL_SIZE);
    }

    void moveHead() {
        int max_x = 0, max_y = 0;
        char head = '@';
        getmaxyx(stdscr, max_y, max_x);
        mvprintw(y, x, " ");

        switch (direction) {
            case LEFT:
                if (x <= 0) x = max_x - 1;
                else --x;
                break;
            case RIGHT:
                if (x >= max_x - 1) x = 0;
                else ++x;
                break;
            case UP:
                if (y <= 0) y = max_y - 1;
                else --y;
                break;
            case DOWN:
                if (y >= max_y - 1) y = 0;
                else ++y;
                break;
        }
        mvprintw(y, x, "%c", head);
        refresh();
    }

    void moveTail() {
        char tailchunk = '*';
        mvprintw(tail[tsize - 1].y, tail[tsize - 1].x, " ");
        for (size_t i = tsize - 1; i > 0; i--) {
            tail[i] = tail[i - 1];
            mvprintw(tail[i].y, tail[i].x, "%c", tailchunk);
        }
        tail[0].x = x;
        tail[0].y = y;
    }

    void addTail() {
        if (tsize < MAX_TAIL_SIZE) {
            tsize++;
        } else {
            mvprintw(0, 0, "Can't add tail");
        }
    }

    bool isCrash() {
        for (size_t i = 1; i < tsize; ++i) {
            if (x == tail[i].x && y == tail[i].y) return true;
        }
        return false;
    }

    void printLevel() {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        mvprintw(0, max_x - 10, "LEVEL: %zu", tsize);
    }
};

// Класс для работы с едой
class FoodMove {
public:
    Food food[MAX_FOOD_SIZE];

    void setFood() {
        char point = '$';
        for (size_t i = 0; i < MAX_FOOD_SIZE; i++) {
            if (!food[i].enable) {
                int max_x = 0, max_y = 0;
                getmaxyx(stdscr, max_y, max_x);
                food[i].x       = rand() % max_x;
                food[i].y       = rand() % max_y;
                food[i].enable  = true;
                food[i].point   = point;
                food[i].put_time = time(nullptr);
                mvprintw(food[i].y, food[i].x, "%c", food[i].point);
                break;
            }
        }
    }

    void clearFood(size_t i) {
        food[i].enable = false;
        mvprintw(food[i].y, food[i].x, " ");
    }

    void moveFood(Snake &snake) {
        for (size_t i = 0; i < MAX_FOOD_SIZE; i++) {
            if (food[i].enable &&
                time(nullptr) - food[i].put_time >= FOOD_EXPIRE_SECONDS) { 
                clearFood(i);
            }

            if (snake.x == food[i].x && snake.y == food[i].y && food[i].enable) {
                snake.addTail();
                clearFood(i);
            }
        }
    }
};

// Вторая змея, движущаяся по заданной траектории
void secondSnakeMove(Snake &secondSnake, std::atomic<bool> &gameRunning, std::vector<int> &trajectory) {
    size_t idx = 0;
    while (gameRunning) {
        secondSnake.direction = trajectory[idx];
        secondSnake.moveHead();

        secondSnake.moveTail();
        idx = (idx + 1) % trajectory.size();
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Задержка движения
    }
}

void changeDirection(int &new_direction, int key) {
    switch (key) {
        case KEY_DOWN: new_direction = DOWN; break;
        case KEY_UP: new_direction = UP; break;
        case KEY_LEFT: new_direction = LEFT; break;
        case KEY_RIGHT: new_direction = RIGHT; break;
    }
}

int main() {
    initscr();//инициализация экрана
    keypad(stdscr, TRUE);
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(FALSE);

    srand(time(nullptr));

    Snake playerSnake;
    FoodMove food;
    // Вторая змея
    Snake secondSnake(10, 10, DOWN); // зададим траекторию второй змейки
    std::vector<int> tr0 = {DOWN, RIGHT, UP, LEFT, DOWN, RIGHT};
    std::vector<int> trajectory = {DOWN, RIGHT, UP, LEFT};
    for (int iter = 0; iter < 100; iter++){
           int mviter = rand() % 5 + 7;
	   int tmp = *(trajectory.end() - 1);
	   int key = (tmp + 2) % 4;
	   int ter = 0;
	   while (tmp == ((key + 2) % 4)){
		   ter++;
		   key = tr0[(mviter * ter) % 6];
	   }
	   trajectory.push_back(key);
    }

    std::atomic<bool> gameRunning(true); // Флаг работы игры
    std::thread secondSnakeThread(secondSnakeMove, std::ref(secondSnake), std::ref(gameRunning), std::ref(trajectory)); // запускаем вторую змею в отдельной нити
    mvprintw(0, 0, " Use arrows for control. Press 'q' for EXIT");
    int input;
    while (gameRunning) { // базовый цикл
        input = getch();
        if (input == STOP_GAME) break;
	if (playerSnake.isCrash()) break;

        if (rand() % SEED_NUMBER == 0) food.setFood();

        changeDirection(playerSnake.direction, input);
        food.moveFood(playerSnake);
        playerSnake.moveHead(); // двигаем змейку
        playerSnake.moveTail();

        // Проверяем столкновение с хвостом второй змейки
        for (size_t i = 0; i < secondSnake.tsize; ++i) {
            if (playerSnake.x == secondSnake.tail[i].x && playerSnake.y == secondSnake.tail[i].y) {
                gameRunning = false;
                break;
            }
        }

        // Проверяем на столкновение двух голов
        if (playerSnake.x == secondSnake.x && playerSnake.y == secondSnake.y) {
            gameRunning = false;
            break;
        }

        playerSnake.printLevel(); // обновляем уровень
        std::this_thread::sleep_for(std::chrono::milliseconds(150)); // Скорость игрока
    }
    secondSnakeThread.join(); // ждём завершения работы второй нити

    mvprintw(4, 4, "Your LEVEL is %zu", playerSnake.tsize);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    gameRunning = false;
    endwin();
    return 0;
}

