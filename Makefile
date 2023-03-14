CC = g++ main.cpp -std=c++17 -pthread -o main
NUM_THREADS=1 2 4 8 16 32

main: main.cpp
	${CC}

clean:
	rm main

part_b:
	./main 1

part_c:
	$(foreach t,$(NUM_THREADS),./main $(t);)