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
    char head, body;
    std::vector<Tail> tail;

    Snake(int xx, int yy, char c1, char c2, int d) : x(xx), y(yy), direction(d), 
	head(c1), body(c2), tsize(START_TAIL_SIZE+1) {
        tail.resize(MAX_TAIL_SIZE);
    }

    void move(std::mutex& M) {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
	
	M.lock();
        
	mvprintw(y, x, " ");

        switch (direction) {
            case LEFT:
                if (x <= 0) x = max_x;
		x--;
                break;
            case RIGHT:
                if (x >= max_x) x = 0;
                x++;
		break;
            case UP:
                if (y <= 0) y = max_y;
                y--;
		break;
            case DOWN:
                if (y >= max_y) y = 0;
                y++;
                break;
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

class Game {
    std::vector<Food> food;
    Snake snake1, snake2;
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
        //putFood();
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

    void repairSeed(Snake& snake) {
    	for (size_t i = 0; i < snake.tsize; i++) {
            for (auto& f : food) {
            	if (f.x == snake.tail[i].x && f.y == snake.tail[i].y && f.enable) {
                    putFoodSeed(f);
            	}
            }
    	}
    }

    bool snakes_collided(Snake& snake) {
	if (snake.head == '@') {
            for (size_t i = 1; i < snake2.tsize; ++i) {
            	if (snake.x == snake2.tail[i].x && snake.y == snake2.tail[i].y) return true;
            }
            return false;
	}
	else {
            for (size_t i = 1; i < snake1.tsize; ++i) {
            	if (snake.x == snake1.tail[i].x && snake.y == snake1.tail[i].y) return true;
            }
            return false;

	}
    }
   
    void changeDirection(int key) {
	M.lock();
	switch (key) {
	    case KEY_DOWN: if (snake2.direction != UP) 
			       snake2.direction = DOWN; 
			   break;
	    case 's': if (snake1.direction != UP)
		          snake1.direction = DOWN;
		      break;
	    case KEY_UP: if (snake2.direction != DOWN)
			     snake2.direction = UP; 
			 break;
	    case 'w': if (snake1.direction != DOWN)
		          snake1.direction = UP; 
		      break;
            case KEY_LEFT: if (snake2.direction != RIGHT)
			       snake2.direction = LEFT; 
			   break;
            case 'a': if (snake1.direction != RIGHT)
		          snake1.direction = LEFT; 
		      break;
            case KEY_RIGHT: if (snake2.direction != LEFT)
			        snake2.direction = RIGHT; 
			    break;
            case 'd': if (snake1.direction != LEFT)
		          snake1.direction = RIGHT; 
		      break;
	}
	M.unlock();
    }
   
    void printLevel() {
        int max_x = 0, max_y = 0;
	getmaxyx(stdscr, max_y, max_x);
	
	M.lock();

	mvprintw(0, max_x - 10, "LEVEL1: %zu", snake1.tsize);
	mvprintw(1, max_x - 10, "LEVEL2: %zu", snake2.tsize);
	refresh();
	M.unlock();
    }
   
    void printExit() {
    	int max_x = 0, max_y = 0;
    	getmaxyx(stdscr, max_y, max_x);
    	mvprintw(max_y / 2, max_x / 2 - 5, "Snake1 LEVEL is %zu", snake1.tsize);
    	mvprintw(max_y / 2 + 2, max_x / 2 - 5, "Snake2 LEVEL is %zu", snake2.tsize);
    }

    void end_game() {
	printExit();
	timeout(SPEED);
	getch();
	endwin();
    }

public:

    Game() : snake1(0, 3, '@', '*', RIGHT), snake2(0, 25, '&', '+', LEFT) {
	key_pressed = 0;
	srand(time(nullptr));
	init_food();
	
	initscr();
	keypad(stdscr, TRUE);
	raw();
	noecho();
	curs_set(FALSE);

	mvprintw(0, 0, " Use arrows for control. Press 'q' for EXIT");
	refresh();
	timeout(10);
    }

    void snake_run(Snake& snake) {
	while (run_cont) {
            if (snake.isCrash()) {
		run_cont = false;
		break;
	    }
	    if (snakes_collided(snake)) {
		run_cont = false;
		break;
	    }

            snake.move(M);
            snake.moveTail(M);

            if (haveEat(snake)) {
            	snake.addTail();
            }

	    repairSeed(snake);

	    std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
    }

    void run() {
	putFood();
	printLevel();

	std::thread s1(&Game::snake_run, this, std::ref(snake1));
	std::thread s2(&Game::snake_run, this, std::ref(snake2));
        
	while (key_pressed != STOP_GAME && run_cont) {
            key_pressed = getch();
            changeDirection(key_pressed);

	    refreshFood();
            printLevel();
            
	    timeout(100);
        }
	run_cont = false;
	s1.join();
	s2.join();
	end_game();
    }
};

int main() {
    Game game_in_snake;
    game_in_snake.run();

    return 0;
}
