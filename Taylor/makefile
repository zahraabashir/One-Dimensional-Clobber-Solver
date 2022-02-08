
#./clobber BWBWBWBWBWBWBWBWBWBW W 100

SHELL := /bin/bash

CC = g++
FLAGS = -Wall --std=c++17 -O3

run: clobber
	time ./clobber BWBWBWBWBWBWBWBWBW W 1

clobber: main.o utils.o state.o solver.o
	$(CC) $(FLAGS) $^ -o $@

%.o: %.cpp %.h
	$(CC) $(FLAGS) $< -o $@ -c

clean:
	-rm *.o clobber

test: clobber
	python3 test.py
	python3 final_test.py
