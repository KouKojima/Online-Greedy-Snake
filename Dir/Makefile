main:  Server EatingSnake

Server: game_server.cpp
	g++ game_server.cpp -o server -std=c++11

EatingSnake: eating_snake.cpp
	g++ eating_snake.cpp -o eat -std=c++11 -lpthread

.PHONY: clean
clean:
	rm Server EatingSnake
