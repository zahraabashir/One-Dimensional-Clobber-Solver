#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <map>

#include "database2.h"
#include "utils.h"
#include "solver.h"

using namespace std;

Database *db = NULL;
BasicSolver *solver = NULL;


int gameResult(int boardSize, char *board, int player);


int gameResult(int boardSize, char *board, int player) {
    int result;

    auto start = chrono::steady_clock::now();


    #if defined(SOLVER_FIX_MEMORY_LEAK)
    //BasicSolver *solver = new BasicSolver(player, FIXED_BOARD_SIZE, &db);

    char boardText[FIXED_BOARD_SIZE + 1];
    memset(boardText, '.', FIXED_BOARD_SIZE);
    boardText[FIXED_BOARD_SIZE] = 0;

    for (int i = 0; i < boardSize; i++) {
        boardText[i] = playerNumberToChar(board[i]);
    }
    #else
    //BasicSolver *solver = new BasicSolver(player, boardSize, &db);
    solver->rootPlayer = player;

    char boardText[boardSize + 1];
    boardText[boardSize] = 0;
    for (int i = 0; i < boardSize; i++) {
        boardText[i] = playerNumberToChar(board[i]);
    }
    #endif

    solver->timeLimit = 1000000000.0;
    solver->startTime = start;



    State *root = new State(boardText, player);

    result = solver->solveID(root, player, opponentNumber(player));

    //delete solver;
    delete root;

    return result;
}







void getShapeList(ShapeNode *node, vector<ShapeNode *> &list) {
    if (node->shape.size() > 0) {
        list.push_back(node);
    }

    for (ShapeNode *child : node->children) {
        getShapeList(child, list);
    }
}

bool shapeListSort(const ShapeNode *n1, const ShapeNode *n2) {
    if (n1->bits < n2->bits) {
        return true;
    }

    for (int i = 0; i < min(n1->shape.size(), n2->shape.size()); i++) {
        if (n1->shape[i] < n2->shape[i]) {
            return true;
        }

        if (n1->shape[i] > n2->shape[i]) {
            return false;
        }
    }

    return false;
}


int main() {
    db = new Database();
    db->initData();

    solver = new BasicSolver(0, 100, db);


    //Get all shapes
    vector<ShapeNode *> shapeList;
    getShapeList(db->shapeTree, shapeList);
    sort(shapeList.begin(), shapeList.end(), shapeListSort);

    for (const ShapeNode *node : shapeList) {
        const vector<int> &shape = node->shape;

        int boardSize = node->bits + shape.size(); //shape.size() and not shape.size() - 1 because of null terminator
        int bits = node->bits;

        char board[boardSize];

        char boardText[boardSize];
        boardText[boardSize - 1] = 0;

        int minGame = 0;
        int maxGame = (1 << node->bits) - 1;

        for (int number = minGame; number <= maxGame; number++) {
            board[boardSize - 1] = 0;
            boardText[boardSize - 1] = 0;

            {
                int shapeIdx = 0;
                int currentChunk = 0;
                int shift = 0;

                for (int i = 0; i < node->bits + shape.size() - 1; i++) {
                    if (currentChunk >= shape[shapeIdx]) {
                        currentChunk = 0;
                        shapeIdx += 1;
                        board[i] = 0;
                        boardText[i] = playerNumberToChar(board[i]);

                        continue;
                    }

                    currentChunk += 1;

                    board[i] = ((number >> shift) & 1) == 0 ? 1 : 2;
                    boardText[i] = playerNumberToChar(board[i]);
                    shift += 1;
                }
            }

            cout << shape << " |" << node->bits <<  "| (" << number << "/" << maxGame << ")" << endl;
            cout << boardText << endl;

            ////////////////////Process board///////////////////////////////////////////////
            unsigned char *entry = db->get(boardSize - 1, board);
            cout << ((uint64_t *) entry) << endl;

            if (entry == 0) {
                cout << "Failed to find entry for: " << boardText << endl;
                while (1) {
                }
            }


            //game number, shape, self link
            DB_SET_NUMBER(entry, number);

            uint64_t snum = shapeVectorToNumber(shape);
            DB_SET_SHAPE(entry, snum);

            int selfLink = db->getEntryLink(entry);
            DB_SET_LINK(entry, selfLink);

            //domBlack, domWhite
            uint64_t domBlack = 0;
            uint64_t domWhite = 0;
            DB_SET_DOMINATED(entry, 1, domBlack);
            DB_SET_DOMINATED(entry, 2, domWhite);

            //lowerBound, upperBound
            int8_t bounds[2] = {0, 0};
            DB_SET_BOUND(entry, 0, bounds[0]);
            DB_SET_BOUND(entry, 1, bounds[1]);


            //metric
            int metric = 0;
            DB_SET_METRIC(entry, metric);

            //outcome
            int8_t outcome;

            int result1, result2;

            {
                auto start = chrono::steady_clock::now();
                //BasicSolver *solver = new BasicSolver(1, length, &db);
                solver->rootPlayer = 1;
                solver->timeLimit = 1000000000.0;
                solver->startTime = start;

                State *root = new State(boardText, 1);
                
                result1 = solver->solveID(root, 1, opponentNumber(1));

                //delete solver;
                delete root;
            }

            {
                auto start = chrono::steady_clock::now();
                //BasicSolver *solver = new BasicSolver(2, length, &db);
                solver->rootPlayer = 2;
                solver->timeLimit = 1000000000.0;
                solver->startTime = start;

                State *root = new State(boardText, 2);
                
                result2 = solver->solveID(root, 2, opponentNumber(2));


                //delete solver;
                delete root;
            }

            outcome = 0;

            if (result1 == result2) {
                outcome = result1;
            } else if (result1 == 1) {
                outcome = OC_N;
            } else {
                outcome = OC_P;
            }

            DB_SET_OUTCOME(entry, outcome);


            //put information into bound map

            ///////////////////End of process board/////////////////////////////////////////

            cout << endl;
        }

    }



    db->save();


    delete db;
    delete solver;




    return 0;
}
