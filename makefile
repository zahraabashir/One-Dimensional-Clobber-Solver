SHELL := /bin/bash

CC = g++
FLAGS = -Wall --std=c++17 -O3

run: clobber
	time ./clobber BWBWBWBWBWBWBWBWBW W  100

clobber: main.o utils.o state.o solver.o
	$(CC) $(FLAGS) $^ -o $@

%.o: %.cpp %.h
	$(CC) $(FLAGS) $< -o $@ -c

clean:
	-rm *.o clobber

test: clobber
	python3 test.py
