#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <ncurses.h>
#include <chrono>
#include <thread>

enum Direction
{
    LEFT = 1,
    UP,
    RIGHT,
    DOWN
};
const int MAX_TAIL_SIZE = 1000;
const int START_TAIL_SIZE = 3;
const int MAX_FOOD_SIZE = 20;
const int FOOD_EXPIRE_SECONDS = 10;

enum GameResult
{
    NONE,
    SNAKE1_WON,
    SNAKE2_WON,
    DRAW
};

struct Tail
{
    int x, y;
};

struct Food
{
    int x, y;
    time_t put_time;
    char symbol;
    bool enabled;

    Food() : x(0), y(0), put_time(0), symbol('$'), enabled(false) {}

    void placeRandomly(int maxX, int maxY, const std::vector<Tail> &snake1_tail, const std::vector<Tail> &snake2_tail)
    {
        bool conflict;
        do
        {
            conflict = false;
            x = rand() % (maxX - 2) + 1;
            y = rand() % (maxY - 2) + 1;
            for (const auto &t : snake1_tail)
            {
                if (x == t.x && y == t.y)
                {
                    conflict = true;
                    break;
                }
            }
            if (!conflict)
            {
                for (const auto &t : snake2_tail)
                {
                    if (x == t.x && y == t.y)
                    {
                        conflict = true;
                        break;
                    }
                }
            }
        } while (conflict);
        put_time = time(nullptr);
        enabled = true;
    }

    bool isEatenBy(int snake_x, int snake_y)
    {
        if (enabled && x == snake_x && y == snake_y)
        {
            enabled = false;
            return true;
        }
        return false;
    }

    void render() const
    {
        if (enabled)
        {
            mvprintw(y, x, "%c", symbol);
        }
    }

    bool isExpired() const
    {
        return enabled && (time(nullptr) - put_time) > FOOD_EXPIRE_SECONDS;
    }
};

class Snake
{
public:
    Snake(int startX, int startY, Direction dir, char headCh, char tailCh)
        : x(startX), y(startY), direction(dir), tsize(START_TAIL_SIZE), headChar(headCh), tailChar(tailCh)
    {
        tail.resize(MAX_TAIL_SIZE);
        int dx = 0, dy = 0;
        switch (direction)
        {
        case LEFT:
            dx = 1;
            dy = 0;
            break;
        case RIGHT:
            dx = -1;
            dy = 0;
            break;
        case UP:
            dx = 0;
            dy = 1;
            break;
        case DOWN:
            dx = 0;
            dy = -1;
            break;
        }
        for (size_t i = 0; i < tsize; ++i)
        {
            tail[i].x = x + dx * i;
            tail[i].y = y + dy * i;
        }
    }

    void move()
    {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        mvprintw(y, x, " ");

        switch (direction)
        {
        case LEFT:
            x = (x <= 1) ? max_x - 2 : x - 1;
            break;
        case RIGHT:
            x = (x >= max_x - 2) ? 1 : x + 1;
            break;
        case UP:
            y = (y <= 1) ? max_y - 2 : y - 1;
            break;
        case DOWN:
            y = (y >= max_y - 2) ? 1 : y + 1;
            break;
        }
    }

    void moveTail()
    {
        mvprintw(tail[tsize - 1].y, tail[tsize - 1].x, " ");
        for (size_t i = tsize - 1; i > 0; --i)
        {
            tail[i] = tail[i - 1];
        }
        tail[0].x = x;
        tail[0].y = y;
    }

    void changeDirection(Direction newDir)
    {
        if ((direction == LEFT && newDir != RIGHT) ||
            (direction == RIGHT && newDir != LEFT) ||
            (direction == UP && newDir != DOWN) ||
            (direction == DOWN && newDir != UP))
        {
            direction = newDir;
        }
    }

    void grow()
    {
        if (tsize < MAX_TAIL_SIZE)
        {
            tsize++;
        }
    }

    void render() const
    {
        mvprintw(y, x, "%c", headChar);
        for (size_t i = 1; i < tsize; ++i)
        {
            mvprintw(tail[i].y, tail[i].x, "%c", tailChar);
        }
    }

