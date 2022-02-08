#pragma once

#include "state.h"

extern int node_count;
extern int best_from;
extern int best_to;
class BasicSolver {
  public:
    int rootPlayer;
    int rootOpponent;
    int boardSize;


    // char *table;
    // int bitMask;
    // int tableEntrySize;


    BasicSolver(int rootPlayer, int boardSize);
    //negamax
    int solve(State *state, int p, int n);
    int solveRoot(State *state, int p, int n);

    //minimax
    bool solveOr(State *state, int p, int n);
    bool solveAnd(State *state, int p, int n);

    char *getTablePtr(int code);


};





