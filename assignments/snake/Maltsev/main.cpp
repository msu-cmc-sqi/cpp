//
//  main.cpp
//  snake
//
//  Created by Егор Мальцев on 29.10.2024.
//

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
#include <mutex>
#include <atomic>

enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME='q'};
enum {MAX_TAIL_SIZE=1000, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10, SPEED=20000, SEED_NUMBER=3};

struct Tail {
    int x, y;
};

struct Food {
    int x, y;
    time_t put_time;
    char point;
    bool enable;
};

class Snake {
public:
    int x, y, direction;
    size_t tsize;
    char headChar, tailChar;
    std::vector<Tail> tail;

    Snake(int startX, int startY, int startDirection, char headCh, char tailCh)
        : x(startX), y(startY), direction(startDirection), tsize(START_TAIL_SIZE+1), headChar(headCh), tailChar(tailCh) {
        tail.resize(MAX_TAIL_SIZE);
    }

    void move(std::mutex& mtx) {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);

        std::lock_guard<std::mutex> lock(mtx);
        mvprintw(y, x, " ");

        switch (direction) {
            case LEFT:  x = (x > 0) ? x - 1 : max_x; break;
            case RIGHT: x = (x < max_x) ? x + 1 : 0; break;
            case UP:    y = (y > 0) ? y - 1 : max_y; break;
            case DOWN:  y = (y < max_y) ? y + 1 : 0; break;
        }
        mvprintw(y, x, "%c", headChar);
        refresh();
    }

    void moveTail(std::mutex& mtx) {
        std::lock_guard<std::mutex> lock(mtx);
        mvprintw(tail[tsize - 1].y, tail[tsize - 1].x, " ");
        for (size_t i = tsize - 1; i > 0; i--) {
            tail[i] = tail[i - 1];
            mvprintw(tail[i].y, tail[i].x, "%c", tailChar);
        }
        tail[0].x = x;
        tail[0].y = y;
    }

    void addTail() {
        if (tsize < MAX_TAIL_SIZE) tsize++;
    }

    bool isCrash() {
        for (size_t i = 1; i < tsize; ++i) {
            if (x == tail[i].x && y == tail[i].y) return true;
        }
        return false;
    }
};

class Game {
public:
    Snake snake1, snake2;
    std::vector<Food> food;
    std::mutex mtx;
    std::atomic<bool> running;

    Game() : snake1(0, 2, RIGHT, '@', '*'), snake2(5, 5, LEFT, '#', '+'), food(MAX_FOOD_SIZE), running(true) {
        srand(time(nullptr));
        initFood();
    }

    void initFood() {
        for (auto& f : food) {
            f = {0, 0, 0, '$', false};
        }
    }

    void putFoodSeed(Food& fp) {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);

        std::lock_guard<std::mutex> lock(mtx);
        mvprintw(fp.y, fp.x, " ");
        fp.x = rand() % (max_x - 1);
        fp.y = rand() % (max_y - 2) + 1;
        fp.put_time = time(nullptr);
        fp.enable = true;
        mvprintw(fp.y, fp.x, "%c", fp.point);
    }

    void putFood() {
        for (auto& f : food) {
            putFoodSeed(f);
        }
    }

    bool haveEat(Snake& snake) {
        for (auto& f : food) {
            if (f.enable && snake.x == f.x && snake.y == f.y) {
                f.enable = false;
                putFoodSeed(f);
                return true;
            }
        }
        return false;
    }

    void play(Snake& snake) {
        while (running) {
            snake.move(mtx);
            snake.moveTail(mtx);

            if (haveEat(snake)) snake.addTail();
            if (snake.isCrash()) running = false;

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void start() {
        initscr();
        keypad(stdscr, TRUE);
        raw();
        noecho();
        curs_set(FALSE);
        putFood();

        std::thread t1(&Game::play, this, std::ref(snake1));
        std::thread t2(&Game::play, this, std::ref(snake2));

        int key_pressed = 0;
        while (running && key_pressed != STOP_GAME) {
            key_pressed = getch();

            std::lock_guard<std::mutex> lock(mtx);
            changeDirection(snake1.direction, key_pressed);
            
            if (key_pressed == 'w') snake2.direction = UP;
            else if (key_pressed == 'a') snake2.direction = LEFT;
            else if (key_pressed == 's') snake2.direction = DOWN;
            else if (key_pressed == 'd') snake2.direction = RIGHT;
        }

        running = false;
        t1.join();
        t2.join();
        endwin();
    }

    void changeDirection(int& direction, int key) {
        switch (key) {
            case KEY_DOWN: direction = DOWN; break;
            case KEY_UP: direction = UP; break;
            case KEY_LEFT: direction = LEFT; break;
            case KEY_RIGHT: direction = RIGHT; break;
        }
    }
};

int main() {
    Game game;
    game.start();
    return 0;
}
