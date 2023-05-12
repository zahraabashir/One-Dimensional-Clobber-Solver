# One-Dimensional-Clobber-Solver

## Intro

A 1 dimensional Clobber solver mostly following the specification of assignments 1 and 2.

g++ is used to build the solver; clang might not work, which can be a problem for Mac OS.

The "sample_c++" and "tests" directories in the src/instructorSolution directory are derived from the "ualberta-mueller-group/clobber_1xn" repository shared with us -- we did not write this code. We added a makefile and rearranged the test cases in these directories, and added a few scripts to the src/instructorSolution directory for testing.

Our source code is in the "src" directory, and consists of all the .h and .cpp files. A makefile in this directory can build the solver and do various other things.

## Usage

In the src directory, 

`make TheSolvers`

will build the solver, while

`make run`

will build the solver and demonstrate usage.


`./TheSolvers`

after building will also demonstrate usage.

`make fcorr`

will run correctness tests based on the testlist.txt in the instructorSolution directory. This test only checks that the outcome is correct, and doesn't check that the returned move is correct, though

`make corr`

will run the tests and check the moves against the reference solver provided to us. The reference solver is much slower, and so is this test, so results from the reference solver are cached by the testing scripts in a .json file.

`make cleanall`

will delete the solver executable and object files, and those for the reference solver in the instructorSolution/sample_c++ directory.

`make db`

Will build the database again.



## Known Bugs/Limitations
The program leaks memory upon exiting, as it doesn't free boards pointed to by transposition table entries. This makes repeated runs faster



## Test files
The final tests files are located in: `src/instructorSolution/tests/finalTests`.

The address of all the files to be tested is listed on the `testlist.txt`. By commenting each row, you can exclude it from being executed. (use # for commenting) 

To verify and run the tests, as mentioned above, you can either use `make fcorr` or `make corr`.
