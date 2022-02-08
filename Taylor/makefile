
#./clobber BWBWBWBWBWBWBWBWBWBW W 100

SHELL := /bin/bash

CC = g++
FLAGS = -Wall --std=c++17 -O3

run: TheSolvers
	./TheSolvers BWBWBWWBWBWBWB B 200

TheSolvers: main.o utils.o state.o solver.o
	$(CC) $(FLAGS) $^ -o $@

%.o: %.cpp %.h
	$(CC) $(FLAGS) $< -o $@ -c

clean:
	-rm *.o TheSolvers

test: TheSolvers
	python3 TheSolvers.py
