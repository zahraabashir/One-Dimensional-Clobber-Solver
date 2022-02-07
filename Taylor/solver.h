#pragma once

#include "state.h"

extern int node_count;
extern int best_from;
extern int best_to;
class BasicSolver {

    int codeLength;
  public:
    int rootPlayer;
    int rootOpponent;
    int boardSize;


    char *table;
    int bitMask;
    int tableEntrySize;


    BasicSolver(int rootPlayer, int boardSize);
    ~BasicSolver();
    int solve(State *state, int p, int n);
    int solveRoot(State *state, int p, int n);
    //void setTableEntry(int code, char *board, char player, char outcome);
    char *getTablePtr(int code);


};





