#pragma once

#include "state.h"
#include "database.h"
#include <chrono>
#include <random>

#define BOARD(te) te
#define PLAYER(te) *(te + boardSize)
#define OUTCOME(te) *(te + boardSize + 1)
#define BESTMOVE(te) *(te + boardSize + 2)
#define DEPTH(te) *(te + boardSize + 3)
#define HEURISTIC(te) *(te + boardSize + 4)

extern int node_count;
extern int best_from;
extern int best_to;

class BasicSolver2 {
  private:
    int maxDepth;

    //this limits the number of searches, for the current depth, that can reach leaf nodes
    //search is aborted after this
    int64_t maxCompleted;
    int64_t completed;

    Database *db;

    std::default_random_engine *rng;

    void simplify(State *state);
  public:
    int rootPlayer;
    int boardSize;

    //unset to allow searches to reach leaf nodes uninterrupted
    bool limitCompletions;

    double timeLimit;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    

    //check this after calling updateTime to see if time is up
    bool outOfTime;


    //transposition table data
    char *table;

    int bitMask2; //bit mask for State code used to index table
    int codeLength2; //how many bits to use as index from the State's code
    int tableEntrySize2; //size in bytes of 1 table entry


    BasicSolver2(int rootPlayer, int boardSize, Database *db);
    ~BasicSolver2();

    //true if the given entry matches the state and player
    bool validateTableEntry2(State *state, int p, char *entry);


    //call this to run the iterative deepening solver
    int solveID2(State *state, int p, int n);


    /*      rootSearchID, searchID

        Visitors for the root and other nodes, in ID solver. Depth starts at 0 and increases with each recursive call.

    if the bool is true, the int is the player who wins, otherwise it is the heuristic score
    */
    std::pair<int, bool> rootSearchID2(State *state, int p, int n, int depth);
    std::pair<int, bool> searchID2(State *state, int p, int n, int depth);


    //basic solver functions
    //int solve(State *state, int p, int n);
    //int solveRoot(State *state, int p, int n);


    //return a table entry pointer based on a State's code. Use validateTableEntry
    //to see if the entry is a match
    char *getTablePtr2(int code);

    //Call this when a node is being visited
    //check outOfTime to see if time is up
    void updateTime2();

    //void setTableEntry(int code, char *board, char player, char outcome);

};



std::vector<std::pair<int, int>> generateSubgames2(State *state);
