#include <iostream>
#include "utils.h"
#include "state.h"
#include "solver.h"

 #include <chrono>

using namespace std;


int main(int argc, char **argv) {
    if (argc < 3) {
        cout << "Usage:\n" << argv[0] << " <board> <toPlay> <time>" << endl;
        return 0;
    }
    string board(argv[1]);

    int rootPlayer = charToPlayerNumber(*argv[2]);
    double timeLimit = (double) atoi(argv[3]);

    BasicSolver solver(rootPlayer, board.length());
    solver.timeLimit = timeLimit - 0.1 - (((double) board.length()) * 0.01);
    solver.startTime = std::chrono::steady_clock::now();

    State *root = new State(board, rootPlayer);




    
    //int result = solver.solveRoot(root, rootPlayer, opponentNumber(rootPlayer));
    int result = solver.solveID(root, rootPlayer, opponentNumber(rootPlayer));


    if (best_from==-1) {
        cout << playerNumberToChar(result) << " None" << " " << node_count;
    } else {
        cout << playerNumberToChar(result) << " " << best_from << "-" << best_to << "  " << node_count;
    }


    return 0;
}
