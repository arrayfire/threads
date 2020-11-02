
mkfile_dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
include_dir := $(mkfile_dir)include/threads
test_dir := $(mkfile_dir)test

all: $(include_dir)/async_queue.hpp $(include_dir)/parallel.hpp $(test_dir)/test.cpp
	g++-4.9 -O3 -g -Wall -std=c++11 -I$(include_dir) $(test_dir)/test.cpp -o test
