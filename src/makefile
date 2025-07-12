
#./clobber BWBWBWBWBWBWBWBWBWBW W

SHELL := /bin/bash

CC = g++
#FLAGS = -Wall --std=c++17 -O3 -DNDEBUG
#FLAGS = -Wall --std=c++17 -O3 -g -fno-omit-frame-pointer -fsanitize=address -D_GLIBCXX_DEBUG
FLAGS = -Wall --std=c++17 -O3 -g


.PHONY: dbtest
.PHONY: default


default: run

compTest: solver.o



run: TheSolvers
	@#./TheSolvers BWBWBWBWBWBWBWBWBW B
	@#@echo Try to beat 47399 nodes 
	./TheSolvers BWBWBWBWBWBWBWBWBWBWBWBWBWBWBWBWBWBWBWBWBWBW B
	@#./TheSolvers BWBWBWBWBWBW B

miscTest: database3.o test.o utils.o
	$(CC) $(FLAGS) $^ -o Test
	-time ./Test

TheSolvers: main.o database3.o utils.o state.o solver.o solverExtra.o bound.o zobrist.o MurmurHash3.cpp miscTypes.o
	$(CC) $(FLAGS) $^ -o $@

DBManage3: dbmanage3.o database3.o utils.o state.o solver.o solverExtra.o bound.o zobrist.o MurmurHash3.cpp miscTypes.o
	$(CC) $(FLAGS) $^ -o $@

DBManage4: dbmanage4.o database3.o utils.o state.o solver.o solverExtra.o bound.o zobrist.o MurmurHash3.cpp miscTypes.o
	$(CC) $(FLAGS) $^ -o $@

explore: dbExplore.cpp database3.o utils.o state.o bound.o zobrist.o MurmurHash3.cpp miscTypes.o
	$(CC) $(FLAGS) $^ -o $@




%.o: %.cpp %.h options.h
	$(CC) $(FLAGS) $< -o $@ -c

%.o: %.cpp options.h
	$(CC) $(FLAGS) $< -o $@ -c

clean:
	-rm *.o TheSolvers DBManage dbtest DBManage2 DBManage3 DBManage4 Test explore

cleanall: clean
	-make clean -C instructorSolution/sample_c++/code


rmdb:
	-rm database3.bin

corr: TheSolvers referenceSolver
	python3 instructorSolution/runTests.py

fcorr: TheSolvers
	python3 instructorSolution/runTests.py --fast

pcorr: TheSolvers
	python3 instructorSolution/runTestsPersistent.py --fast

report: TheSolvers referenceSolver
	python3 instructorSolution/runTests.py -nc > correctnessReport.txt

freport: TheSolvers referenceSolver
	python3 instructorSolution/runTests.py -nc --fast > correctnessReport.txt

db: DBManage4
	-rm database4.bin
	touch database3.bin
	-time ./DBManage4


resetReferenceCache:
	echo -e "{\n}" > instructorSolution/cachedOutcomes.json

referenceSolver:
	make -C instructorSolution/sample_c++/code

hardtest: TheSolvers
	-time ./TheSolvers BWBWBWBWBWBWBWBWBWBWB.WBWBWBWBWBWBWBWBWB W
