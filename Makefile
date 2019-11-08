CC=g++ -g3 -Wall -Wextra -std=c++17 -pthread

lab: lab.cpp
	$(CC) -o $@ $^
