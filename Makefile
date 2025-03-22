all: build run

build:
	g++ -std=c++17 bpe.cpp -O3
run:
	./a.out
