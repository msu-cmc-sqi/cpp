#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <ncurses.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

enum Direction {NONE = 0, LEFT = 1, UP, RIGHT, DOWN};

enum {MAX_TAIL_SIZE= 1000,START_TAIL_SIZE = 3,MAX_FOOD_SIZE = 20,FOOD_EXPIRE_SECONDS = 10, SPEED= 20000};

static const char SNAKE_HEAD = '@';
static const char SNAKE_BODY = '*';
static const char FOOD_CHAR  = '$';

struct Tail {
    int x, y;
};

struct Food {
    int x, y;
    time_t put_time;
    bool enable;
    bool was_eaten;
};

class Snake {
public:
    Snake(int start_x, int start_y, Direction dir): x(start_x), y(start_y), direction(dir), tsize(START_TAIL_SIZE + 1)
    {
        tail.resize(MAX_TAIL_SIZE);
        tail[0] = {x, y};
    }

    bool move(int max_x, int max_y) {
        switch (direction) {
            case LEFT:
                x--; break;
            case RIGHT:
                x++; break;
            case UP:
                y--; break;
            case DOWN:
                y++; break;
            default:break;
        }

        if (x < 0 || x > max_x || y < 0 || y > max_y) {
            return false;
        }

        for (size_t i = tsize - 1; i > 0; i--) {
            tail[i] = tail[i - 1];
        }
        tail[0].x = x;
        tail[0].y = y;
        return true;
    }

    void draw() const {
        mvprintw(y, x, "%c", SNAKE_HEAD);
        for (size_t i = 1; i < tsize; i++) {
            mvprintw(tail[i].y, tail[i].x, "%c", SNAKE_BODY);
        }
    }

    void addTail() {
        if (tsize < MAX_TAIL_SIZE) {
            tsize++;
        }
    }

    bool isCrashItself() const {
        return checkCollision(x, y, 1, tsize);
    }

    bool isCrashWithAnother(const Snake& other) const {
        return checkCollision(x, y, 0, other.tsize, other.tail);
    }

    void setDirection(Direction dir) {
        if (!isOppositeDirection(dir)) {
            direction = dir;
        }
    }

    int getX() const { return x; }
    int getY() const { return y; }
    size_t getSize() const { return tsize; }

private:
    int x, y;
    Direction direction;
    size_t tsize;
    std::vector<Tail> tail;

    bool checkCollision(int check_x, int check_y, size_t start, size_t end, const std::vector<Tail>& check_tail = {}) const

    {
        const auto& refTail = check_tail.empty() ? tail : check_tail;
        for (size_t i = start; i < end; ++i) {
            if (check_x == refTail[i].x && check_y == refTail[i].y) {
                return true;
            }
        }
        return false;
    }
    bool isOppositeDirection(Direction dir) const {
        return (direction == LEFT && dir == RIGHT) || (direction == RIGHT && dir == LEFT) ||(direction == UP && dir == DOWN) ||(direction == DOWN && dir == UP);
    }
};

class FoodManager {
public:
    FoodManager() {food.resize(MAX_FOOD_SIZE);}
    void init(WINDOW* win) {
        getmaxyx(win, max_y, max_x);
        max_x--;
        max_y--;
        for (auto& f : food) {putFoodSeed(f);}
    }

    void refreshFood() {
        time_t now = time(nullptr);
        for (auto& f : food) {
            if (!f.enable && f.was_eaten && now - f.put_time > FOOD_EXPIRE_SECONDS) {
                putFoodSeed(f);
            }
        }
    }

    bool haveEat(Snake& snake) {
        for (auto& f : food) {
            if (f.enable && snake.getX() == f.x && snake.getY() == f.y) {
                f.enable = false;
                f.was_eaten = true;
                mvprintw(f.y, f.x, " ");
                snake.addTail();
                return true;
            }
        }
        return false;
    }

    void drawFoods() {
        for (const auto& f : food) {
            if (f.enable) {
                mvprintw(f.y, f.x, "%c", FOOD_CHAR);
            }
        }
    }

private:
    int max_x = 0, max_y = 0;
    std::vector<Food> food;

    void putFoodSeed(Food& f) {
        f.x = rand() % (max_x + 1);
        f.y = rand() % (max_y + 1);
        f.put_time = time(nullptr);
        f.enable = true;
        f.was_eaten = false;
    }
};