    bool checkSelfCollision() const
    {
        for (size_t i = 1; i < tsize; ++i)
        {
            if (x == tail[i].x && y == tail[i].y)
            {
                return true;
            }
        }
        return false;
    }

    Direction getDirection() const
    {
        return direction;
    }

    int getX() const
    {
        return x;
    }

    int getY() const
    {
        return y;
    }

    size_t getSize() const
    {
        return tsize;
    }

    std::vector<Tail> getTail() const
    {
        return std::vector<Tail>(tail.begin(), tail.begin() + tsize);
    }

private:
    int x, y;
    Direction direction;
    size_t tsize;
    std::vector<Tail> tail;
    char headChar;
    char tailChar;
};

class Game
{
public:
    Game()
        : snake1(nullptr),
          snake2(nullptr),
          gameOver(false),
          result(NONE)
    {
        foods.resize(MAX_FOOD_SIZE);
    }

    ~Game()
    {
        if (snake1)
            delete snake1;
        if (snake2)
            delete snake2;
    }

    void init()
    {
        srand(time(nullptr));

        initscr();
        keypad(stdscr, TRUE);
        raw();
        noecho();
        curs_set(FALSE);
        nodelay(stdscr, TRUE);
        getmaxyx(stdscr, max_y, max_x);

        if (max_x < 40 || max_y < 20)
        {
            endwin();
            std::cerr << "Terminal size too small. Minimum size is 40x20." << std::endl;
            exit(1);
        }

        for (int i = 0; i < max_x; ++i)
        {
            mvprintw(0, i, "#");
            mvprintw(max_y - 1, i, "#");
        }
        for (int i = 0; i < max_y; ++i)
        {
            mvprintw(i, 0, "#");
            mvprintw(i, max_x - 1, "#");
        }

        mvprintw(0, 2, "Use arrows for Snake '@', WASD for Snake '#'. Press 'q' to EXIT");

        snake1 = new Snake(max_x / 4, max_y / 2, RIGHT, '@', '*');
        snake2 = new Snake(3 * max_x / 4, max_y / 2, LEFT, '#', '+');

        for (auto &food : foods)
        {
            placeFood(food);
        }
    }

    void run()
    {
        while (!gameOver)
        {
            auto start_time = std::chrono::steady_clock::now();

            handleInput();
            moveSnakes();
            checkFoodConsumption();
            refreshFood();
            render();
            checkCollisions();

            auto end_time = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            int sleep_time = 100 - elapsed.count();
            if (sleep_time > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
            }
        }
    }

    void endGame()
    {
        gameOver = true;

        clear();
        mvprintw(max_y / 2 - 1, (max_x / 2) - 10, "Game Over!");

        switch (result)
        {
        case SNAKE1_WON:
            mvprintw(max_y / 2, (max_x / 2) - 10, "Snake '@' won!");
            break;
        case SNAKE2_WON:
            mvprintw(max_y / 2, (max_x / 2) - 10, "Snake '#' won!");
            break;
        case DRAW:
            mvprintw(max_y / 2, (max_x / 2) - 10, "It's a draw!");
            break;
        default:
            mvprintw(max_y / 2, (max_x / 2) - 10, "Game ended.");
            break;
        }

        mvprintw(max_y / 2 + 1, (max_x / 2) - 20, "Snake '@' Length: %zu | Snake '#' Length: %zu", snake1->getSize(), snake2->getSize());
        mvprintw(max_y / 2 + 2, (max_x / 2) - 15, "Press any key to exit.");
        nodelay(stdscr, FALSE);
        getch();
        endwin();
    }

private:
    Snake *snake1;
    Snake *snake2;
    std::vector<Food> foods;
    bool gameOver;
    GameResult result;
    int max_y, max_x;

