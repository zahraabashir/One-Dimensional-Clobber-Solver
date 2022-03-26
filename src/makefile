
#./clobber BWBWBWBWBWBWBWBWBWBW W 100

SHELL := /bin/bash

CC = g++
FLAGS = -Wall --std=c++17 -O3

run: TheSolvers
	./TheSolvers BWBWBWWBWBWBWB W 200

TheSolvers: main.o database.o utils.o state.o solver.o
	$(CC) $(FLAGS) $^ -o $@

DBManage: dbmanage.o database.o utils.o state.o solver.o
	$(CC) $(FLAGS) $^ -o $@

%.o: %.cpp %.h
	$(CC) $(FLAGS) $< -o $@ -c

clean:
	-rm *.o TheSolvers DBManage

rmdb:
	-rm database.bin

corr: TheSolvers
	@echo "This is a W position"
	./TheSolvers BWW.BW.WB.WBWWWWWBWWBBWBBBB W 1
	./TheSolvers W.W.BW.WB.WBWWWWWBWWBBWBBBB B 1

test: TheSolvers
	python3 TheSolvers.py

ptest: TheSolvers
	python3 test.py

db: DBManage
	touch database.bin
	./DBManage


hardtest: TheSolvers
	time ./TheSolvers BWBWBWBWBWBWBWBWBWBWB.WBWBWBWBWBWBWBWBWB W 500
