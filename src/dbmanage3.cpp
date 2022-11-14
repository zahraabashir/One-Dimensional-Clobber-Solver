#include <iostream>
#include <vector>
#include <algorithm>

#include "solver.h"
#include "database3.h"
#include "utils.h"

using namespace std;

Database *db;
Solver *solver;

/*
void computeBounds(uint8_t *board, size_t boardLen, uint *bounds) {
    char compare[32];

    map<int, int> ltFlags; //-1 false, 0 unknown, 1 true
    map<int, int> gtFlags;

    bool found = false;

    for (int i = 0; abs(i) < 31; i = i >= 0 ? -(i + 1) : -i) {
        //cout << "[" << i << "]" << endl;

        memset(compare, 0, 32);
        char player = i < 0 ? 1 : 2; //we're adding the negative of compare -- flip 1 and 2


        compare[0] = opponentNumber(player);

        for (int j = 0; j < abs(i); j++) {
            compare[j + 1] = player;
        }

        char *g = addGames(strlen(board), board, strlen(compare), compare);

        int boardSize = strlen(board) + 1 + strlen(compare);

        //for (int j = 0; j < boardSize; j++) {
        //    cout << playerNumberToChar(g[j]);
        //}
        //cout << endl;

        int result1 = gameResult(db, g, boardSize, 1);
        int result2 = gameResult(db, g, boardSize, 2);

        delete[] g;

        int outcomeClass;
        if (result1 == result2) {
            outcomeClass = result1;
        } else {
            if (result1 == 1) {
                outcomeClass = OC_N;
            } else {
                outcomeClass = OC_P;
            }
        }


        // C >= G --> 0 >= G - C --> G - C <= 0
        gtFlags[i] = (outcomeClass == OC_W || outcomeClass == OC_P) ? 1 : -1;

        // C <= G --> 0 <= G - C --> G - C >= 0
        ltFlags[i] = (outcomeClass == OC_B || outcomeClass == OC_P) ? 1 : -1;
 

        //Now check if we can determine the bound from this
        int low, high;
        bool foundLow = false;
        bool foundHigh = false;

        for (int j = -31; j < 31; j++) {
            if (ltFlags[j] == 1 && ltFlags[j + 1] == -1) {
                low = j;
                foundLow = true;
                break;
            }
            
        }

        for (int j = 31; j > -31; j--) {
            if (gtFlags[j] == 1 && gtFlags[j - 1] == -1) {
                high = j;
                foundHigh = true;
                break;
            }
        }

        if (foundLow && foundHigh) {
            bounds[0] = low;
            bounds[1] = high;
            found = true;
            break;
        }

    }

    if (!found || bounds[0] > bounds[1]) {
        cout << "Bounds not found..." << endl;
        cout << "{" << (int) bounds[0] << " " << (int) bounds[1] << "}" << endl;


        cout << "LT" << endl;
        for (int i = -32; i < 32; i++) {
            cout << "(" << i << " " << ltFlags[i] << ") ";
        }
        cout << endl;

        cout << "GT" << endl;
        for (int i = -32; i < 32; i++) {
            cout << "(" << i << " " << gtFlags[i] << ") ";
        }
        cout << endl;

        while (1) {
        }
    }

}
*/


int main() {
    //Initialize: DB, solver, shapes
    db = new Database();
    db->init();

    solver = new Solver(DB_MAX_BITS, db);

    vector<vector<int>> shapeList = makeShapes();

    sort(shapeList.begin(), shapeList.end(),
        [](const vector<int> &s1, const vector<int> &s2) {
            return shapeToNumber(s1) < shapeToNumber(s2);
        }
    );

    //Iterate over all shapes
    for (const vector<int> &shape : shapeList) {
        uint64_t shapeNumber = shapeToNumber(shape);

        //iterate over all games
        int gameBits = 0;
        for (int chunk : shape) {
            gameBits += chunk;
        }

        uint32_t minGame = 0;
        uint32_t gameCount = 1 << gameBits;

        for (uint32_t gameNumber = minGame; gameNumber < gameCount; gameNumber++) {
            //Get game and print it
            uint8_t *board;
            size_t boardLen;
            makeGame(shapeNumber, gameNumber, &board, &boardLen);
            cout << "Shape " << shape << " (" << shapeNumber << ") ";
            printBoard(board, boardLen, true);

            //Get entry
            uint64_t idx = db->getIdx(board, boardLen);
            cout << "idx " << idx << endl;
            uint8_t *entry = db->getFromIdx(idx);
            assert(entry);
            assert(*db_get_outcome(entry) == 0);

            //Set trivial values (self link, shape, number)
            *db_get_link(entry) = idx;
            *db_get_shape(entry) = shapeNumber;
            *db_get_number(entry) = gameNumber;

            //Compute outcome class
            int result1 = solver->solveID(board, boardLen, BLACK);
            int result2 = solver->solveID(board, boardLen, WHITE);

            uint8_t outcomeClass = OC_UNKNOWN;

            if (result1 == result2) {
                outcomeClass = result1;
            } else {
                if (result1 == BLACK) {
                    outcomeClass = OC_N;
                } else {
                    outcomeClass = OC_P;
                }
            }


            *db_get_outcome(entry) = outcomeClass;
            //cout << (int) outcomeClass << " " << (int) *db_get_outcome(entry) << endl;


            //compute dominance
            if (boardLen <= DB_MAX_DOMINANCE_BITS) {
            }


            //compute bounds TODO
            //int8_t[2] bounds;
            //computeBounds(board, boardLen, bounds);
            //db_get_bounds(entry)[0] = bounds[0];
            //db_get_bounds(entry)[1] = bounds[1];



            //compute metric
            //add to map




            delete[] board;
        }


    }

    //do second pass to find links


    db->save();

    delete solver;
    delete db;
}