    void handleInput()
    {
        int ch = getch();
        switch (ch)
        {
        case KEY_UP:
            snake1->changeDirection(UP);
            break;
        case KEY_DOWN:
            snake1->changeDirection(DOWN);
            break;
        case KEY_LEFT:
            snake1->changeDirection(LEFT);
            break;
        case KEY_RIGHT:
            snake1->changeDirection(RIGHT);
            break;
        case 'w':
        case 'W':
            snake2->changeDirection(UP);
            break;
        case 's':
        case 'S':
            snake2->changeDirection(DOWN);
            break;
        case 'a':
        case 'A':
            snake2->changeDirection(LEFT);
            break;
        case 'd':
        case 'D':
            snake2->changeDirection(RIGHT);
            break;
        case 'q':
        case 'Q':
            if (!gameOver)
            {
                if (snake1->getSize() > snake2->getSize())
                {
                    result = SNAKE1_WON;
                }
                else if (snake2->getSize() > snake1->getSize())
                {
                    result = SNAKE2_WON;
                }
                else
                {
                    result = DRAW;
                }
                gameOver = true;
            }
            break;
        default:
            break;
        }
    }

    void moveSnakes()
    {
        snake1->move();
        snake1->moveTail();
        snake2->move();
        snake2->moveTail();
    }

    void checkFoodConsumption()
    {
        for (auto &food : foods)
        {
            if (food.isEatenBy(snake1->getX(), snake1->getY()))
            {
                snake1->grow();
                placeFood(food);
            }
            if (food.isEatenBy(snake2->getX(), snake2->getY()))
            {
                snake2->grow();
                placeFood(food);
            }
        }
    }

    void placeFood(Food &food)
    {
        food.placeRandomly(max_x, max_y, snake1->getTail(), snake2->getTail());
        food.render();
    }

    void refreshFood()
    {
        for (auto &food : foods)
        {
            if (food.isExpired())
            {
                placeFood(food);
            }
        }
    }

    void render()
    {
        snake1->render();
        snake2->render();
        for (const auto &food : foods)
        {
            food.render();
        }
        mvprintw(1, 2, "Snake '@' Length: %zu", snake1->getSize());
        mvprintw(2, 2, "Snake '#' Length: %zu", snake2->getSize());
        refresh();
    }

    void checkCollisions()
    {
        bool snake1_hits_snake2_body = false;
        bool snake2_hits_snake1_body = false;
        bool heads_collide = false;

        std::vector<Tail> snake1_tail = snake1->getTail();
        std::vector<Tail> snake2_tail = snake2->getTail();

        for (size_t i = 1; i < snake2_tail.size(); ++i)
        {
            if (snake1->getX() == snake2_tail[i].x && snake1->getY() == snake2_tail[i].y)
            {
                snake1_hits_snake2_body = true;
                break;
            }
        }

        for (size_t i = 1; i < snake1_tail.size(); ++i)
        {
            if (snake2->getX() == snake1_tail[i].x && snake2->getY() == snake1_tail[i].y)
            {
                snake2_hits_snake1_body = true;
                break;
            }
        }

        if (snake1->getX() == snake2->getX() && snake1->getY() == snake2->getY())
        {
            heads_collide = true;
        }

        if (snake2_hits_snake1_body && snake1_hits_snake2_body)
        {
            result = DRAW;
            gameOver = true;
            return;
        }
        else if (snake2_hits_snake1_body)
        {
            result = SNAKE1_WON;
            gameOver = true;
            return;
        }
        else if (snake1_hits_snake2_body)
        {
            result = SNAKE2_WON;
            gameOver = true;
            return;
        }
        else if (heads_collide)
        {
            if (snake1->getSize() > snake2->getSize())
            {
                result = SNAKE1_WON;
            }
            else if (snake2->getSize() > snake1->getSize())
            {
                result = SNAKE2_WON;
            }
            else
            {
                result = DRAW;
            }
            gameOver = true;
            return;
        }

        if (snake1->checkSelfCollision())
        {
            result = SNAKE2_WON;
            gameOver = true;
            return;
        }
        if (snake2->checkSelfCollision())
        {
            result = SNAKE1_WON;
            gameOver = true;
            return;
        }

        if (snake1->getX() <= 0 || snake1->getX() >= max_x - 1 || snake1->getY() <= 0 || snake1->getY() >= max_y - 1)
        {
            result = SNAKE2_WON;
            gameOver = true;
            return;
        }
        if (snake2->getX() <= 0 || snake2->getX() >= max_x - 1 || snake2->getY() <= 0 || snake2->getY() >= max_y - 1)
        {
            result = SNAKE1_WON;
            gameOver = true;
            return;
        }
    }
};

int main()
{
    Game game;
    game.init();
    game.run();
    game.endGame();
    return 0;
}
