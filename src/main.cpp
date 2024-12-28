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

void printUsage(const char *execName) {
    cout << "Usage:\n" << execName << " [--persist] <board> <toPlay> <time (ignored)>" << endl;
}

void persistMain() {
    Database db;
    db.load();


    string boardStr;
    string toPlayStr;

    //int rootPlayer = charToPlayerNumber(*argv[2]);

    Solver solver(70, &db); // boardLen is only used to pick TT hash length

    while (cin) {
        cin >> boardStr;

        if (cin.fail()) {
            if (!cin.eof()) {
                cerr << "Some input error happened reading board" << endl;
                exit(-1);
            }
            return;
        }

        cin >> toPlayStr;

        if (cin.fail()) {
            cerr << "Some input error happened reading player" << endl;
            exit(-1);
        }

        // Validate input
        for (int i = 0; i < boardStr.size(); i++) {
            const char &c = boardStr[i];
            if (c != '.' && c != 'B' && c != 'W') {
                cerr << "Invalid board character. Use 'B', 'W', and '.'" << endl;
                exit(-1);
            }
        }

        if (toPlayStr.size() != 1 || (toPlayStr[0] != 'B' && toPlayStr[0] != 'W')) {
            cerr << "Bad player character. Use 'B' or 'W'" << endl;
            exit(-1);
        }

        size_t boardLen = strlen(boardStr.c_str());
        uint8_t board[boardLen];

        memcpy(board, boardStr.c_str(), boardLen);

        // sanity checks
        if (boardLen != boardStr.size()) { // string size() excludes null
            cerr << "Wrong board size" << endl;
            exit(-1);
        }

        for (size_t i = 0; i < boardLen; i++) {
            if (board[i] != boardStr[i]) {
                cerr << "Bad board copy" << endl;
                exit(-1);
            }
        }

        // Convert board format
        for (size_t i = 0; i < boardLen; i++) {
            board[i] = charToPlayerNumber(board[i]);
        }

        int rootPlayer = charToPlayerNumber(toPlayStr[0]);

        // Reset these
        node_count = 0;
        best_from = -1;
        best_to = -1;

        int result = solver.solveID(board, boardLen, rootPlayer);

        if (best_from == -1) {
            cout << playerNumberToChar(result) << " None" << " " << node_count;
        } else {
            cout << playerNumberToChar(result) << " " << best_from << "-" << best_to << " " << node_count;
        }

        cout << endl;
    }
}


int main(int argc, char **argv) {
    if (test()) {
        return 0;
    }

    if (argc < 2) {
        printUsage(argv[0]);
        return 0;
    }

    bool persist = strcmp(argv[1], "--persist") == 0;
    if (persist) {
        persistMain();
        return 0;
    }

    if (argc < 3) {
        printUsage(argv[0]);
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
