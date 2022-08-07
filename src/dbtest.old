#include <iostream>
#include "database.h"
#include "utils.h"
#include "state.h"

using namespace std;


int main() {
    Database db;
    db.load();

    char boardText[] = "BWBWWWWWWW";
    int len = sizeof(boardText) - 1;

    cout << boardText << endl;

    char board[len + 1];
    board[len] = 0;

    for (int i = 0; i < len; i++) {
        board[i] = charToPlayerNumber(boardText[i]);
    }


    unsigned char *entry = db.get(len, board);

    cout << "-----" << endl;
    for (int i = 0; i < 1 + 2 * sizeof(uint64_t); i++) {
        cout << (int) entry[i] << " ";
    }
    cout << endl;
    cout << "-----" << endl;


    cout << "Len: " << len << endl;
    cout << "Entry: " << (int *) entry << endl;
    uint64_t bDom = DB_GET_DOMINATED(entry, 1);
    uint64_t wDom = DB_GET_DOMINATED(entry, 2);
    cout << "Outcome, domBlack, domWhite: " << DB_GET_OUTCOME(entry) << " " << bDom << " " << wDom << endl;


    State state(boardText, 1);

    size_t moveCount = 0;
    int *moves = state.getMoves(1, 2, &moveCount);

    cout << "Moves:" << endl;
    for (int i = 0; i < moveCount; i++) {
        cout << moves[2 * i] << + " - " << moves[2 * i + 1] << endl;
    }

    cout << "Dominated moves:" << endl;
    for (int i = 0; i < 64; i++) {
        if ((bDom >> i) & ((uint64_t) 1)) {
            cout << i << endl;
        }
    }





    return 0;
}
