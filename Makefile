all: build run

build:
	g++ -std=c++11 bpe.cpp 
run:
	./a.out
