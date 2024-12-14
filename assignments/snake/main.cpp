#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <ncurses.h>
#include <unistd.h>
#include <mutex>
#include <atomic>
#include <thread>

enum { LEFT = 1, UP, RIGHT, DOWN, STOP_GAME = 'q' };
enum { MAX_TAIL_SIZE = 1000, START_TAIL_SIZE = 3, MAX_FOOD_SIZE = 20, FOOD_EXPIRE_SECONDS = 10, SPEED = 20000, SEED_NUMBER = 3 };

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
    char head, body;
    std::vector<Tail> tail;

    Snake(int xx, int yy, char c1, char c2, int d) : x(xx), y(yy), direction(d),
                                                     head(c1), body(c2), tsize(START_TAIL_SIZE + 1) {
        tail.resize(MAX_TAIL_SIZE);
    }

    void move(std::mutex& M) {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);

        M.lock();
        mvprintw(y, x, " ");

        switch (direction) {
            case LEFT: x = (x <= 0) ? max_x : x - 1; break;
            case RIGHT: x = (x >= max_x) ? 0 : x + 1; break;
            case UP: y = (y <= 0) ? max_y : y - 1; break;
            case DOWN: y = (y >= max_y) ? 0 : y + 1; break;
        }
        mvprintw(y, x, "%c", head);
        refresh();
        M.unlock();
    }

    void moveTail(std::mutex& M) {
        M.lock();
        mvprintw(tail[tsize - 1].y, tail[tsize - 1].x, " ");
        for (size_t i = tsize - 1; i > 0; i--) {
            tail[i] = tail[i - 1];
            if (tail[i].y || tail[i].x) {
                mvprintw(tail[i].y, tail[i].x, "%c", body);
            }
        }
        M.unlock();

        tail[0].x = x;
        tail[0].y = y;
    }

    void addTail() {
        if (tsize < MAX_TAIL_SIZE) {
            tsize++;
        }
    }

    bool isCrash() {
        for (size_t i = 1; i < tsize; ++i) {
            if (x == tail[i].x && y == tail[i].y) return true;
        }
        return false;
    }

    bool hitBarrier(const std::vector<std::pair<int, int>>& barriers) {
        for (const auto& b : barriers) {
            if (x == b.first && y == b.second) return true;
        }
        return false;
    }
};

class Game {
    std::vector<Food> food;
    std::vector<std::pair<int, int>> barriers;
    Snake computer, player;
    std::mutex M;
    std::atomic<bool> run_cont = true;
    int key_pressed;

    void init_food() {
        food.resize(MAX_FOOD_SIZE);
        int max_y = 0, max_x = 0;
        getmaxyx(stdscr, max_y, max_x);
        for (auto& f : food) {
            f = {0, 0, 0, '$', false};
        }
    }

    void putFoodSeed(Food& fp) {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        M.lock();
        mvprintw(fp.y, fp.x, " ");
        fp.x = rand() % (max_x - 1);
        fp.y = rand() % (max_y - 3) + 2;
        fp.put_time = time(nullptr);
        fp.enable = true;
        mvprintw(fp.y, fp.x, "%c", fp.point);
        M.unlock();
    }

    void putFood() {
        for (auto& f : food) {
            putFoodSeed(f);
        }
        refresh();
    }

    void refreshFood() {
        for (auto& f : food) {
            if (f.put_time && (!f.enable || (time(nullptr) - f.put_time) > FOOD_EXPIRE_SECONDS)) {
                putFoodSeed(f);
            }
        }
    }

    void placeBarriers() {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);

        barriers.clear();
        for (int i = 0; i < 10; ++i) {
            int bx = rand() % (max_x - 1);
            int by = rand() % (max_y - 3) + 2;
            barriers.emplace_back(bx, by);
        }

        for (const auto& b : barriers) {
            mvprintw(b.second, b.first, "#");
        }
        refresh();
    }

    void randomizeDirection(Snake& snake) {
        while (run_cont) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            int new_direction;
            do {
                new_direction = rand() % 4 + 1;
            } while ((snake.direction == LEFT && new_direction == RIGHT) ||
                     (snake.direction == RIGHT && new_direction == LEFT) ||
                     (snake.direction == UP && new_direction == DOWN) ||
                     (snake.direction == DOWN && new_direction == UP));

            M.lock();
            snake.direction = new_direction;
            M.unlock();
        }
    }

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

    void printLevel() {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        M.lock();
        mvprintw(0, max_x - 17, "COMPUTER LEVEL: %zu", computer.tsize);
        mvprintw(1, max_x - 15, "PLAYER LEVEL: %zu", player.tsize);
        refresh();
        M.unlock();
    }

    void end_game() {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        mvprintw(max_y / 2, max_x / 2 - 10, "Game Over!");
        mvprintw(max_y / 2 + 2, max_x / 2 - 10, "Press any key to exit...");
        timeout(SPEED);
        getch();
        endwin();
    }

public:
    Game() : computer(0, 3, '@', '*', RIGHT), player(10, 15, '*', '+', LEFT) {
        key_pressed = 0;
        srand(time(nullptr));
        init_food();
        initscr();
        keypad(stdscr, TRUE);
        raw();
        noecho();
        curs_set(FALSE);
        mvprintw(0, 0, "Use arrows to control. Press 'q' to exit.");
        refresh();
        timeout(10);
    }

    void snake_run(Snake& snake) {
        while (run_cont) {
            if (snake.isCrash() || snake.hitBarrier(barriers)) {
                run_cont = false;
                break;
            }

            snake.move(M);
            snake.moveTail(M);

            if (snake.hitBarrier(barriers)) {
                run_cont = false;
                break;
            }

            if (haveEat(snake)) {
                snake.addTail();
            }

            refreshFood();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    bool haveEat(Snake& snake) {
        for (auto& f : food) {
            if (f.enable && snake.x == f.x && snake.y == f.y) {
                M.lock();
                f.enable = false;
                M.unlock();
                return true;
            }
        }
        return false;
    }

    void run() {
        putFood();
        placeBarriers();
        printLevel();

        std::thread player_thread(&Game::snake_run, this, std::ref(player));
        std::thread computer_thread(&Game::snake_run, this, std::ref(computer));
        std::thread randomizer(&Game::randomizeDirection, this, std::ref(computer));

        while (run_cont) {
            key_pressed = getch();

            if (key_pressed == STOP_GAME) {
                run_cont = false;
            } else {
                changeDirection(key_pressed);
            }

            printLevel();
        }

        player_thread.join();
        computer_thread.join();
        randomizer.join();
        end_game();
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}

