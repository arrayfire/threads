
all:	async_queue.hpp test.cpp
	clang++ -std=c++11 test.cpp -o test
