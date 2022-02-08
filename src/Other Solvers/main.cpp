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

    string board(argv[1]);

    int rootPlayer = charToPlayerNumber(*argv[2]);
    BasicSolver solver(rootPlayer, board.length());
    State *root = new State(board, rootPlayer);

    //uncomment for minimax
    // int result = solver.solveOr(root, rootPlayer, opponentNumber(rootPlayer));

    //negamax without tt
    int result = solver.solveRoot(root, rootPlayer, opponentNumber(rootPlayer));



    cout <<" \n"<<playerNumberToChar(result)<<" "<<node_count;
    

    return 0;
}
