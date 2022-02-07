#pragma once

#include "state.h"

#define BOARD(te) te
#define PLAYER(te) *(te + boardSize)
#define OUTCOME(te) *(te + boardSize + 1)
#define BESTMOVE(te) *(te + boardSize + 2)
#define DEPTH(te) *((int *) (te + boardSize + 3))
#define HEURISTIC(te) *((int *) (te + boardSize + 3 + sizeof(int)))


extern int node_count;
extern int best_from;
extern int best_to;
class BasicSolver {

  private:
    int maxDepth;

  public:
    int rootPlayer;
    int rootOpponent;
    int boardSize;



    char *table;
    int bitMask;
    int tableEntrySize;


    BasicSolver(int rootPlayer, int boardSize);
    ~BasicSolver();

    bool validateTableEntry(State *state, int p, char *entry);

    int solveID(State *state, int p, int n);
    std::pair<int, bool> searchID(State *state, int p, int n, int depth);

    int solve(State *state, int p, int n);
    int solveRoot(State *state, int p, int n);
    //void setTableEntry(int code, char *board, char player, char outcome);
    char *getTablePtr(int code);


};





