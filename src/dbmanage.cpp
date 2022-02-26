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


    int maxLength = 16;
    int maxGame = 0;

    for (int length = 1; length <= maxLength; length++) {

        maxGame *= 2;
        maxGame += 1;

        char boardText[length];
        char board[length];

        for (int game = 0; game <= maxGame; game++) {
            cout << "Length - Game: " << length << " " << game << endl;
            printBits(game, length);

            for (int i = 0; i < length; i++) {
                if ((game >> i) & 1) {
                    boardText[i] = 'W';
                    board[i] = WHITE;
                } else {
                    boardText[i] = 'B';
                    board[i] = BLACK;
                }
            }

            for (int i = 0; i < length; i++) {
                cout << boardText[i];
            }
            cout << endl;



            // check for game in database and maybe solve it
            int outcome = db.get(length, board);
            if (outcome != 0) {
                cout << endl;
                continue;
            }

            cout << "solving" << endl;
            int result1, result2;

            {
                auto start = std::chrono::steady_clock::now();
                BasicSolver *solver = new BasicSolver(1, length);
                solver->timeLimit = 1000000000.0;
                solver->startTime = start;

                State *root = new State(boardText, 1);
                
                result1 = solver->solveID(root, 1, opponentNumber(1));

                delete solver;
                delete root;
            }

            {
                auto start = std::chrono::steady_clock::now();
                BasicSolver *solver = new BasicSolver(2, length);
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

            db.set(length, board, outcome);
            cout << endl;
        }

    }




    return 0;
}
