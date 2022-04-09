#include <iostream>
#include <cstring>
#include "utils.h"
#include "state.h"
#include "solver.h"
#include "database.h"
#include <chrono>
#include <map>

using namespace std;


int gameResult(Database &db, char *board, int boardSize, int player) {
    int result;

    auto start = chrono::steady_clock::now();
    BasicSolver *solver = new BasicSolver(player, boardSize, &db);
    solver->timeLimit = 1000000000.0;
    solver->startTime = start;


    char boardText[boardSize + 1];
    //memcpy(boardText, board, boardSize);
    boardText[boardSize] = 0;
    for (int i = 0; i < boardSize; i++) {
        boardText[i] = playerNumberToChar(board[i]);
    }

    State *root = new State(boardText, player);

    result = solver->solveID(root, player, opponentNumber(player));

    delete solver;
    delete root;

    return result;
}

char *addGames(char *g1, char *g2) {
    size_t l1 = strlen(g1);
    size_t l2 = strlen(g2);

    char *g3 = new char[l1 + l2 + 2];

    memcpy(g3, g1, l1);
    memcpy(g3 + l1 + 1, g2, l2);
    g3[l1] = 0;
    g3[l1 + l2 + 1] = 0;

    return g3;
}


void computeBounds(Database &db, char *board, int8_t *bounds) {
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

        char *g = addGames(board, compare);

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



using namespace std;

void printBits(int x, int length) {
    for (int i = 0; i < length; i++) {
        cout << ((x >> i) & 1);
    }
    cout << endl;
}

int main() {

    Database db;
    //db.load(); //might be wrong to load here

    //(low, high, outcome) --> pointer to vector of udMoveCount1, link1, udMoveCount2, link2, ...
    map<triple<int, int, int>, vector<int> *> udMap;

    int maxLength = DB_MAX_BITS;
    int maxGame = 0;

    for (int length = 1; length <= maxLength; length++) {

        maxGame *= 2;
        maxGame += 1;

        char boardText[length + 1];
        char board[length + 1];
        char mirrorBoard[length + 1];

        boardText[length] = 0;
        board[length] = 0;
        mirrorBoard[length] = 0;

        unsigned char *entry;

        for (int game = 0; game <= maxGame; game++) {
            cout << "Length - Game: " << length << " " << game << endl;
            printBits(game, length);


            bool mirror = false;

            if (game > (maxGame + 2) / 2) {
                mirror = true;
            }

            //mirror = false;

            for (int i = 0; i < length; i++) {
                if ((game >> i) & 1) {
                    boardText[i] = 'W';
                    board[i] = WHITE;
                } else {
                    boardText[i] = 'B';
                    board[i] = BLACK;
                }
            }

            if (mirror) {
                for (int i = 0; i < length; i++) {
                    if ((game >> i) & 1) {
                        mirrorBoard[i] = BLACK;
                    } else {
                        mirrorBoard[i] = WHITE;
                    }
                }
            }



            
            for (int i = 0; i < length; i++) {
                cout << boardText[i];
            }
            cout << endl;

            int outcome = 0;
            uint64_t domBlack = 0;
            uint64_t domWhite = 0;
            int8_t lowerBound = 0;
            int8_t upperBound = 0;
            int udMoveCount = 0;
            int link = 0;

            if (mirror) {
                entry = db.get(length, mirrorBoard);
                outcome = DB_GET_OUTCOME(entry);
                domBlack = DB_GET_DOMINATED(entry, 1);
                domWhite = DB_GET_DOMINATED(entry, 2);
                lowerBound = DB_GET_BOUND(entry, 0);
                upperBound = DB_GET_BOUND(entry, 1);
                udMoveCount = DB_GET_UDMOVECOUNT(entry);

                //TODO mirror links somehow

                switch (outcome) {
                    case OC_B:
                        outcome = OC_W;
                        break;

                    case OC_W:
                        outcome = OC_B;
                        break;

                    case OC_N:
                        outcome = OC_N;
                        break;

                    case OC_P:
                        outcome = OC_P;
                        break;
                }


                entry = db.get(length, board);
                link = db.getIdx(length, board);

                DB_SET_OUTCOME(entry, outcome);
                DB_SET_DOMINATED(entry, 1, domWhite); //invert dominance
                DB_SET_DOMINATED(entry, 2, domBlack);
                DB_SET_BOUND(entry, 0, -upperBound);
                DB_SET_BOUND(entry, 1, -lowerBound);
                DB_SET_UDMOVECOUNT(entry, udMoveCount);
                DB_SET_LINK(entry, link);

                cout << endl;
                continue;
            }


            cout << "solving" << endl;




            int result1, result2;

            {
                auto start = chrono::steady_clock::now();
                BasicSolver *solver = new BasicSolver(1, length, &db);
                solver->timeLimit = 1000000000.0;
                solver->startTime = start;

                State *root = new State(boardText, 1);
                
                result1 = solver->solveID(root, 1, opponentNumber(1));

                delete solver;
                delete root;
            }

            {
                auto start = chrono::steady_clock::now();
                BasicSolver *solver = new BasicSolver(2, length, &db);
                solver->timeLimit = 1000000000.0;
                solver->startTime = start;

                State *root = new State(boardText, 2);
                
                result2 = solver->solveID(root, 2, opponentNumber(2));

                delete solver;
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

            cout << "Lookup" << endl;
            entry = db.get(length, board);
            int link = db.getIdx(length, board);

            //find bounds
            if (length <= DB_MAX_BOUND_BITS) {
        
                int8_t bounds[2];
                computeBounds(db, board, bounds);

                DB_SET_BOUND(entry, 0, bounds[0]);
                DB_SET_BOUND(entry, 1, bounds[1]);

                cout << "<" << (int) bounds[0] << " " << (int) bounds[1] << ">" << endl;
            }




            if (DB_GET_OUTCOME(entry) != 0) {
                cout << "Overwriting outcome: " << DB_GET_OUTCOME(entry) << endl;
                while(1){}
            }
            DB_SET_OUTCOME(entry, outcome);


            if (length <= DB_MAX_DOMINANCE_BITS ) {
                int udMoveCount = 0;

                //find dominated moves for B
                char sumBoard[length + 1 + length + 1];
                sumBoard[length + 1 + length] = 0;
                sumBoard[length] = E;

                {
                    State s1(boardText, 1);
                    State s2(boardText, 1);

                    char boardCopy[length];
                    memcpy(boardCopy, s2.board, length);

                    size_t moveCount = 0;
                    int *moves = s1.getMoves(1, 2, &moveCount);

                    udMoveCount += moveCount;

                    char undo1[sizeof(int) + 2 * sizeof(char)];
                    char undo2[sizeof(int) + 2 * sizeof(char)];

                    for (int i = 0; i < moveCount; i++) {
                        s1.play(moves[2 * i], moves[2 * i + 1], undo1);

                        for (int j = i + 1; j < moveCount; j++) {
                            s2.play(moves[2 * j], moves[2 * j + 1], undo2);

                            memcpy(sumBoard, s1.board, length);
                            memcpy(&sumBoard[length + 1], s2.board, length);


                            for (int k = length + 1; k < 2 * length + 1; k++) {
                                sumBoard[k] = opponentNumber(sumBoard[k]);
                            }


                            int bFirst = gameResult(db, sumBoard, 2 * length + 1, 1);
                            int wFirst = gameResult(db, sumBoard, 2 * length + 1, 2);

                            memcpy(s2.board, boardCopy, length);

                            if (bFirst == wFirst) {
                                if (bFirst == 1) { // I - J is positive for black --> I > J
                                    domBlack |= (((uint64_t) 1) << j);
                                } else { // I - J is negative for black --> I < J
                                    domBlack |= (((uint64_t) 1) << i);
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

                //find dominated moves for W
                {
                    State s1(boardText, 2);
                    State s2(boardText, 2);

                    char boardCopy[length];
                    memcpy(boardCopy, s2.board, length);

                    size_t moveCount = 0;
                    int *moves = s1.getMoves(2, 1, &moveCount);

                    udMoveCount += moveCount;

                    char undo1[sizeof(int) + 2 * sizeof(char)];
                    char undo2[sizeof(int) + 2 * sizeof(char)];

                    for (int i = 0; i < moveCount; i++) {
                        s1.play(moves[2 * i], moves[2 * i + 1], undo1);

                        for (int j = i + 1; j < moveCount; j++) {
                            s2.play(moves[2 * j], moves[2 * j + 1], undo2);

                            memcpy(sumBoard, s1.board, length);
                            memcpy(&sumBoard[length + 1], s2.board, length);


                            for (int k = length + 1; k < 2 * length + 1; k++) {
                                sumBoard[k] = opponentNumber(sumBoard[k]);
                            }


                            int bFirst = gameResult(db, sumBoard, 2 * length + 1, 1);
                            int wFirst = gameResult(db, sumBoard, 2 * length + 1, 2);

                            memcpy(s2.board, boardCopy, length);


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


                DB_SET_DOMINATED(entry, 1, domBlack);
                DB_SET_DOMINATED(entry, 2, domWhite);

                udMoveCount -= (sumBits(domBlack) + sumBits(domWhite));
                DB_SET_UDMOVECOUNT(entry, udMoveCount);

                int low = DB_GET_BOUND(entry, 0);
                int high = DB_GET_BOUND(entry, 1);
                int outcome = DB_GET_OUTCOME(entry);

                triple<int, int, int> mapTriple(low, high, outcome);

                vector<int> *udVec = udMap[mapTriple];

                if (udVec == nullptr) {
                    udVec = new vector<int>();
                    udMap[mapTriple] = udVec;
                }

                udVec->push_back(udMoveCount);
                udVec->push_back(link);

                cout << domBlack << " " << domWhite << endl;
            }



            cout << endl;
        }

    }

    cout << "Starting second pass" << endl;

    maxLength = min(DB_MAX_BITS, min(DB_MAX_DOMINANCE_BITS, DB_MAX_BOUND_BITS));
    maxGame = 0;

    for (int length = 1; length <= maxLength; length++) {

        maxGame *= 2;
        maxGame += 1;

        char boardText[length + 1];
        char board[length + 1];
        char mirrorBoard[length + 1];

        boardText[length] = 0;
        board[length] = 0;
        mirrorBoard[length] = 0;

        unsigned char *entry;

        for (int game = 0; game <= maxGame; game++) {
            cout << "Length - Game: " << length << " " << game << endl;
            printBits(game, length);


            bool mirror = false;

            if (game > (maxGame + 2) / 2) {
                mirror = true;
            }

            //mirror = false;

            for (int i = 0; i < length; i++) {
                if ((game >> i) & 1) {
                    boardText[i] = 'W';
                    board[i] = WHITE;
                } else {
                    boardText[i] = 'B';
                    board[i] = BLACK;
                }
            }

            if (mirror) {
                for (int i = 0; i < length; i++) {
                    if ((game >> i) & 1) {
                        mirrorBoard[i] = BLACK;
                    } else {
                        mirrorBoard[i] = WHITE;
                    }
                }
            }



            
            for (int i = 0; i < length; i++) {
                cout << boardText[i];
            }
            cout << endl;

            int outcome = 0;
            uint64_t domBlack = 0;
            uint64_t domWhite = 0;
            int8_t lowerBound = 0;
            int8_t upperBound = 0;
            int udMoveCount = 0;

        





    db.save();
    return 0;
}
