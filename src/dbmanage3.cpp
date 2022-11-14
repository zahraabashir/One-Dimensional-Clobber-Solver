#include <iostream>
#include <vector>
#include <algorithm>

#include "solver.h"
#include "database3.h"
#include "utils.h"

using namespace std;

Database *db;
Solver *solver;

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

            //Set trivial values (shape and number)
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
            cout << (int) outcomeClass << " " << (int) *db_get_outcome(entry) << endl;




            delete[] board;
        }


    }


    db->save();

    delete solver;
    delete db;
}
