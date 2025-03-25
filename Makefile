all: build run

build:
	clang++ -std=c++17 bpe.cpp -O3 -fopenmp
run:
	./a.out
