#include <iostream>
#include "utils.h"
#include "state.h"
#include "solver.h"
#include <chrono>
#include "options.h"
#include <cstring>


using namespace std;


//NOTE: _argc/argc and _argv/argv are different here...
int main(int _argc, char **_argv) {

    //Do initialization one time
    Database db;
    db.load();

    #if defined(SOLVER_FIX_MEMORY_LEAK)
    BasicSolver solver(rootPlayer, FIXED_BOARD_SIZE, &db);
    #else
    //BasicSolver solver(rootPlayer, board.length(), &db);
    BasicSolver solver(0, 90, &db); //Try setting some root player and large board size...
    #endif



    while (true) {
        solver.reset(); //Just reset visited node count

        string input;
        getline(cin, input);

        if (input.length() == 0) {
            break;
        }

        vector<string> tokens;

        tokens.push_back("./TheSolvers");
        
        {
            int start = 0;
            for (size_t i = 0; i <= input.size(); i++) {
                if (i == input.size() || input[i] == ' ') {
                    string token = input.substr(start, i - start);

                    if (token.size() > 0) {
                        tokens.push_back(token);
                    }

                    start = i + 1;
                }
            }
        }


        int argc = tokens.size();
        char *argv[argc];

        for (int i = 0; i < argc; i++) {
            string &token = tokens[i];

            argv[i] = new char[token.size() + 1];
            argv[i][token.size()] = 0;
            memcpy(argv[i], token.c_str(), token.size());
        }


        auto start = std::chrono::steady_clock::now();

        //Initialize solver and state
        string board(argv[1]);

        int rootPlayer = charToPlayerNumber(*argv[2]);
        double timeLimit = (double) atoi(argv[3]);

        solver.rootPlayer = rootPlayer;


        solver.timeLimit = timeLimit - 0.05 - ((double) board.length()) * 0.002;
        solver.startTime = start;

        #if defined(SOLVER_FIX_MEMORY_LEAK)
        char boardFixed[FIXED_BOARD_SIZE + 1];

        memset(boardFixed, '.', FIXED_BOARD_SIZE);
        boardFixed[FIXED_BOARD_SIZE] = 0;

        memcpy(boardFixed, board.data(), board.size());

        State *root = new State(boardFixed, rootPlayer);
        #else
        State *root = new State(board, rootPlayer);
        #endif

        // int result = solver.solveRoot(root, rootPlayer, opponentNumber(rootPlayer));
        int result = solver.solveID(root, rootPlayer, opponentNumber(rootPlayer));

        //Print output
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - solver.startTime).count();

        if (solver.outOfTime) {
            cout << "?" << " None " << elapsed << " " << node_count;
        } else if (best_from == -1) {
            cout << playerNumberToChar(result) << " None" << " " << elapsed << " " << node_count;
        } else {
            cout << playerNumberToChar(result) << " " << best_from << "-" << best_to << " " << elapsed << " " << node_count;
        }

        cout << endl;

        delete root;

        for (int i = 0; i < argc; i++) {
            delete[] argv[i];
        }


    }


    return 0;
}
