#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <ncurses.h>
#include <random>
#include <algorithm>
#include <chrono>
#include <thread>

class Snake {
private:
    std::vector<std::pair<int, int>> body;
    int direction;
    std::mutex mtx;
    char bodyChar;
    int score;

public:
    Snake(int startX, int startY, int startDirection, char bodySymbol): direction(startDirection), bodyChar(bodySymbol), score(0) {
        body.push_back({startX, startY});
    }

    void move(int maxX, int maxY) {
        std::lock_guard<std::mutex> lock(mtx);
        auto [headX, headY] = body.front();

        switch (direction) {
            case KEY_UP:    headY--; break;
            case KEY_DOWN:  headY++; break;
            case KEY_LEFT:  headX--; break;
            case KEY_RIGHT: headX++; break;
        }

        headX = (headX + maxX) % maxX;
        headY = (headY + maxY) % maxY;

        body.insert(body.begin(), {headX, headY});
        body.pop_back();
    }

    void setDirection(int newDirection) {
        std::lock_guard<std::mutex> lock(mtx);
        direction = newDirection;
    }

    bool checkSelfCollision() const {
        auto head = body.front();
        return std::any_of(body.begin() + 1, body.end(), [&head](const auto& segment) return segment == head;);
    }

    bool checkCollision(const Snake& other) const {
        auto head = body.front();
        return std::any_of(other.body.begin(), other.body.end(), [&head](const auto& segment) return segment == head;);
    }

    void grow() {
        std::lock_guard<std::mutex> lock(mtx);
        body.push_back(body.back());
    }

    const std::vector<std::pair<int, int>>& getBody() const {
        return body;
    }

    char getBodyChar() const {
        return bodyChar;
    }

    void incrementScore() {
        std::lock_guard<std::mutex> lock(mtx);
        score++;
    }

    int getScore() const {
        return score;
    }
};

class Game {
public:
    Game() : snake1(10, 10, KEY_RIGHT, 'O'), snake2(20, 20, KEY_LEFT, '#'), gameOver(false), rng(std::random_device{}()) {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        timeout(100);
        curs_set(0);
        getmaxyx(stdscr, maxY, maxX);
        placeFood();
    }

    ~Game() {
        endwin();
    }

    void start() {
        while (!gameOver) {
            int ch = getch();
            handleInput(ch);
            updateGame();
            render();
        }
        showFinalScore();
    }

private:
    Snake snake1;
    Snake snake2;
    int maxX, maxY;
    std::pair<int, int> food;
    bool gameOver;
    std::mt19937 rng;
    
    void handleInput(int ch) {
        switch (ch) {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
                snake1.setDirection(ch);
                break;
            case 'w':
                snake2.setDirection(KEY_UP);
                break;
            case 's':
                snake2.setDirection(KEY_DOWN);
                break;
            case 'a':
                snake2.setDirection(KEY_LEFT);
                break;
            case 'd':
                snake2.setDirection(KEY_RIGHT);
                break;
            case 'q':
                gameOver = true;
                break;
        }
    }

    void updateGame() {
        snake1.move(maxX, maxY);
        snake2.move(maxX, maxY);

        if (snake1.checkSelfCollision() || snake2.checkSelfCollision() ||
            snake1.checkCollision(snake2) || snake2.checkCollision(snake1)) {
            gameOver = true;
            return;
        }

        for (auto* snake : {&snake1, &snake2}) {
            if (snake->getBody().front() == food) {
                snake->grow();
                snake->incrementScore();
                placeFood();
            }
        }
    }

    void render() {
        clear();

        for (const auto* snake : {&snake1, &snake2}) {
            for (const auto& [x, y] : snake->getBody()) {
                mvaddch(y, x, snake->getBodyChar());
            }
        }

        mvaddch(food.second, food.first, '*');

        mvprintw(0, 0, "Snake 1: %d | Snake 2: %d", snake1.getScore(), snake2.getScore());

        refresh();
    }

    void placeFood() {
        std::uniform_int_distribution<> distX(0, maxX - 1);
        std::uniform_int_distribution<> distY(0, maxY - 1);
        do {
            food = {distX(rng), distY(rng)};
        } while (snake1.getBody().front() == food || snake2.getBody().front() == food);
    }

    void showFinalScore() {
        clear();
        mvprintw(maxY / 2 - 1, maxX / 2 - 5, "Game Over!");
        mvprintw(maxY / 2, maxX / 2 - 10, "Final Scores:");
        mvprintw(maxY / 2 + 1, maxX / 2 - 10, "Snake 1: %d", snake1.getScore());
        mvprintw(maxY / 2 + 2, maxX / 2 - 10, "Snake 2: %d", snake2.getScore());
        refresh();
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
};

int main() {
    Game game;
    game.start();
    return 0;
}
