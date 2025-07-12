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
#include <map>
#include <unordered_set>
#include <unordered_map>

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
Solver *solver;
unordered_map<ReplacementMapIdx, vector<shared_ptr<IndirectLink>>> replacementMap;

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

uint64_t getDominanceImplFor(const uint8_t *board, size_t boardLen, int player) {
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

        if (getDominated(mask, i))
            continue;

        play(g1, undo1, moves[2 * i], moves[2 * i + 1]);

        for (size_t j = i + 1; j < moveCount; j++) {
            assertRestore2();
            assert(!getDominated(mask, i));

            if (getDominated(mask, j))
                continue;

            play(g2, undo2, moves[2 * j], moves[2 * j + 1]);
            negateBoard(g2, g2Size);

            size_t g3Size;
            uint8_t *g3 = addGames(g1, g1Size, g2, g2Size, &g3Size);

            int bFirst = solver->solveID(g3, g3Size, BLACK);
            int wFirst = solver->solveID(g3, g3Size, WHITE);

            delete[] g3;

            negateBoard(g2, g2Size);
            undo(g2, undo2);

            int compare = 0;

            if (bFirst == wFirst)
                compare = (bFirst == BLACK) ? 1 : -1; // Positive or negative

            if (player == WHITE)
                compare = -compare; // Positive or negative for current player

            if (compare == -1) { // I < J (from POV of player)
                setDominated(mask, i);
                break;
            }
            else if (compare == 1) // I > J (from POV of player)
                setDominated(mask, j);
        }

        undo(g1, undo1);
    }

    assertRestore1();
    assertRestore2();

    return mask;
}

DominancePair getDominance(const uint8_t *board, size_t boardLen) {
    DominancePair dp;
    dp.domBlack = getDominanceImplFor(board, boardLen, BLACK);
    dp.domWhite = getDominanceImplFor(board, boardLen, WHITE);
    return dp;
}

DominancePair getDominance(const Subgame &sg) {
    return getDominance(sg.board(), sg.size());
}

inline uint64_t getChildMetric(const Subgame &sg) {
    uint8_t *entry = db->get(sg);
    assert(entry != 0);
    assert(*db_get_outcome(entry) != 0 && *db_get_metric(entry) != uint64_t(-1));
    return *db_get_metric(entry);
}

