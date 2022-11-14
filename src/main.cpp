#include <iostream>
#include "utils.h"
//#include "state.h"
#include "solver.h"
#include "options.h"
#include <cstring>
#include <chrono>

//#include "game.h"

using namespace std;


bool test() {
    return false;

    Database db;
    db.load();

    uint8_t board[] = {1,1,2};
    size_t len = sizeof(board);
    
    uint8_t *entry = db.get(board, len);
    assert(entry);

    cout << "Outcome is: ";
    cout << (int) *db_get_outcome(entry) << endl;


    cout << "Entry is:" << endl;
    cout << (uint64_t) *db_get_outcome(entry) << endl;
    cout << (uint64_t) db_get_dominance(entry)[0] << endl;
    cout << (uint64_t) db_get_dominance(entry)[1] << endl;
    cout << (uint64_t) db_get_bounds(entry)[0] << endl;
    cout << (uint64_t) db_get_bounds(entry)[1] << endl;
    cout << (uint64_t) *db_get_metric(entry) << endl;
    cout << (uint64_t) *db_get_link(entry) << endl;
    cout << (uint64_t) *db_get_shape(entry) << endl;
    cout << (uint64_t) *db_get_number(entry) << endl;
        

    cout << endl;

    uint8_t board1[] = {1,1,1,1};
    uint8_t board2[] = {2,1,1,1};
    uint8_t *ptr1 = db.get(board1, 4);
    uint8_t *ptr2 = db.get(board2, 4);

    assert(ptr1);
    assert(ptr2);
    cout << "Diff: " << (uint64_t) (ptr2 - ptr1) << endl;



    return true;
}


int main(int argc, char **argv) {
    if (test()) {
        return 0;
    }

    if (argc < 3) {
        cout << "Usage:\n" << argv[0] << " <board> <toPlay> <time (ignored)>" << endl;
        return 0;
    }

    Database db;
    db.load();
    //db.init();

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
