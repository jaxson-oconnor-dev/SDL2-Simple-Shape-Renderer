all:
	g++ main.cpp -o main -Isrc/include -Lsrc/lib -lmingw32 -lSDL2main -lSDL2 -std=c++11