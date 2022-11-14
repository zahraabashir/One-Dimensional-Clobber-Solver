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
        uint32_t maxGame = 1 << gameBits;

        for (uint32_t gameNumber = minGame; gameNumber <= maxGame; gameNumber++) {
            uint8_t *board;
            size_t boardLen;
            makeGame(shapeNumber, gameNumber, &board, &boardLen);

            cout << "Shape " << shape << " (" << shapeNumber << ") ";
            printBoard(board, boardLen, true);
            delete[] board;


        }


    }


    db->save();

    delete solver;
    delete db;
}