uint64_t getMetric(const Subgame &sg) {
    uint64_t metric = 0;

    uint8_t *entry = db->get(sg);
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

    vector<Subgame*> normalChildrenBlack = sg.getNormalizedChildren(BLACK, domBlack);
    vector<Subgame*> normalChildrenWhite = sg.getNormalizedChildren(WHITE, domWhite);

    for (Subgame *sg : normalChildrenBlack) {
        metric += getChildMetric(*sg);
        delete sg;
    }

    for (Subgame *sg : normalChildrenWhite) {
        metric += getChildMetric(*sg);
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

void addToReplacementMap(const Subgame &game, const ReplacementMapIdx &rmapIdx) {
    vector<shared_ptr<IndirectLink>> &vec = replacementMap[rmapIdx];

    uint8_t *entry = db->get(game);
    assert(entry != 0);
    assert(*db_get_outcome(entry) != 0 && *db_get_metric(entry) != uint64_t(-1));

    Subgame sgCopy = game;

    size_t foundIdx = 0;
    bool found = false;

    const size_t N = vec.size();
    for (size_t i = 0; i < N; i++) {
        shared_ptr<IndirectLink> &indirect = vec[i];

        uint8_t *linkedEntry = db->getFromIndirectIdx(*indirect);
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
        }
    }

    if (!found) {
        uint64_t directLink = db->getIdxDirect(game);
        vec.emplace_back(new IndirectLink(directLink));
    } else {
        const uint64_t metric = *db_get_metric(entry);

        IndirectLink &indirectLink = *(vec[foundIdx]);
        uint8_t *foundEntry = db->getFromIndirectIdx(indirectLink);
        assert(foundEntry != 0 && *db_get_outcome(entry) != 0);

        const uint64_t foundMetric = *db_get_metric(foundEntry);
        assert(foundMetric != uint64_t(-1));

        if (metric < foundMetric) { // New game is better
            indirectLink.directLink = db->getIdxDirect(game);
        } else if (metric > foundMetric) { // Existing game is better
            *db_get_link(entry) = (uint64_t) &indirectLink;
        }
    }
}


////////////////////////////////////////////////// main pass functions

// Initialize all entries to uninitialized values
void pass_initializeAllEntries() {
    cout << "Initializing all entries" << endl;

    GameGenerator gen;

    size_t entryNumber = 0;

    while (gen) {
        assert(entryNumber < db->entryCount);

        GeneratedGame genGame = gen.generate();
        ++gen;

        uint64_t directLink = db->getIdxDirect(*genGame.game);

        IndirectLink &indirect = db->getDefaultIndirectLink(entryNumber);
        indirect.directLink = directLink;

        uint64_t indirectLink = (uint64_t) &indirect;

        uint8_t *entry = db->getFromIdx(indirectLink);
        assert(entry != 0);
        assert(entry == db->get(*genGame.game));

        *db_get_outcome(entry) = 0;
        db_get_dominance(entry)[0] = 0;
        db_get_dominance(entry)[1] = 0;
        db_get_bounds(entry)[0] = numeric_limits<int8_t>::max();
        db_get_bounds(entry)[1] = numeric_limits<int8_t>::min();
        *db_get_metric(entry) = uint64_t(-1);
        *db_get_link(entry) = (uint64_t) &indirect;
        *db_get_shape(entry) = genGame.shapeNumber;
        *db_get_number(entry) = genGame.gameNumber;

        entryNumber++;
    }
    assert(entryNumber == db->entryCount);

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

void pass_normalGameDominance() {
    cout << "Finding dominated moves for normalized games" << endl;

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

        DominancePair dp = getDominance(*normal);
        db_get_dominance(normalEntry)[0] = dp.domBlack;
        db_get_dominance(normalEntry)[1] = dp.domWhite;

    }
}

void pass_normalGameMetric() {
    cout << "Finding metrics for normal games" << endl;
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
        assert(*db_get_metric(normalEntry) == uint64_t(-1));


        const uint64_t metric = getMetric(*normal);
        *db_get_metric(normalEntry) = metric;
    }
}

void pass_normalGameLinks() {
    //WWBBW.WWB
    static const vector<uint8_t> problemCase = {2,2,1,1,2,0,2,2,1};

    auto equalsProblemCase = [&](const Subgame &game) -> bool {
        if (problemCase.size() != game.size())
            return false;

        for (size_t i = 0; i < problemCase.size(); i++)
            if (problemCase[i] != game[i])
                return false;

        return true;
    };

    cout << "Finding links for normal games" << endl;
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

        cout << *normal << " " << std::flush;

        BoundsPair bp1 = getBounds(*normal, 1);
        assert(bp1.low <= bp1.high);
        BoundsPair bp2 = getBounds(*normal, 2);
        assert(bp2.low <= bp2.high);

        db_get_bounds(normalEntry)[0] = bp1.low;
        db_get_bounds(normalEntry)[1] = bp1.high;

        cout << "<" << (int) bp1.low << " " << (int) bp1.high << " | ";
        cout << (int) bp2.low << " " << (int) bp2.high << ">" << endl;

        ReplacementMapIdx idx;
        idx.outcome = *db_get_outcome(normalEntry);
        idx.low1 = bp1.low;
        idx.high1 = bp1.high;
        idx.low2 = bp2.low;
        idx.high2 = bp2.high;

        //if (equalsProblemCase(*normal)) {
        //    assert(!Solver::doDebug);
        //    cout << "ENABLING DEBUG" << endl;
        //    Solver::doDebug = true;
        //}

        addToReplacementMap(*normal, idx);
    }
}



} // namespace

int main() {
    db = new Database();
    db->init();

    db->enableIndirectLinks();

    pass_initializeAllEntries();

    solver = new Solver(DB_MAX_BITS, db);

    pass_normalGameOutcomes();
    pass_normalGameDominance();
    pass_normalGameMetric();
    pass_normalGameLinks();

    db->finalizeIndirectLinks();

    db->save();
    delete solver;
    delete db;
}
