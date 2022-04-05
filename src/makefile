
#./clobber BWBWBWBWBWBWBWBWBWBW W 100

SHELL := /bin/bash

CC = g++
FLAGS = -Wall --std=c++17 -O3

.PHONY: dbtest

run: TheSolvers
	./TheSolvers BWBWBWBWBWBWBWB B 500

TheSolvers: main.o database.o utils.o state.o solver.o 
	$(CC) $(FLAGS) $^ -o $@

DBManage: dbmanage.o database.o utils.o state.o validSolver.o
	$(CC) $(FLAGS) $^ -o $@

%.o: %.cpp %.h
	$(CC) $(FLAGS) $< -o $@ -c

%.o: %.cpp
	$(CC) $(FLAGS) $< -o $@ -c

clean:
	-rm *.o TheSolvers DBManage dbtest

cleanall: clean
	-make clean -C instructorSolution/sample_c++/code

rmdb:
	-rm database.bin

#@echo "This is a W position"
#./TheSolvers BWW.BW.WB.WBWWWWWBWWBBWBBBB W 1
#./TheSolvers W.W.BW.WB.WBWWWWWBWWBBWBBBB B 1
corr: TheSolvers referenceSolver
	python3 instructorSolution/runTests.py

fcorr: TheSolvers referenceSolver
	python3 instructorSolution/runTests.py --fast

report: TheSolvers referenceSolver
	python3 instructorSolution/runTests.py -nc > correctnessReport.txt

freport: TheSolvers referenceSolver
	python3 instructorSolution/runTests.py -nc --fast > correctnessReport.txt


test: TheSolvers
	python3 TheSolvers.py

ptest: TheSolvers
	python3 test.py

db: DBManage
	touch database.bin
	./DBManage

dbtest: database.o dbtest.o utils.o state.o
	$(CC) $(FLAGS) $^ -o $@
	./dbtest

resetReferenceCache:
	echo -e "{\n}" > instructorSolution/cachedOutcomes.json

referenceSolver:
	make -C instructorSolution/sample_c++/code

hardtest: TheSolvers
	time ./TheSolvers BWBWBWBWBWBWBWBWBWBWB.WBWBWBWBWBWBWBWBWB W 500
