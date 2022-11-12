#include <iostream>
#include "utils.h"
#include "state.h"
#include "solver.h"
#include "options.h"
#include <cstring>
#include <chrono.h>

#include "game.h"

using namespace std;


int main(int argc, char **argv) {
    if (argc < 3) {
        cout << "Usage:\n" << argv[0] << " <board> <toPlay> <time (ignored)>" << endl;
        return 0;
    }

    Database db;
    db.load();

    size_t boardLen = strlen(argv[1]);
    uint8_t board[boardLen];

    for (size_t i = 0; i < boardLen; i++) {
        board[i] = charToPlayerNumber(argv[1][i]);
    }

    int rootPlayer = charToPlayerNumber(*argv[2]);

    Solver solver(boardLen, &db);

    auto startTime = std::chrono::steady_clock::now();
    int result = solver.solveID(board, boardLen, rootPlayer);

    //Print output
    auto endTime = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(endTime - startTime).count();

    if (best_from == -1) {
        cout << playerNumberToChar(result) << " None" << " " << elapsed << " " << node_count;
    } else {
        cout << playerNumberToChar(result) << " " << best_from << "-" << best_to << " " << elapsed << " " << node_count;
    }

    cout << endl;

    return 0;
}
