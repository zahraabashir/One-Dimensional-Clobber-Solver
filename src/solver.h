#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <random>
#include <optional>

#include "options.h"
#include "database3.h"

typedef std::pair<int, bool> SolveResult;

struct SubgameRange {
    int start;
    int length;
    int end;
};

inline std::ostream &operator<<(std::ostream &os, const SubgameRange &sr) {
    os << "(" << sr.start << ", " << sr.length << ", " << sr.end << ")";
    return os;
}


#define BLOCK_SIZE 4

extern uint64_t node_count;
extern int best_from;
extern int best_to;

#define STORED_BEST_MOVES 1

typedef int32_t heuristic_t;
constexpr heuristic_t H_MIN = std::numeric_limits<heuristic_t>::min() + 1;
constexpr heuristic_t H_MAX = std::numeric_limits<heuristic_t>::max();
static_assert(H_MIN == -H_MAX);

struct TTLayout {
    static constexpr size_t arr[] = {
        //sz(uint8_t),        // board length
        //sz(uint8_t *),      // board pointer
        sz(uint8_t),        // player
        sz(uint8_t),        // outcome
        sz(int8_t[STORED_BEST_MOVES]),     // best moves
        sz(unsigned int),   // depth
        sz(heuristic_t),         // heuristic
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
heuristic_t *tt_get_heuristic(uint8_t *entry);
bool *tt_get_valid(uint8_t *entry);
uint64_t *tt_get_hash(uint8_t *entry);

class Solver {
  public:
    static bool useBWMoveOrder;
    static bool useID;
    static bool useLinks;
    static bool deleteGames;
    static bool deleteDominated;

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
    static bool doDebug;

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


    std::optional<std::pair<int, bool>> checkWinFromStaticRules(
            const uint8_t *board, size_t boardLen,
            const std::vector<std::pair<int, int>> &subgames,
            std::vector<int> &outcomes, int player);

    void simplify(uint8_t **board, size_t *boardLen);
    void simplifyNew(uint8_t **board, size_t *boardLen);


    std::optional<std::pair<int, bool>> checkStaticResult(uint8_t *dbEntry, int player);


    Subgame *getSimpleBoard(const uint8_t* board, size_t len);
    Subgame *getShortBoard(const uint8_t* board, size_t len);

    std::optional<SolveResult> fullBoardStaticRules(
                                                    uint8_t *dbEntry,
                                                    int player);

    std::optional<SolveResult> subgameStaticRules(
const uint8_t *sboard,
        int player, const std::vector<SubgameRange> &ranges,
        std::vector<int> &outcomes, std::vector<uint8_t> &simpleMoves);



};


inline void pruneInversePairs(std::vector<Subgame*> &subgames) {
    const size_t N = subgames.size();

    for (size_t i = 0; i < N; i++) {
        Subgame *sg1 = subgames[i];
        if (sg1 == nullptr)
            continue;

        for (size_t j = i + 1; j < N; j++) {
            assert(subgames[i] != nullptr);
            Subgame *sg2 = subgames[j];
            if (sg2 == nullptr)
                continue;

            if (Subgame::isVisuallyInversePair(sg1, sg2)) {
                delete sg1;
                delete sg2;
                subgames[i] = nullptr;
                subgames[j] = nullptr;
                break;
            }
        }
    }
}


inline void pruneNoMoveGames(std::vector<Subgame*> &subgames) {
    const size_t nGames = subgames.size();

    for (size_t i = 0; i < nGames; i++) {
        Subgame *sg = subgames[i];

        if (sg == nullptr)
            continue;

        bool hasMoves = false;
        const size_t N = sg->size();

        if (N >= 2) {
            const size_t M = N - 1;
            for (size_t j = 0; j < M; j++) {
                const uint8_t &tile1 = (*sg)[j];
                const uint8_t &tile2 = (*sg)[j + 1];

                if (tile1 + tile2 == 3) {
                    hasMoves = true;
                    break;
                }
            }
        }

        if (!hasMoves) {
            delete sg;
            subgames[i] = nullptr;
        }
    }
}

inline void stitchGames(std::vector<Subgame*> &allGames, uint8_t **board,
                        size_t *boardLen) {

    const size_t nFinalGames = allGames.size();

    const size_t nSpaces = nFinalGames > 1 ? nFinalGames - 1 : 0;
    size_t totalSize = nSpaces;

    for (const Subgame *sg : allGames) {
        assert(sg != nullptr);
        totalSize += sg->size();
    }

    if (totalSize == 0)
        totalSize = 1;

    uint8_t *newBoard = new uint8_t[totalSize];
    for (size_t i = 0; i < totalSize; i++)
        newBoard[i] = 0;

    size_t cumulative = 0;
    for (Subgame *sg : allGames) {
        assert(sg != nullptr);

        const size_t sgLen = sg->size();
        const std::vector<uint8_t> &vec = sg->boardVecConst();

        for (size_t i = 0; i < sgLen; i++)
            newBoard[cumulative++] = vec[i];

        cumulative++;
        delete sg;
    }
    assert(
            (cumulative == totalSize + 1) ||
            (cumulative == 0 && nFinalGames == 0)
            );

    delete[] *board;
    *board = newBoard;
    *boardLen = totalSize;
}

inline bool entryValid(const uint8_t *entry) {
    return (entry != 0) && (*db_get_outcome(entry) != 0);
}


inline Subgame *Solver::getSimpleBoard(const uint8_t* board, size_t len) {
    uint8_t *newBoard = new uint8_t[len];
    memcpy(newBoard, board, len);

    simplifyNew(&newBoard, &len);

    Subgame *game = new Subgame(newBoard, len);
    delete[] newBoard;
    return game;
}

