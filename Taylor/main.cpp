#include <iostream>
#include "utils.h"
#include "state.h"
#include "solver.h"

using namespace std;


int main(int argc, char **argv) {
    if (argc < 4) {
        cout << "Usage:\n" << argv[0] << " <board> <toPlay> <time> [debug]" << endl;
        return 0;
    }

    int rootPlayer = charToPlayerNumber(*argv[2]);
    BasicSolver solver(rootPlayer);
    State *root = new State(argv[1], rootPlayer);

/*
    for (size_t i = 0; i < root->moveCount; i++) {
        cout << root->moves[2 * i] << " " << root->moves[2 * i + 1] << endl;
    }

    return 0;
*/

    int result = solver.solveOr(root);
    cout << playerNumberToChar(result) << endl;

    return 0;
}
