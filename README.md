# One-Dimensional-Clobber-Solver

## Intro

A 1 dimensional Clobber solver mostly following the specification of assignments 1 and 2.

The "sample_c++" and "tests" directories in the src/instructorSolution directory are derived from the "ualberta-mueller-group/clobber_1xn" repository shared with us -- we did not write that code. We added a makefile and rearranged the test cases in these directories, and added a few scripts to the src/instructorSolution directory for testing.

Our source code is in the "src" directory, and consists of all the .h and .cpp files. A makefile in this directory can build the solver and do various other things.

## Usage

In the src directory, 

make TheSolvers

will build the solver, while

make run

will build the solver and demonstrate usage.


./TheSolvers

after building will also demonstrate usage.

make fcorr

will run correctness tests based on the testlist.txt in the instructorSolution directory. This test doesn't check that the returned move is correct, though

make corr

will run the tests and check the moves against the reference solver provided to us. The reference solver is much slower, and so is this test, so results from it are cached by the testing scripts in a .json file.

make cleanall

will delete the solver executable and object files, and those for the reference solver in the instructorSolution/sample_c++ directory.

make db

Will build the database again. When building the database, you will need to edit options.h as follows:
Comment out SOLVER_SUBSTITUTE, and enable SOLVER_FIX_MEMORY_LEAK.



## Known Bugs
When the first player has no move, "0-0" is sometimes printed instead of "None", but this is a minor formatting issue, and not a correctness issue.

The BasicSolver class in Solver.h and Solver.cpp has a memory leak as it doesn't clean up the transposition table properly on exit. This is easy to fix, and the BasicSolver destructor has code to do this, but it's left commented out, to speed up subsequent invocations of the solver. This is because substituting simpler games in place of subgames can increase the size of the board, so the transposition table allows variable size boards, which is achieved by storing a pointer to a board buffer in each entry whose size can vary per entry. This doesn't affect correctness, and memory is only leaked on deletion of the solver, so this doesn't affect long searches.

