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

char *addGames(size_t l1, char *g1, size_t l2, char *g2) {
    char *g3 = new char[l1 + 1 + l2 + 1];

    memcpy(g3, g1, l1);
    memcpy(g3 + l1 + 1, g2, l2);
    g3[l1] = 0;
    g3[l1 + l2 + 1] = 0;

    return g3;
}

void computeBounds(int boardSize, char *board, int8_t *bounds) {
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

        char *g = addGames(boardSize, board, strlen(compare), compare);

        int gSize = boardSize + 1 + strlen(compare);

        //for (int j = 0; j < boardSize; j++) {
        //    cout << playerNumberToChar(g[j]);
        //}
        //cout << endl;

        int result1 = gameResult(gSize, g, 1);
        int result2 = gameResult(gSize, g, 2);

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
            int UDMoveCount = 0;

            if (bits <= DB_MAX_DOMINANCE_BITS) {
                //find dominated moves for B
                char sumBoard[boardSize + 1 + boardSize + 1];
                sumBoard[boardSize + 1 + boardSize] = 0;
                sumBoard[boardSize] = 0;

                {
                    State s1(boardText, 1);
                    State s2(boardText, 1);

                    char boardCopy[boardSize];
                    memcpy(boardCopy, s2.board, boardSize);

                    size_t moveCount = 0;
                    int *moves = s1.getMoves(1, 2, &moveCount);

                    UDMoveCount += moveCount;

                    char undo1[sizeof(int) + 2 * sizeof(char)];
                    char undo2[sizeof(int) + 2 * sizeof(char)];

                    for (int i = 0; i < moveCount; i++) {
                        s1.play(moves[2 * i], moves[2 * i + 1], undo1);

                        for (int j = i + 1; j < moveCount; j++) {
                            if (((domBlack >> i) & 1) && ((domBlack >> j) & 1)) {
                                //savedDuplicates += 1;
                                continue;
                            }

                            s2.play(moves[2 * j], moves[2 * j + 1], undo2);

                            memcpy(sumBoard, s1.board, boardSize);
                            memcpy(&sumBoard[boardSize + 1], s2.board, boardSize);


                            for (int k = boardSize + 1; k < 2 * boardSize + 1; k++) {
                                sumBoard[k] = opponentNumber(sumBoard[k]);
                            }


                            int bFirst = gameResult(2 * boardSize + 1, sumBoard, 1);
                            int wFirst = gameResult(2 * boardSize + 1, sumBoard, 2);

                            memcpy(s2.board, boardCopy, boardSize);

                            if (bFirst == wFirst) {
                                if (bFirst == 1) { // I - J is positive for black --> I > J
                                    domBlack |= (((uint64_t) 1) << j);
                                } else { // I - J is negative for black --> I < J
                                    domBlack |= (((uint64_t) 1) << i);
                                }
                            }
                        }

                        s1.undo(undo1);
                    }

                    if (moveCount > 0) {
                        delete[] moves;
                    }
                }

                //find dominated moves for W
                {
                    State s1(boardText, 2);
                    State s2(boardText, 2);

                    char boardCopy[boardSize];
                    memcpy(boardCopy, s2.board, boardSize);

                    size_t moveCount = 0;
                    int *moves = s1.getMoves(2, 1, &moveCount);

                    UDMoveCount += moveCount;

                    char undo1[sizeof(int) + 2 * sizeof(char)];
                    char undo2[sizeof(int) + 2 * sizeof(char)];

                    for (int i = 0; i < moveCount; i++) {
                        s1.play(moves[2 * i], moves[2 * i + 1], undo1);

                        for (int j = i + 1; j < moveCount; j++) {
                            if (((domWhite >> i) & 1) && ((domWhite >> j) & 1)) {
                                //savedDuplicates += 1;
                                continue;
                            }


                            s2.play(moves[2 * j], moves[2 * j + 1], undo2);

                            memcpy(sumBoard, s1.board, boardSize);
                            memcpy(&sumBoard[boardSize + 1], s2.board, boardSize);


                            for (int k = boardSize + 1; k < 2 * boardSize + 1; k++) {
                                sumBoard[k] = opponentNumber(sumBoard[k]);
                            }


                            int bFirst = gameResult(2 * boardSize + 1, sumBoard, 1);
                            int wFirst = gameResult(2 * boardSize + 1, sumBoard, 2);

                            memcpy(s2.board, boardCopy, boardSize);


                            if (bFirst == wFirst) {
                                if (bFirst == 2) { // I - J is positive for white --> I > J
                                    domWhite |= (((uint64_t) 1) << j);
                                } else { // I - J is negative for white --> I < J
                                    domWhite |= (((uint64_t) 1) << i);
                                }
                                //while (1) {}
                            }



                            //s2.undo(undo2);
                        }

                        s1.undo(undo1);
                    }

                    if (moveCount > 0) {
                        delete[] moves;
                    }
                }
            } //end of dominance stuff
            DB_SET_DOMINATED(entry, 1, domBlack);
            DB_SET_DOMINATED(entry, 2, domWhite);

            //lowerBound, upperBound
            int8_t bounds[2] = {0, 0};

            if (bits <= DB_MAX_BOUND_BITS) {
                //computeBounds(boardSize, board, bounds);

                DB_SET_BOUND(entry, 0, bounds[0]);
                DB_SET_BOUND(entry, 1, bounds[1]);

                cout << "<" << (int) bounds[0] << " " << (int) bounds[1] << ">" << endl;
            }

            //metric
            int metric = UDMoveCount;
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
