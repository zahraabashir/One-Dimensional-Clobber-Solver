#pragma once

#include <cstddef>
#include <cstdint>
#include <random>

#include "options.h"
#include "database3.h"

#define BLOCK_SIZE 4

extern uint64_t node_count;
extern int best_from;
extern int best_to;

#define STORED_BEST_MOVES 1

struct TTLayout {
    static constexpr size_t arr[] = {
        //sz(uint8_t),        // board length
        //sz(uint8_t *),      // board pointer
        sz(uint8_t),        // player
        sz(uint8_t),        // outcome
        sz(int8_t[STORED_BEST_MOVES]),     // best moves
        sz(unsigned int),   // depth
        sz(int8_t),         // heuristic
        sz(bool),           // valid
        sz(uint64_t),       // hash
    };

    static constexpr size_t N = sizeof(arr) / sizeof(size_t);

    //size of one entry
    static constexpr size_t size() {
        size_t sum = 0;

        for (size_t i = 0; i < N; i++) {
            sum += arr[i];
        }

        return sum;
    }
};

enum {
    //TT_LENGTH = 0,
    //TT_BOARD,
    TT_PLAYER = 0,
    TT_OUTCOME,
    TT_BEST_MOVES,
    TT_DEPTH,
    TT_HEURISTIC,
    TT_VALID,
    TT_HASH,
    TT_COST,
};

//uint8_t *tt_get_length(uint8_t *entry);
//uint8_t **tt_get_board(uint8_t *entry);
uint8_t *tt_get_player(uint8_t *entry);
uint8_t *tt_get_outcome(uint8_t *entry);
int8_t *tt_get_best_moves(uint8_t *entry);
unsigned int *tt_get_depth(uint8_t *entry);
int8_t *tt_get_heuristic(uint8_t *entry);
bool *tt_get_valid(uint8_t *entry);
uint64_t *tt_get_hash(uint8_t *entry);

class Solver {
  private:
    size_t boardLen;

    //size_t tableEntryCount;
    static constexpr size_t blockHeaderSize = BLOCK_SIZE;
    size_t blockCount;
    static constexpr size_t tableEntrySize = TTLayout::size();
    size_t tableSize;

    size_t totalBlockSize;

    bool doABPrune;
    bool limitCompletions;
    uint64_t maxCompleted;
    uint64_t completed;


    int maxDepth;



  public:
    int codeLength;
    int codeMask;

    uint8_t *table;

    Database *db;
    std::default_random_engine *rng;


    Solver(size_t boardLen, Database *db);
    ~Solver();

    int solveID(uint8_t *board, size_t len, int n);

    std::pair<int, bool> searchID(uint8_t *board, size_t boardLen, 
        int n, int p, int depth);


    std::pair<int, bool> rootSearchID(uint8_t *board, size_t boardLen,
        int n, int p, int depth);


    uint8_t *getBlockPtr(uint64_t code);

    uint8_t *getEntryPtr(uint8_t *blockPtr, uint8_t *board, size_t len, int player, uint64_t hash, int mode);


    void simplify(uint8_t **board, size_t *boardLen);
};






