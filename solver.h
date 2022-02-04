#pragma once
#include "state.h"
#include <string>
#include <tuple>


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
    int DLNegaMax(State *state, int p, int n, int depth);
    int IDSearch(State *state, int p, int n);
    //void setTableEntry(int code, char *board, char player, char outcome);
    char *getTablePtr(int code);

};
