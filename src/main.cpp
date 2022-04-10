#include <iostream>
#include "utils.h"
#include "state.h"
#include "solver.h"
#include <chrono>
#include "options.h"


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

    BasicSolver solver(rootPlayer, board.length(), &db);
    solver.timeLimit = timeLimit - 0.05 - ((double) board.length()) * 0.002;
    solver.startTime = start;

    State *root = new State(board, rootPlayer);

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