class Game {
public:
    Game()
            : snake1(0, 2, RIGHT), snake2(5, 5, LEFT),
              exit_flag(false), score1(0), score2(0)
    {
        srand(time(nullptr));
        initscr();
        keypad(stdscr, TRUE);
        raw();
        noecho();
        curs_set(FALSE);
        nodelay(stdscr, TRUE);

        getmaxyx(stdscr, max_y, max_x);
        max_x--;
        max_y--;

        foodManager.init(stdscr);
    }

    ~Game() {
        endwin();
    }

    void run() {
        std::thread second_snake_thread(&Game::snake2Loop, this);

        while (!exit_flag) {
            int key_pressed = getch();
            if (key_pressed == 'q') {
                exit_flag = true;
            }
            handleInput(key_pressed);

            updateSnake(snake1, score1);
            foodManager.refreshFood();
            checkCollisions();

            redraw();

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (second_snake_thread.joinable()) {
            second_snake_thread.join();
        }

        showWinner();
    }

private:
    Snake snake1, snake2;
    FoodManager foodManager;
    int max_x = 0, max_y = 0;
    std::atomic<bool> exit_flag;
    std::mutex field_mutex;

    int score1;
    int score2;

    void handleInput(int key) {
        Direction d1 = NONE, d2 = NONE;

        switch (key) {
            case KEY_DOWN:
                d1 = DOWN;
                break;
            case KEY_UP:
                d1 = UP;
                break;
            case KEY_LEFT:
                d1 = LEFT;
                break;
            case KEY_RIGHT:
                d1 = RIGHT;
                break;
            case 'w':
                d2 = UP;
                break;
            case 'a':
                d2 = LEFT;
                break;
            case 's':
                d2 = DOWN;
                break;
            case 'd':
                d2 = RIGHT;

                break;
        }


        std::lock_guard<std::mutex> lock(field_mutex);
        if (d1 != NONE) {snake1.setDirection(d1);
        }
        if (d2 != NONE) {
            snake2.setDirection(d2);
        }
    }

    void updateSnake(Snake& snake, int& score) {
        if (!snake.move(max_x, max_y)) {
            exit_flag = true;
        }
        if (foodManager.haveEat(snake)) {
            score++;
        }
    }

    void snake2Loop() {
        while (!exit_flag) {
            updateSnake(snake2, score2);
            checkCollisions();
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    }

    void redraw() {
        std::lock_guard<std::mutex> lock(field_mutex);
        clear();

        snake1.draw();
        snake2.draw();

        foodManager.drawFoods();

        drawCenteredText(0, "Score1: " + std::to_string(score1) + " | Score2: " + std::to_string(score2));

        refresh();
    }

    void checkCollisions() {
        bool crash1_self = snake1.isCrashItself();
        bool crash2_self = snake2.isCrashItself();

        bool crash1_other = snake1.isCrashWithAnother(snake2);
        bool crash2_other = snake2.isCrashWithAnother(snake1);

        if (crash1_self || crash1_other) {
            exit_flag = true;
        }
        if (crash2_self || crash2_other) {
            exit_flag = true;
        }
    }

    void showWinner() {
        bool s1_lost = snake1.isCrashItself() || snake1.isCrashWithAnother(snake2) || !inBounds(snake1.getX(), snake1.getY());
        bool s2_lost = snake2.isCrashItself() || snake2.isCrashWithAnother(snake1) || !inBounds(snake2.getX(), snake2.getY());

        if (s1_lost) {
            score1 = score1 / 2;
        }
        if (s2_lost) {
            score2 = score2 / 2;
        }

        clear();

        std::string result;
        if (score1 > score2) {
            result = "Snake1 WINS!!!!! Score1: " + std::to_string(score1) + ", Score2: " + std::to_string(score2);
        } else if (score2 > score1) {
            result = "Snake2 WINS!!!!! Score1: " + std::to_string(score1) + ", Score2: " + std::to_string(score2);
        } else {
            result = "DRAW! Score1: " + std::to_string(score1) + ", Score2: " + std::to_string(score2);
        }

        drawCenteredText(max_y / 2 - 1, result);
        drawCenteredText(max_y / 2 + 1, "Press 'q' to exit...");

        int key;
        do {
            key = getch();
        } while (key != 'q');
    }

    bool inBounds(int x, int y) {
        return (x >= 0 && x <= max_x && y >= 0 && y <= max_y);
    }

    void drawCenteredText(int row, const std::string& text) {
        int col = (max_x - text.length()) / 2;
        mvprintw(row, col, "%s", text.c_str());
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
