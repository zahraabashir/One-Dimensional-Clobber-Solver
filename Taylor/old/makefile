run: debugBuild
	./clobber BWBWBWBWBW B 100

build: debugBuild


debugBuild:
	g++ clobber.cpp -o clobber -Wall --std=c++17 -g

optimizedBuild:
	g++ clobber.cpp -o clobber -Wall --std=c++17 -O3

test: optimizedBuild
	python3 test.py

clean:
	-rm clobber

gdb: debugBuild
	gdb ./clobber -ex "set args BWBWBWBWBW B 100"
