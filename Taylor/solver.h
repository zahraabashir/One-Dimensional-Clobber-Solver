#pragma once

#include "state.h"

extern int node_count;
class BasicSolver {
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
    //void setTableEntry(int code, char *board, char player, char outcome);
    char *getTablePtr(int code);

};





