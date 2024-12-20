/*
 * Для компиляции необходимо добавить ключ -lncurses
 * g++ -o snake main.cpp -lncurses
 */

#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <ncurses.h>

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
    std::vector<Tail> tail;

    Snake() : x(0), y(2), direction(RIGHT), tsize(START_TAIL_SIZE+1) {
        tail.resize(MAX_TAIL_SIZE);
    }

    void move() {
        int max_x = 0, max_y = 0;
        char ch = '@';
        getmaxyx(stdscr, max_y, max_x);
        mvprintw(y, x, " ");

        switch (direction) {
            case LEFT:
                if (x <= 0) x = max_x;
                mvprintw(y, --x, "%c", ch);
                break;
            case RIGHT:
                if (x >= max_x) x = 0;
                mvprintw(y, ++x, "%c", ch);
                break;
            case UP:
                if (y <= 0) y = max_y;
                mvprintw(--y, x, "%c", ch);
                break;
            case DOWN:
                if (y >= max_y) y = 0;
                mvprintw(++y, x, "%c", ch);
                break;
        }
        refresh();
    }

    void moveTail() {
        char ch = '*';
        mvprintw(tail[tsize - 1].y, tail[tsize - 1].x, " ");
        for (size_t i = tsize - 1; i > 0; i--) {
            tail[i] = tail[i - 1];
            if (tail[i].y || tail[i].x) {
                mvprintw(tail[i].y, tail[i].x, "%c", ch);
            }
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
};

void changeDirection(int& new_direction, int key) {
    switch (key) {
        case KEY_DOWN: new_direction = DOWN; break;
        case KEY_UP: new_direction = UP; break;
        case KEY_LEFT: new_direction = LEFT; break;
        case KEY_RIGHT: new_direction = RIGHT; break;
    }
}

void initFood(std::vector<Food>& food) {
    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);
    for (auto& f : food) {
        f = {0, 0, 0, '$', false};
    }
}

void putFoodSeed(Food& fp) {
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(fp.y, fp.x, " ");
    fp.x = rand() % (max_x - 1);
    fp.y = rand() % (max_y - 2) + 1;
    fp.put_time = time(nullptr);
    fp.enable = true;
    mvprintw(fp.y, fp.x, "%c", fp.point);
}

void putFood(std::vector<Food>& food) {
    for (auto& f : food) {
        putFoodSeed(f);
    }
}

void refreshFood(std::vector<Food>& food) {
    for (auto& f : food) {
        if (f.put_time && (!f.enable || (time(nullptr) - f.put_time) > FOOD_EXPIRE_SECONDS)) {
            putFoodSeed(f);
        }
    }
}

bool haveEat(Snake& snake, std::vector<Food>& food) {
    for (auto& f : food) {
        if (f.enable && snake.x == f.x && snake.y == f.y) {
            f.enable = false;
            return true;
        }
    }
    return false;
}

void repairSeed(std::vector<Food>& food, Snake& snake) {
    for (size_t i = 0; i < snake.tsize; i++) {
        for (auto& f : food) {
            if (f.x == snake.tail[i].x && f.y == snake.tail[i].y && f.enable) {
                putFoodSeed(f);
            }
        }
    }
}

void printLevel(Snake& snake) {
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(0, max_x - 10, "LEVEL: %zu", snake.tsize);
}

void printExit(Snake& snake) {
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(max_y / 2, max_x / 2 - 5, "Your LEVEL is %zu", snake.tsize);
}

int main() {
    srand(time(nullptr));

    Snake snake;
    std::vector<Food> food(MAX_FOOD_SIZE);
    initFood(food);

    initscr();
    keypad(stdscr, TRUE);
    raw();
    noecho();
    curs_set(FALSE);
    
    mvprintw(0, 0, "  Use arrows for control. Press 'q' for EXIT");
    putFood(food);
    timeout(0);

    int key_pressed = 0;
    while (key_pressed != STOP_GAME) {
        key_pressed = getch();
        changeDirection(snake.direction, key_pressed);
        
        if (snake.isCrash()) break;

        snake.move();
        snake.moveTail();

        if (haveEat(snake, food)) {
            snake.addTail();
            printLevel(snake);
        }

        refreshFood(food);
        repairSeed(food, snake);

        timeout(100);
    }

    printExit(snake);
    timeout(SPEED);
    getch();
    endwin();

    return 0;
}
