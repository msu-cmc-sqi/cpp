# Makefile
LDFLAGS=-lncurses -std=c++17 -Wall -Wextra -Werror

snake: main.cpp
	g++ -o snake main.cpp $(LDFLAGS)

run: snake
	./snake	

clean:
	rm snake

all: snake