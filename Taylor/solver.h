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


    char *table;
    int bitMask;
    int tableEntrySize;


    BasicSolver(int rootPlayer, int boardSize);
    ~BasicSolver();
    int solve(State *state, int p, int n);
    int solveRoot(State *state, int p, int n);
    //Heuristic ID
    int H_DLNegaMax(State *state, int p, int n, int depth);
    int H_IDSearch(State *state, int p, int n);
   //negamax ID
    int DLNegaMax(State *state, int p, int n, int depth);
    int IDSearch(State *state, int p, int n);
    //minimax
    bool solveOr(State *state, int p, int n);
    bool solveAnd(State *state, int p, int n);

    int tt_IDSearch(State *state, int p, int n);
    int tt_ID_NegaMax(State *state, int p, int n, int depth);
    //void setTableEntry(int code, char *board, char player, char outcome);
    char *getTablePtr(int code);


};





