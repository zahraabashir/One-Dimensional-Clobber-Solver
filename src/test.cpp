#include <iostream>
#include "utils.h"
#include "database3.h"
#include <cassert>

using namespace std;

int main() {
    Database db;
    db.init();

    cout << "-1 is " << (uint64_t) -1 << endl;

    uint8_t arr[] = {2, 1, 0, 2, 2, 1};
    size_t len = sizeof(arr) / sizeof(uint8_t);


    uint64_t idx = db.getIdx(arr, len);
    cout << idx << endl;


        // Uncomment to test database get
    /*
    cout << "Testing database get" << endl;
    uint64_t totalChecked = 0;

    for (size_t len = 1; len <= DB_MAX_BITS; len++) {
        uint8_t board[len] = {};

        uint64_t gameCount = 1;
        for (size_t j = 0; j < len; j++) {
            gameCount *= 3;
        }

        for (size_t g = 0; g < gameCount; g++) {
            //print board
            //cout << g << ": ";
            for (size_t j = 0; j < len; j++) {
               // cout << (int) board[j] << " ";
            }
            //cout << endl;

            uint8_t *entry = db.get(board, len);
            totalChecked += 1;
            //cout << "Got " << (uint64_t *) entry << endl;
            if (entry) {
                assert(*db_get_outcome(entry) == 0);
            }

            //add board
            board[0] += 1;
            for (size_t i = 0; i < len; i++) {
                if (board[i] >= 3) {
                    board[i] %= 3;
                    board[i + 1] += 1;
                }
            }

        }
        cout << endl;
        cout << "Done testing " << len << endl;
    }
    cout << "Checked " << totalChecked << endl;
    */




    return 0;
}
