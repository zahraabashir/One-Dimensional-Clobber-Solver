/*
TODO:

    -- game/board generator
    -- normalize function
    -- computeOutcome()



*/
#include <functional>
#include <iostream>
#include <limits>
#include <ostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstring>
#include <unordered_set>
#include <unordered_map>

#include "options.h"
#include "solver.h"
#include "miscTypes.h"
#include "database3.h"
#include "utils.h"
#include "state.h"
using namespace std;

struct ReplacementMapIdx {
    uint8_t outcome;
    uint8_t low1;
    uint8_t high1;
    uint8_t low2;
    uint8_t high2;

    bool operator==(const ReplacementMapIdx &other) const {
        return 
            (outcome == other.outcome) &&
            (low1 == other.low1) &&
            (high1 == other.high1) &&
            (low2 == other.low2) &&
            (high2 == other.high2);
    }

};

template <>
struct std::hash<ReplacementMapIdx> {
    uint64_t operator()(const ReplacementMapIdx &s) const noexcept {
        return
            s.outcome |
            (((uint64_t) s.low1) << 8) |
            (((uint64_t) s.high1) << 16) |
            (((uint64_t) s.low2) << 24) |
            (((uint64_t) s.high2) << 32);
    }
};

namespace {


////////////////////////////////////////////////// types
enum relation {
    REL_UNKNOWN = 0,
    REL_LESS,
    REL_GREATER,
    REL_EQUAL,
    REL_FUZZY,
};

////////////////////////////////////////////////// globals
Database *db;
Database *alt_db;
Solver *solver;

typedef unordered_map<ReplacementMapIdx, vector<shared_ptr<IndirectLink>>> rmap_t;

rmap_t replacementMap;
rmap_t altReplacementMap;

////////////////////////////////////////////////// helper functions


/*

SCALE:
    -4 ooooox.xo      (4v)
    -3 oooox          (3v)
    -2 ooox.xo        (2v)
    -1 oox            (v)
    0  o OR x         (0)
    1  xxo            (^)
    2  xxxo.xo        (2^)
    3  xxxxo          (3^)
    4  xxxxxo.xo      (4^)
*/
Subgame *getInverseScaleGameNoStar(int8_t scaleIdx) {
    Subgame *sg = new Subgame();
    vector<uint8_t> &boardVec = sg->boardVec();

    if (scaleIdx == 0) {
        boardVec.push_back(BLACK);
        return sg;
    }

    // Inverse. Main color WHITE if positive idx
    const int mainColor = scaleIdx > 0 ? WHITE : BLACK;
    const int singleColor = opponentNumber(mainColor);

    boardVec.push_back(singleColor);

    const int8_t nMainStones = abs(scaleIdx) + 1;
    for (int8_t i = 0; i < nMainStones; i++)
        boardVec.push_back(mainColor);

    if (abs(scaleIdx) % 2 == 0) {
        boardVec.push_back(EMPTY);
        boardVec.push_back(BLACK);
        boardVec.push_back(WHITE);
    }

    return sg;
}

/*

SCALE:
    -4 ooooox            (4v*)
    -3 oooox.xo          (3v*)
    -2 ooox              (2v*)
    -1 oox.xo            (v*)
    0  xo                (0*)
    1  xxo.xo            (^*)
    2  xxxo              (2^*)
    3  xxxxo.xo          (3^*)
    4  xxxxxo            (4^*)

*/
Subgame *getInverseScaleGameWithStar(int8_t scaleIdx) {
    Subgame *sg = new Subgame();
    vector<uint8_t> &boardVec = sg->boardVec();

    if (abs(scaleIdx) % 2 != 0 || scaleIdx == 0) {
        boardVec.push_back(BLACK);
        boardVec.push_back(WHITE);
        boardVec.push_back(EMPTY);
    }

    if (scaleIdx == 0)
        return sg;

    // Inverse. Main color WHITE if positive idx
    const int mainColor = scaleIdx > 0 ? WHITE : BLACK;
    const int singleColor = opponentNumber(mainColor);

    boardVec.push_back(singleColor);

    const int8_t nMainStones = abs(scaleIdx) + 1;
    for (int8_t i = 0; i < nMainStones; i++)
        boardVec.push_back(mainColor);

    return sg;
}

Subgame *getInverseScaleGame(int8_t scaleIdx) {
    Subgame *sg = new Subgame();
    vector<uint8_t> &boardVec = sg->boardVec();

    // Inverse. Main color WHITE if positive idx
    const int mainColor = scaleIdx > 0 ? WHITE : BLACK;
    const int singleColor = opponentNumber(mainColor);

    boardVec.push_back(singleColor);

    const int8_t nMainStones = abs(scaleIdx);
    for (int8_t i = 0; i < nMainStones; i++)
        boardVec.push_back(mainColor);

    return sg;
}

uint8_t getOutcome(const uint8_t *board, size_t boardLen) {
    uint8_t boardCopy[boardLen];
    for (size_t i = 0; i < boardLen; i++)
        boardCopy[i] = board[i];

    bool blackWin = solver->solveID(boardCopy, boardLen, BLACK) == BLACK;
    assert(memcmp(board, boardCopy, boardLen) == 0);

    bool whiteWin = solver->solveID(boardCopy, boardLen, WHITE) == WHITE;
    assert(memcmp(board, boardCopy, boardLen) == 0);

    if (!blackWin && !whiteWin) // 0 0
        return OC_P;
    if (!blackWin && whiteWin) // 0 1
        return OC_W;
    if (blackWin && !whiteWin) // 1 0
        return OC_B;
    if (blackWin && whiteWin) // 1 1
        return OC_N;

    assert(false);
}

uint8_t getOutcome(const Subgame &sg) {
    return getOutcome(sg.board(), sg.size());
}

uint64_t lookupComplexity(const uint8_t *board, size_t boardLen) {
    uint8_t *entry = db->get(board, boardLen);
    if (entry == 0)
        return 0;
    assert(*db_get_outcome(entry) != 0);
    const uint64_t metric = *db_get_metric(entry);
    assert(metric != uint64_t(-1));
    return metric;
}

uint64_t lookupAltComplexity(const uint8_t *board, size_t boardLen) {
    uint8_t *entry = alt_db->get(board, boardLen);
    if (entry == 0)
        return 0;
    assert(*db_get_outcome(entry) != 0);
    const uint64_t metric = *db_get_metric(entry);
    assert(metric != uint64_t(-1));
    return metric;
}

uint64_t getDominanceImplFor(const uint8_t *board, size_t boardLen, int player, uint64_t &equal) {
    assert(player == BLACK || player == WHITE);

    const size_t g1Size = boardLen;
    uint8_t g1[g1Size];
    uint8_t undo1[UNDO_BUFFER_SIZE];

    const size_t g2Size = boardLen;
    uint8_t g2[g2Size];
    uint8_t undo2[UNDO_BUFFER_SIZE];

    for (size_t i = 0; i < boardLen; i++) {
        g1[i] = board[i];
        g2[i] = board[i];
    }

    auto assertRestore1 = [&]() -> void {
        assert(memcmp(g1, board, boardLen) == 0);
    };

    auto assertRestore2 = [&]() -> void {
        assert(memcmp(g2, board, boardLen) == 0);
    };

    assertRestore1();
    assertRestore2();
    uint64_t mask = 0;

    size_t moveCount;
    unique_ptr<int[]> moves(getMoves(g1, g1Size, player, &moveCount));

    for (size_t i = 0; i < moveCount; i++) {
        assertRestore1();

        //if (getDominated(mask, i))
        //    continue;

        play(g1, undo1, moves[2 * i], moves[2 * i + 1]);

        for (size_t j = i + 1; j < moveCount; j++) {
            assertRestore2();
            //assert(!getDominated(mask, i));

            if (getDominated(mask, i) && getDominated(mask, j))
                continue;

            //if (getDominated(mask, j))
            //    continue;

            play(g2, undo2, moves[2 * j], moves[2 * j + 1]);
            negateBoard(g2, g2Size);

            size_t g3Size;
            uint8_t *g3 = addGames(g1, g1Size, g2, g2Size, &g3Size);

            int bFirst = solver->solveID(g3, g3Size, BLACK);
            int wFirst = solver->solveID(g3, g3Size, WHITE);

            delete[] g3;

            int compare = 0;

            if (bFirst == wFirst)
                compare = (bFirst == BLACK) ? 1 : -1; // Positive or negative

            if (player == WHITE)
                compare = -compare; // Positive or negative for current player

            if (compare == -1) { // I < J (from POV of player)
                setDominated(mask, i);
                //break;
            }
            else if (compare == 1) // I > J (from POV of player)
                setDominated(mask, j);
            else if (bFirst == WHITE && wFirst == BLACK) {
                const uint64_t metric1 = lookupComplexity(g1, g1Size);
                const uint64_t metric2 = lookupComplexity(g2, g2Size);

                const bool pruneFirst = metric1 > metric2;

                if (pruneFirst)
                    setDominated(equal, i);
                else
                    setDominated(equal, j);
            }

            negateBoard(g2, g2Size);
            undo(g2, undo2);
        }

        undo(g1, undo1);
    }

    assertRestore1();
    assertRestore2();

    return mask;
}

DominancePair getDominance(const uint8_t *board, size_t boardLen, DominancePair &equal) {
    DominancePair dp;
    dp.domBlack = getDominanceImplFor(board, boardLen, BLACK, equal.domBlack);
    dp.domWhite = getDominanceImplFor(board, boardLen, WHITE, equal.domWhite);
    return dp;
}

DominancePair getDominance(const Subgame &sg, DominancePair &equal) {
    return getDominance(sg.board(), sg.size(), equal);
}

//////////////////////////////
uint64_t getAltDominanceImplFor(const uint8_t *board, size_t boardLen, int player, uint64_t &equal) {
    assert(player == BLACK || player == WHITE);

    const size_t g1Size = boardLen;
    uint8_t g1[g1Size];
    uint8_t undo1[UNDO_BUFFER_SIZE];

    const size_t g2Size = boardLen;
    uint8_t g2[g2Size];
    uint8_t undo2[UNDO_BUFFER_SIZE];

    for (size_t i = 0; i < boardLen; i++) {
        g1[i] = board[i];
        g2[i] = board[i];
    }

    auto assertRestore1 = [&]() -> void {
        assert(memcmp(g1, board, boardLen) == 0);
    };

    auto assertRestore2 = [&]() -> void {
        assert(memcmp(g2, board, boardLen) == 0);
    };

    assertRestore1();
    assertRestore2();
    uint64_t mask = 0;

    size_t moveCount;
    unique_ptr<int[]> moves(getMoves(g1, g1Size, player, &moveCount));

    for (size_t i = 0; i < moveCount; i++) {
        assertRestore1();

        //if (getDominated(mask, i))
        //    continue;

        play(g1, undo1, moves[2 * i], moves[2 * i + 1]);

        for (size_t j = i + 1; j < moveCount; j++) {
            assertRestore2();
            //assert(!getDominated(mask, i));

            if (getDominated(mask, i) && getDominated(mask, j))
                continue;

            //if (getDominated(mask, j))
            //    continue;

            play(g2, undo2, moves[2 * j], moves[2 * j + 1]);
            negateBoard(g2, g2Size);

            size_t g3Size;
            uint8_t *g3 = addGames(g1, g1Size, g2, g2Size, &g3Size);

            int bFirst = solver->solveID(g3, g3Size, BLACK);
            int wFirst = solver->solveID(g3, g3Size, WHITE);

            delete[] g3;

            int compare = 0;

            if (bFirst == wFirst)
                compare = (bFirst == BLACK) ? 1 : -1; // Positive or negative

            if (player == WHITE)
                compare = -compare; // Positive or negative for current player

            if (compare == -1) { // I < J (from POV of player)
                setDominated(mask, i);
                //break;
            }
            else if (compare == 1) // I > J (from POV of player)
                setDominated(mask, j);
            else if (bFirst == WHITE && wFirst == BLACK) {
                const uint64_t metric1 = lookupAltComplexity(g1, g1Size);
                const uint64_t metric2 = lookupAltComplexity(g2, g2Size);

                const bool pruneFirst = metric1 > metric2;

                if (pruneFirst)
                    setDominated(equal, i);
                else
                    setDominated(equal, j);
            }

            negateBoard(g2, g2Size);
            undo(g2, undo2);
        }

        undo(g1, undo1);
    }

    assertRestore1();
    assertRestore2();

    return mask;
}

DominancePair getAltDominance(const uint8_t *board, size_t boardLen, DominancePair &equal) {
    DominancePair dp;
    dp.domBlack = getAltDominanceImplFor(board, boardLen, BLACK, equal.domBlack);
    dp.domWhite = getAltDominanceImplFor(board, boardLen, WHITE, equal.domWhite);
    return dp;
}

DominancePair getAltDominance(const Subgame &sg, DominancePair &equal) {
    return getAltDominance(sg.board(), sg.size(), equal);
}
//////////////////////////////

inline uint64_t getChildMetric(const Subgame &sg, Database *target_db) {
    uint8_t *entry = target_db->get(sg);
    if (entry == 0)
        return 0;
    assert(*db_get_outcome(entry) != 0 && *db_get_metric(entry) != uint64_t(-1));
    return *db_get_metric(entry);
}


uint64_t countMovesFor(const Subgame &sg, int player) {
    size_t moveCount = 0;

    int *moves = getMoves(sg.board(), sg.size(), player, &moveCount);
    delete[] moves;

    return moveCount;
}

uint64_t countMoves(const Subgame &sg) {
    const uint64_t bMoves = countMovesFor(sg, BLACK);
    const uint64_t wMoves = countMovesFor(sg, WHITE);

    return bMoves + wMoves;
}

void getSimplest(const Subgame &sg, uint64_t domBlack, uint64_t domWhite,
                 uint8_t &simplestBlack, uint8_t &simplestWhite, Database *target_db)
{
    vector<optional<Subgame*>> childrenBlack = sg.getChildren(BLACK, domBlack);
    vector<optional<Subgame*>> childrenWhite = sg.getChildren(WHITE, domWhite);

    uint64_t bestBlack = uint64_t(-1);
    uint64_t bestWhite = uint64_t(-1);

    const size_t childrenBlackSize = childrenBlack.size();
    for (size_t i = 0; i < childrenBlackSize; i++) {
        optional<Subgame*> &sg_opt = childrenBlack[i];

        if (!sg_opt.has_value())
            continue;

        Subgame *sg = sg_opt.value();
        const uint64_t childMetric = getChildMetric(*sg, target_db);

        if (childMetric < bestBlack) {
            bestBlack = childMetric;
            simplestBlack = i;
        }

        delete sg;
    }

    const size_t childrenWhiteSize = childrenWhite.size();
    for (size_t i = 0; i < childrenWhiteSize; i++) {
        optional<Subgame*> &sg_opt = childrenWhite[i];

        if (!sg_opt.has_value())
            continue;

        Subgame *sg = sg_opt.value();
        const uint64_t childMetric = getChildMetric(*sg, target_db);

        if (childMetric < bestWhite) {
            bestWhite = childMetric;
            simplestWhite = i;
        }

        delete sg;
    }
}

uint64_t getMetric(const Subgame &sg, bool shallow, Database *target_db) {
    uint64_t metric = 0;

    uint8_t *entry = target_db->get(sg);
    assert(entry != 0);
    assert(*db_get_outcome(entry) != 0);

    const uint64_t domBlack = db_get_dominance(entry)[0];
    const uint64_t domWhite = db_get_dominance(entry)[1];

    {
        size_t moveCount;
        int *movesBlack = getMoves(sg.board(), sg.size(), BLACK, &moveCount);
        metric += moveCount;
        delete[] movesBlack;

        int *movesWhite = getMoves(sg.board(), sg.size(), WHITE, &moveCount);
        metric += moveCount;
        delete[] movesWhite;

        uint64_t nDominatedMoves = 0;
        nDominatedMoves += sumBits(domBlack);
        nDominatedMoves += sumBits(domWhite);
        assert(nDominatedMoves <= metric);
        metric -= nDominatedMoves;
    }

    if (shallow)
        return metric;

    vector<optional<Subgame*>> childrenBlack = sg.getChildren(BLACK, domBlack);
    vector<optional<Subgame*>> childrenWhite = sg.getChildren(WHITE, domWhite);

    const size_t childrenBlackSize = childrenBlack.size();
    for (size_t i = 0; i < childrenBlackSize; i++) {
        optional<Subgame*> &sg_opt = childrenBlack[i];

        if (!sg_opt.has_value())
            continue;

        Subgame *sg = sg_opt.value();
        const uint64_t childMetric = getChildMetric(*sg, target_db);
        metric += childMetric;

        delete sg;
    }

    const size_t childrenWhiteSize = childrenWhite.size();
    for (size_t i = 0; i < childrenWhiteSize; i++) {
        optional<Subgame*> &sg_opt = childrenWhite[i];

        if (!sg_opt.has_value())
            continue;

        Subgame *sg = sg_opt.value();
        const uint64_t childMetric = getChildMetric(*sg, target_db);
        metric += childMetric;

        delete sg;
    }

    return metric;
}

uint8_t getOutcomeOnScale(const Subgame &sg, int8_t scaleIdx, int scale) {
    Subgame sum;
    vector<uint8_t> &boardVec = sum.boardVec();

    Subgame *inverseScaleGame = nullptr;
    if (scale == 0)
        inverseScaleGame = getInverseScaleGame(scaleIdx);
    else if (scale == 1)
        inverseScaleGame = getInverseScaleGameNoStar(scaleIdx);
    else if (scale == 2)
        inverseScaleGame = getInverseScaleGameWithStar(scaleIdx);
    else
        assert(false);

    boardVec = sg.boardVecConst();
    boardVec.reserve(boardVec.size() + 1 + inverseScaleGame->size());

    boardVec.push_back(EMPTY);

    for (size_t i = 0; i < inverseScaleGame->size(); i++)
        boardVec.push_back((*inverseScaleGame)[i]);

    delete inverseScaleGame;

    return getOutcome(sum);
}

inline relation outcomeToRelation(uint8_t outcome) {
    if (outcome == OC_P)
        return REL_EQUAL;
    if (outcome == OC_B)
        return REL_GREATER;
    if (outcome == OC_W)
        return REL_LESS;
    if (outcome == OC_N)
        return REL_FUZZY;

    assert(false);
}

relation getRelationOnScale(const Subgame &sg, int8_t scaleIdx, int scale) {
    uint8_t outcome = getOutcomeOnScale(sg, scaleIdx, scale);
    return outcomeToRelation(outcome);
}

BoundsPair getBounds(const Subgame &game, int scale) {
    BoundsPair bp;

    constexpr int8_t radius = 32;

    bool haveLow = false;
    bool haveHigh = false;
    int8_t low = numeric_limits<int8_t>::min();
    int8_t high = numeric_limits<int8_t>::max();

    auto setRelation = [&](int8_t scaleIdx, relation rel) -> bool {
        assert(rel != REL_UNKNOWN);

        if (rel == REL_LESS) {
            haveHigh = true;
            high = min(high, scaleIdx);
        } else if (rel == REL_GREATER) {
            haveLow = true;
            low = max(low, scaleIdx);
        } else if (rel == REL_EQUAL) {
            haveLow = true;
            haveHigh = true;
            low = scaleIdx;
            high = scaleIdx;
        }

        return haveLow && haveHigh;
    };


    for (int8_t magnitude = 0; magnitude < radius; magnitude++) {
        const int8_t idxPos = magnitude;
        const relation relPos = getRelationOnScale(game, idxPos, scale);
        if (setRelation(idxPos, relPos))
            break;

        if (magnitude == 0)
            continue;

        const int8_t idxNeg = -magnitude;
        const relation relNeg = getRelationOnScale(game, idxNeg, scale);
        if (setRelation(idxNeg, relNeg))
            break;
    }

    assert(haveLow && haveHigh);
    assert(low <= high);

    bp.low = low;
    bp.high = high;

    return bp;
}

//void addToReplacementMap(const Subgame &game, const ReplacementMapIdx &rmapIdx) {

void addToReplacementMap(const Subgame &game, const ReplacementMapIdx &rmapIdx,
        rmap_t &targetMap, Database *target_db) {

    vector<shared_ptr<IndirectLink>> &vec = targetMap[rmapIdx];

    uint8_t *entry = target_db->get(game);
    assert(entry != 0);
    assert(*db_get_outcome(entry) != 0 && *db_get_metric(entry) != uint64_t(-1));

    Subgame sgCopy = game;

    size_t foundIdx = 0;
    bool found = false;

    const size_t N = vec.size();
    for (size_t i = 0; i < N; i++) {
        shared_ptr<IndirectLink> &indirect = vec[i];

        uint8_t *linkedEntry = target_db->getFromIndirectIdx(*indirect);
        assert(linkedEntry != 0);
        uint64_t linkedShape = *db_get_shape(linkedEntry);
        uint32_t linkedNumber = *db_get_number(linkedEntry);

        vector<Subgame*> subgames = makeGameNew(linkedShape, linkedNumber);

        for (Subgame *sg : subgames)
            sg->negate();

        subgames.push_back(&sgCopy);
        Subgame *sum = Subgame::concatSubgames(subgames);
        subgames.pop_back();

        for (Subgame *sg : subgames)
            delete sg;

        relation rel = outcomeToRelation(getOutcome(*sum));
        delete sum;

        if (rel == REL_EQUAL) {
            foundIdx = i;
            found = true;
            break;
        }
    }

    bool allowInsert = game.size() <= RMAP_SIZE;

    if (!found) {
        if (allowInsert) {
            uint64_t directLink = target_db->getIdxDirect(game);
            vec.emplace_back(new IndirectLink(directLink));
        }
    } else {
        const uint64_t metric = *db_get_metric(entry);

        IndirectLink &indirectLink = *(vec[foundIdx]);
        uint8_t *foundEntry = target_db->getFromIndirectIdx(indirectLink);
        assert(foundEntry != 0 && *db_get_outcome(entry) != 0);

        const uint64_t foundMetric = *db_get_metric(foundEntry);
        assert(foundMetric != uint64_t(-1));

        if (metric < foundMetric) { // New game is better
            if (allowInsert)
                indirectLink.directLink = target_db->getIdxDirect(game);
        } else if (metric > foundMetric) { // Existing game is better
            //*db_get_link(entry) = (uint64_t) &indirectLink;
            ((IndirectLink*) *db_get_link(entry))->directLink = indirectLink.directLink;
        }
    }
}

////////////////////////////////////////////////// main pass functions

// Initialize all entries to uninitialized values
void pass_initializeAllEntries(Database *target_db) {
    cout << "Initializing entries" << endl;

    GameGenerator gen;

    size_t entryNumber = 0;

    while (gen) {
        assert(entryNumber < target_db->entryCount);

        GeneratedGame genGame = gen.generate();
        ++gen;
        
        uint8_t *entry = target_db->get(*genGame.game);
        assert(entry != 0);

        *db_get_outcome(entry) = 0;
        db_get_dominance(entry)[0] = 0;
        db_get_dominance(entry)[1] = 0;
        db_get_bounds(entry)[0] = numeric_limits<int8_t>::max();
        db_get_bounds(entry)[1] = numeric_limits<int8_t>::min();
        db_get_bounds(entry)[2] = numeric_limits<int8_t>::max();
        db_get_bounds(entry)[3] = numeric_limits<int8_t>::min();
        *db_get_metric(entry) = uint64_t(-1);
        *db_get_shape(entry) = genGame.shapeNumber;
        *db_get_number(entry) = genGame.gameNumber;
        db_get_simplest_moves(entry)[0] = uint8_t(-1);
        db_get_simplest_moves(entry)[1] = uint8_t(-1);

        uint64_t directLink = target_db->getIdxDirect(*genGame.game);

        {
            IndirectLink &indirect = target_db->getDefaultIndirectLink1(entryNumber);
            indirect.directLink = directLink;
            uint64_t indirectLink = (uint64_t) &indirect;
            *db_get_link(entry) = indirectLink;

            assert(target_db->getFromIdx(indirectLink) == entry);
        }

        entryNumber++;
    }
    assert(entryNumber == target_db->entryCount);

}

// Outcome classes for normal games
void pass_normalGameOutcomes() {
    cout << "Finding outcome classes for normalized games" << endl;

    GameGenerator gen;

    while (gen) {
        GeneratedGame genGame = gen.generate();
        ++gen;

        unique_ptr<Subgame> normal(genGame.game->getNormalizedGame());
        if (normal->size() == 0)
            continue;

        uint8_t *entry = db->get(*normal);
        assert(entry != 0);
        if (*db_get_outcome(entry) != 0)
            continue;

        *db_get_outcome(entry) = getOutcome(*normal);
    }
}

void pass_normalGameBounds() {
    cout << "Finding bounds for normal games" << endl;

    GameGenerator gen;

    unordered_set<uint64_t> hashes;

    while (gen) {
        GeneratedGame genGame = gen.generate();
        ++gen;

        unique_ptr<Subgame> normal(genGame.game->getNormalizedGame());

        if (normal->size() == 0)
            continue;

        const uint64_t normalHash = normal->getHash();

        {
            auto it = hashes.insert(normalHash);
            if (!it.second)
                continue;
        }

        uint8_t *normalEntry = db->get(*normal);
        assert(normalEntry != 0);

        assert(*db_get_outcome(normalEntry) != 0);
        assert(db_get_bounds(normalEntry)[0] == numeric_limits<int8_t>::max());
        assert(db_get_bounds(normalEntry)[1] == numeric_limits<int8_t>::min());
        assert(db_get_bounds(normalEntry)[2] == numeric_limits<int8_t>::max());
        assert(db_get_bounds(normalEntry)[3] == numeric_limits<int8_t>::min());

        BoundsPair bp1 = getBounds(*normal, 1);
        db_get_bounds(normalEntry)[0] = bp1.low;
        db_get_bounds(normalEntry)[1] = bp1.high;
        BoundsPair bp2 = getBounds(*normal, 2);
        db_get_bounds(normalEntry)[2] = bp2.low;
        db_get_bounds(normalEntry)[3] = bp2.high;
    }
}

void pass_mainNoNormal() {
    cout << "Main pass no normal" << endl;
    GameGenerator gen;

    while (gen) {
        GeneratedGame genGame = gen.generate();
        ++gen;

        uint8_t *entry = db->get(*genGame.game);
        assert(entry != 0);

        bool hasOutcome = *db_get_outcome(entry) != 0;
        if (!hasOutcome) {
            int outcome = getOutcome(*genGame.game);
            *db_get_outcome(entry) = outcome;
        }

        assert(*db_get_outcome(entry) != 0);

        bool hasBounds = true;
        hasBounds &= db_get_bounds(entry)[0] != numeric_limits<int8_t>::max();
        hasBounds &= db_get_bounds(entry)[1] != numeric_limits<int8_t>::min();
        hasBounds &= db_get_bounds(entry)[2] != numeric_limits<int8_t>::max();
        hasBounds &= db_get_bounds(entry)[3] != numeric_limits<int8_t>::min();

        cout << *genGame.game << " " << genGame.shape << " " << std::flush;

        // Bounds on scale 1
        BoundsPair bp1;
        BoundsPair bp2;
        if (hasBounds) {
            bp1.low = db_get_bounds(entry)[0];
            bp1.high = db_get_bounds(entry)[1];
            bp2.low = db_get_bounds(entry)[2];
            bp2.high = db_get_bounds(entry)[3];
        } else {
            bp1 = getBounds(*genGame.game, 1);
            db_get_bounds(entry)[0] = bp1.low;
            db_get_bounds(entry)[1] = bp1.high;

            bp2 = getBounds(*genGame.game, 2);
            db_get_bounds(entry)[2] = bp2.low;
            db_get_bounds(entry)[3] = bp2.high;
        }

        // Find dominated moves
        DominancePair equal;
        equal.domBlack = 0;
        equal.domWhite = 0;

        DominancePair dp = getDominance(*genGame.game, equal);
        db_get_dominance(entry)[0] = dp.domBlack;
        db_get_dominance(entry)[1] = dp.domWhite;

        // Find metric
        uint64_t metric = getMetric(*genGame.game, false, db);
        *db_get_metric(entry) = metric;

        // Update pruned with "equal"
        dp.domBlack |= equal.domBlack;
        dp.domWhite |= equal.domWhite;
        db_get_dominance(entry)[0] = dp.domBlack;
        db_get_dominance(entry)[1] = dp.domWhite;
          
        // Find "simplest" moves
        uint8_t simplestBlack = uint8_t(-1);
        uint8_t simplestWhite = uint8_t(-1);

        getSimplest(*genGame.game, dp.domBlack, dp.domWhite, simplestBlack,
                    simplestWhite, db);
        
        db_get_simplest_moves(entry)[0] = simplestBlack;
        db_get_simplest_moves(entry)[1] = simplestWhite;

        // Index for replacement map
        assert(bp1.low <= bp1.high);
        assert(bp2.low <= bp2.high);

        cout << "<" << (int) bp1.low << " " << (int) bp1.high << " | ";
        cout << (int) bp2.low << " " << (int) bp2.high << ">" << endl;

        // Find link
        ReplacementMapIdx idx;
        idx.outcome = *db_get_outcome(entry);
        idx.low1 = bp1.low;
        idx.high1 = bp1.high;
        idx.low2 = bp2.low;
        idx.high2 = bp2.high;

        //if (equalsProblemCase(*normal)) {
        //    assert(!Solver::doDebug);
        //    cout << "ENABLING DEBUG" << endl;
        //    Solver::doDebug = true;
        //}

        addToReplacementMap(*genGame.game, idx, replacementMap, db);
    }
}

void pass_altDatabase() {
    cout << "Alt DB pass" << endl;
    GameGenerator gen;

    while (gen) {
        GeneratedGame genGame = gen.generate();
        ++gen;

        uint8_t *entry = db->get(*genGame.game);
        uint8_t *alt_entry = alt_db->get(*genGame.game);
        assert(entry != 0 && alt_entry != 0);

        // Copy trivial fields

        // Outcome
        *db_get_outcome(alt_entry) = *db_get_outcome(entry);

        // Bounds
        db_get_bounds(alt_entry)[0] = db_get_bounds(entry)[0];
        db_get_bounds(alt_entry)[1] = db_get_bounds(entry)[1];
        db_get_bounds(alt_entry)[2] = db_get_bounds(entry)[2];
        db_get_bounds(alt_entry)[3] = db_get_bounds(entry)[3];

        BoundsPair bp1;
        BoundsPair bp2;

        bp1.low = db_get_bounds(alt_entry)[0];
        bp1.high = db_get_bounds(alt_entry)[1];
        bp2.low = db_get_bounds(alt_entry)[2];
        bp2.high = db_get_bounds(alt_entry)[3];

        // Shape
        *db_get_shape(alt_entry) = *db_get_shape(entry);

        // Number
        *db_get_number(alt_entry) = *db_get_number(entry);

        // Compute new info

        // Metric
        //uint64_t totalMoves = countMoves(*genGame.game);
        //*db_get_metric(alt_entry) = totalMoves;


        // Dominance
        DominancePair equal;
        equal.domBlack = 0;
        equal.domWhite = 0;

        DominancePair dp = getAltDominance(*genGame.game, equal);
        db_get_dominance(alt_entry)[0] = dp.domBlack;
        db_get_dominance(alt_entry)[1] = dp.domWhite;

        // New metric
        uint64_t metric = getMetric(*genGame.game, true, alt_db);
        *db_get_metric(alt_entry) = metric;

        // Update equal
        dp.domBlack |= equal.domBlack;
        dp.domWhite |= equal.domWhite;
        db_get_dominance(alt_entry)[0] = dp.domBlack;
        db_get_dominance(alt_entry)[1] = dp.domWhite;

        // Simplest moves
        uint8_t simplestBlack = uint8_t(-1);
        uint8_t simplestWhite = uint8_t(-1);

        getSimplest(*genGame.game, dp.domBlack, dp.domWhite, simplestBlack,
                    simplestWhite, alt_db);

        db_get_simplest_moves(alt_entry)[0] = simplestBlack;
        db_get_simplest_moves(alt_entry)[1] = simplestWhite;


        // Links
        assert(bp1.low <= bp1.high);
        assert(bp2.low <= bp2.high);

        cout << "COPY: <" << (int) bp1.low << " " << (int) bp1.high << " | ";
        cout << (int) bp2.low << " " << (int) bp2.high << ">" << endl;

        ReplacementMapIdx idx;
        idx.outcome = *db_get_outcome(alt_entry);
        idx.low1 = bp1.low;
        idx.high1 = bp1.high;
        idx.low2 = bp2.low;
        idx.high2 = bp2.high;

        addToReplacementMap(*genGame.game, idx, altReplacementMap, alt_db);
    }

}


} // namespace



#define TIME(x) {t.start(); x; cout << ((double) t.stop()) / 1000.0 << "s" << endl;} \
static_assert(true)

int main() {
    db = new Database();
    db->init();
    db->enableIndirectLinks();

    pass_initializeAllEntries(db);

    solver = new Solver(DB_MAX_BITS, db);

    Timer t;

    TIME(pass_normalGameOutcomes());
    TIME(pass_normalGameBounds());
    TIME(pass_mainNoNormal());

    alt_db = new Database();
    alt_db->init();
    alt_db->enableIndirectLinks();
    pass_initializeAllEntries(alt_db);


    TIME(pass_altDatabase());

    db->finalizeIndirectLinks();
    db->save("database3.bin");

    alt_db->finalizeIndirectLinks();
    alt_db->save("database3_alt.bin");


    delete solver;
    delete db;
    delete alt_db;
}
