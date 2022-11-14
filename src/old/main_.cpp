#include <iostream>
#include "utils.h"
#include "state.h"
#include "solver.h"
#include <chrono>
#include "options.h"
#include <cstring>

#include "game.h"



using namespace std;


int main(int argc, char **argv) {


    auto start = std::chrono::steady_clock::now();

    if (argc < 4) {
        cout << "Usage:\n" << argv[0] << " <board> <toPlay> <time>" << endl;
        return 0;
    }

    Database db;
    db.load();


    //Initialize solver and state
    string board(argv[1]);

    int rootPlayer = charToPlayerNumber(*argv[2]);
    double timeLimit = (double) atoi(argv[3]);

    #if defined(SOLVER_FIX_MEMORY_LEAK)
    BasicSolver solver(rootPlayer, FIXED_BOARD_SIZE, &db);
    #else
    BasicSolver solver(rootPlayer, board.length(), &db);
    #endif

    solver.timeLimit = timeLimit - 0.05 - ((double) board.length()) * 0.002;
    solver.startTime = start;

    #if defined(SOLVER_FIX_MEMORY_LEAK)
    char boardFixed[FIXED_BOARD_SIZE + 1];

    memset(boardFixed, '.', FIXED_BOARD_SIZE);
    boardFixed[FIXED_BOARD_SIZE] = 0;

    memcpy(boardFixed, board.data(), board.size());

    State *root = new State(boardFixed, rootPlayer);
    #else
    State *root = new State(board, rootPlayer);
    #endif

    // int result = solver.solveRoot(root, rootPlayer, opponentNumber(rootPlayer));
    int result = solver.solveID(root, rootPlayer, opponentNumber(rootPlayer));

    //Print output
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - solver.startTime).count();

    if (solver.outOfTime) {
        cout << "?" << " None " << elapsed << " " << node_count;
    } else if (best_from == -1) {
        cout << playerNumberToChar(result) << " None" << " " << elapsed << " " << node_count;
    } else {
        cout << playerNumberToChar(result) << " " << best_from << "-" << best_to << " " << elapsed << " " << node_count;
    }

    cout << endl;

    delete root;
    return 0;
}
