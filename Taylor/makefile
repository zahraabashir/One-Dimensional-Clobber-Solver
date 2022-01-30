CC = g++
FLAGS = -Wall --std=c++17 -g

run: clobber
	./clobber BWBWBW B 100

clobber: main.o utils.o state.o state.o
	$(CC) $(FLAGS) $^ -o $@

%.o: %.cpp %.h
	$(CC) $(FLAGS) $< -o $@ -c

clean:
	-rm *.o clobber
