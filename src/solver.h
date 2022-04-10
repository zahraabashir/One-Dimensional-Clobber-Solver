#pragma once

#define SOLVER_SIMPLIFY

#include "state.h"
#include "database.h"
#include <chrono>
#include <random>
#include "bound.h"

#define BOARDLEN(te) *(te) //uint8_t
#define BOARDPTR(te) *((char **) (te + 1)) //char *
#define PLAYER(te) *(te + 1 + sizeof(char *)) //uint8_t
#define OUTCOME(te) *(te + 1 + sizeof(char *) + 1) //uint8_t
#define BESTMOVE(te) *(te + 1 + sizeof(char *) + 2) //uint8_t
#define DEPTH(te) *((unsigned int *) (te + 1 + sizeof(char *) + 3)) //unsigned int
#define HEURISTIC(te) *(te + 1 + sizeof(char *) + 3 + sizeof(unsigned int)) //int8_t

#define RESIZETTBOARD(entry, newSize) \
{ \
    uint8_t oldSize = BOARDLEN(entry); \
    if (oldSize != newSize) { \
        if (oldSize != 0) { \
            delete[] BOARDPTR(entry); \
        } \
        if (newSize != 0) { \
            BOARDPTR(entry) = new char[newSize]; \
        } \
        BOARDLEN(entry) = newSize; \
    } \
}

#define RESIZESTATEBOARD(state, newSize) \
{ \
    int oldSize = state->boardSize; \
    if (oldSize != newSize) { \
        if (oldSize != 0) { \
            delete[] state->board; \
        } \
        if (newSize != 0) { \
            state->board = new char[newSize]; \
        } \
        state->boardSize = newSize; \
    } \
}

extern int node_count;
extern int best_from;
extern int best_to;

class BasicSolver {
  private:
    int maxDepth;

    size_t entryCount;

    //this limits the number of searches, for the current depth, that can reach leaf nodes
    //search is aborted after this
    int64_t maxCompleted;
    int64_t completed;

    Database *db;

    std::default_random_engine *rng;

    void simplify(State *state, int depth);
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

    int bitMask; //bit mask for State code used to index table
    int codeLength; //how many bits to use as index from the State's code
    int tableEntrySize; //size in bytes of 1 table entry


    BasicSolver(int rootPlayer, int boardSize, Database *db);
    ~BasicSolver();

    //true if the given entry matches the state and player
    bool validateTableEntry(State *state, int p, char *entry);


    //call this to run the iterative deepening solver
    int solveID(State *state, int p, int n);


    /*      rootSearchID, searchID

        Visitors for the root and other nodes, in ID solver. Depth starts at 0 and increases with each recursive call.

    if the bool is true, the int is the player who wins, otherwise it is the heuristic score
    */
    std::pair<int, bool> rootSearchID(State *state, int p, int n, int depth);
    std::pair<int, bool> searchID(State *state, int p, int n, int depth);


    //basic solver functions
    //int solve(State *state, int p, int n);
    //int solveRoot(State *state, int p, int n);


    //return a table entry pointer based on a State's code. Use validateTableEntry
    //to see if the entry is a match
    char *getTablePtr(int code);

    //Call this when a node is being visited
    //check outOfTime to see if time is up
    void updateTime();

    //void setTableEntry(int code, char *board, char player, char outcome);


    int checkBounds(State *state);

    void generateBounds(State *state, Bound &alpha, Bound &beta);


};



std::vector<std::pair<int, int>> generateSubgames(State *state);
