
#./clobber BWBWBWBWBWBWBWBWBWBW W 100

SHELL := /bin/bash

CC = g++
FLAGS = -Wall --std=c++17 -g


.PHONY: dbtest
.PHONY: default


default: run

compTest: solver.o


run: TheSolvers
	#-./TheSolvers BWBWBWBWBWBW W 200
	-./TheSolvers BWBWBWB.WBWBWBW W 1

miscTest: database3.o test.o utils.o
	$(CC) $(FLAGS) $^ -o Test
	-time ./Test

TheSolvers: main.o database3.o utils.o state.o solver.o bound.o
	$(CC) $(FLAGS) $^ -o $@

DBManage: dbmanage.o database.o utils.o state.o solver.o bound.o
	$(CC) $(FLAGS) $^ -o $@

DBManage2: dbmanage2.o database2.o utils.o state.o solver.o bound.o
	$(CC) $(FLAGS) $^ -o $@

%.o: %.cpp %.h options.h
	$(CC) $(FLAGS) $< -o $@ -c

%.o: %.cpp options.h
	$(CC) $(FLAGS) $< -o $@ -c

clean:
	-rm *.o TheSolvers DBManage dbtest DBManage2 Test

cleanall: clean
	-make clean -C instructorSolution/sample_c++/code

rmdb:
	-rm database.bin

rm2:
	-rm database2.bin

#@echo "This is a W position"
#./TheSolvers BWW.BW.WB.WBWWWWWBWWBBWBBBB W 1
#./TheSolvers W.W.BW.WB.WBWWWWWBWWBBWBBBB B 1
corr: TheSolvers referenceSolver
	python3 instructorSolution/runTests.py

fcorr: TheSolvers
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
	-./DBManage

db2: DBManage2
	touch database2.bin
	-./DBManage2

dbtest: database2.o dbtest.o utils.o state.o
	$(CC) $(FLAGS) $^ -o $@
	-./dbtest

resetReferenceCache:
	echo -e "{\n}" > instructorSolution/cachedOutcomes.json

referenceSolver:
	make -C instructorSolution/sample_c++/code

hardtest: TheSolvers
	-time ./TheSolvers BWBWBWBWBWBWBWBWBWBWB.WBWBWBWBWBWBWBWBWB W 500
