#include <iostream>
#include "utils.h"
#include "state.h"
#include "solver.h"

// #include <chrono>
// using namespace std::chrono;
  
// Use auto keyword to avoid typing long
// type definitions to get the timepoint
// at this instant use function now()



using namespace std;


int main(int argc, char **argv) {
    // auto start = high_resolution_clock::now();
    if (argc < 4) {
        cout << "Usage:\n" << argv[0] << " <board> <toPlay> <time> [debug]" << endl;
        return 0;
    }





    string board(argv[1]);

    int rootPlayer = charToPlayerNumber(*argv[2]);
    int timelimit = *argv[3];
    BasicSolver solver(rootPlayer, board.length());
    State *root = new State(board, rootPlayer);

/*
    for (size_t i = 0; i < root->moveCount; i++) {
        cout << root->moves[2 * i] << " " << root->moves[2 * i + 1] << endl;
    }

    return 0;
*/

    
    int result = solver.solveRoot(root, rootPlayer, opponentNumber(rootPlayer));
    //auto stop = high_resolution_clock::now();

    // auto finish = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> elapsed = finish - start;
    // std::cout << "Elapsed time: " << elapsed.count() << " s\n";


    if (best_from==-1){
    cout << playerNumberToChar(result)<< " None"<<" "<<node_count;
    }
    else{
        cout << playerNumberToChar(result) <<" "<<best_from<<"-"<< best_to<<" "<<node_count;
    }

    // if (elapsed.count() >= timelimit){
    //     cout << "?" ;
    //     }

    // int result = solver.solveRoot(root, rootPlayer, opponentNumber(rootPlayer));
    
    // if (best_from==-1){
    // cout << playerNumberToChar(result)<< " None"<<" "<<node_count;
    // }
    // else{
    //     cout << playerNumberToChar(result) <<" "<<best_from<<"-"<< best_to<<" "<<node_count;
    // }
    

    return 0;
}
