#include <iostream>
#include "utils.h"
#include "state.h"
#include "solver.h"
#include "database.h"
#include <chrono>

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


    int maxLength = DB_MAX_BITS;
    int maxGame = 0;

    for (int length = 1; length <= maxLength; length++) {

        maxGame *= 2;
        maxGame += 1;

        char boardText[length];
        char board[length];

        char mirrorBoard[length];
        unsigned char *entry;

        for (int game = 0; game <= maxGame; game++) {
            cout << "Length - Game: " << length << " " << game << endl;
            printBits(game, length);


            bool mirror = false;

            if (game > (maxGame + 2) / 2) {
                mirror = true;
            }

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

            if (mirror) {
                entry = db.get(length, mirrorBoard);
                outcome = DB_GET_OUTCOME(entry);

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
                DB_SET_OUTCOME(entry, outcome);

                cout << endl;
                continue;
            }


            cout << "solving" << endl;
            int result1, result2;

            {
                auto start = std::chrono::steady_clock::now();
                BasicSolver *solver = new BasicSolver(1, length, &db);
                solver->timeLimit = 1000000000.0;
                solver->startTime = start;

                State *root = new State(boardText, 1);
                
                result1 = solver->solveID(root, 1, opponentNumber(1));

                delete solver;
                delete root;
            }

            {
                auto start = std::chrono::steady_clock::now();
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

            entry = db.get(length, board);
            DB_SET_OUTCOME(entry, outcome);
            cout << endl;
        }

    }


    db.save();
    return 0;
}
