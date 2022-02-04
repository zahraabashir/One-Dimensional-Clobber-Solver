#include "solver.h"
#include "utils.h"
#include <tuple>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>
#define BOARD(te) te
#define PLAYER(te) *(te + boardSize)
#define OUTCOME(te) *(te + boardSize + 1)
bool failure = true;
BasicSolver::BasicSolver(int rootPlayer, int boardSize) {
    this->rootPlayer = rootPlayer;
    this->rootOpponent = opponentNumber(rootPlayer);
    this->boardSize = boardSize;

    int bits = 20;
    //board, player, outcome
    tableEntrySize = boardSize + 2;

    bitMask = 0;
    for (int i = 0; i < bits; i++) {
        bitMask <<= 1;
        bitMask |= 1;
    }

    table = (char *) calloc((1 << bits) * 1 * tableEntrySize, 1);
}

BasicSolver::~BasicSolver() {
    free(table);
}

int BasicSolver::solve(State *state, int p, int n) {

    //Look up state in table
    int code = state->code(p);
    char *entry = getTablePtr(code);

    bool found = false;
    if (PLAYER(entry) == p) {
        found = true;
        for (int i = 0; i < boardSize; i++) {
            if (entry[i] != state->board[i]) {
                found = false;
                break;
            }
        }
    }

    if (found) {
        return OUTCOME(entry);
    }

    size_t moveCount;
    int *moves = state->getMoves(p, n, &moveCount);

    if (moveCount == 0) {
        memcpy(entry, state->board, boardSize);
        PLAYER(entry) = p;
        OUTCOME(entry) = n;

        return n;
    }

    char undoBuffer[sizeof(int) + 2 * sizeof(char)];

    for (size_t i = 0; i < moveCount; i++) {
        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        state->play(from, to, undoBuffer);
        int result = solve(state, n, p);
        state->undo(undoBuffer);

        if (result == p) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = p;

            delete[] moves;
            return p;
        }
    }

    memcpy(entry, state->board, boardSize);
    PLAYER(entry) = p;
    OUTCOME(entry) = n;

    delete[] moves;
    return n;
}
/////////////////////////////
int BasicSolver::DLNegaMax(State *state, int p, int n, int depth) {

    size_t moveCount;
    int *moves = state->getMoves(p, n, &moveCount);
    // std::cout << "moveCount\n"<<moveCount;
    if (moveCount == 0) {
        return n;
    }
    else if (depth == 0){
        // std::cout << "DEPTH = 0\n";
        return  3;
    }

    char undoBuffer[sizeof(int) + 2 * sizeof(char)];
    bool Flag = false;
    for (size_t i = 0; i < moveCount; i++) {
        int from = moves[2 * i];
        int to = moves[2 * i + 1];
        state->play(from, to, undoBuffer);
   
        int result = DLNegaMax(state, n, p, depth-1);
        // std::cout <<"result:"<<result<<"\n"; 
        state->undo(undoBuffer);
        
        if (result == p) {
            // std::cout << "RESULT FOUND = P\n";
            delete[] moves;
            return p;
        }
        if (result == 3 ){
            Flag = true;
        }
    }
    if (Flag){
        delete[] moves;
        return 3;
    }
    // std::cout << "RESULT FOUND = N\n";
    delete[] moves;
    return n;
}
int BasicSolver::IDSearch(State *state, int p, int n) {
    int depth = 0;
    //bool failure = true;
    // state->failure = true;
    int result = 3;
    while(result == 3){
        std :: cout << depth; 
        // std :: cout << "Here\n";
        result= DLNegaMax(state, p, n, depth);
        
        depth +=1;
    }
    return result;
}
char *BasicSolver::getTablePtr(int code) {
    int idx = code & bitMask;
    return table + (idx * tableEntrySize);
}
