SHELL := /bin/bash

CC = g++
FLAGS = -Wall --std=c++17 -O3

clobber: clobber.o clobbercache.o game.o sumgame.o
	$(CC) $(FLAGS) $^ -o $@

%.o: %.cpp %.h
	$(CC) $(FLAGS) $< -o $@ -c

%.o: %.cpp
	$(CC) $(FLAGS) $< -o $@ -c

clean:
	-rm *.o clobber

