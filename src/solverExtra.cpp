#include "solverExtra.h"
#include "solver.h"
#include "database3.h"
#include "options.h"
#include "utils.h"
#include "state.h"
#include "miscTypes.h"
#include <cstring>
#include <iostream>
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <set>

using namespace std;

namespace {
namespace dbUtil {

bool simplifySubgamesSmallest(vector<Subgame*> &subgames, const vector<size_t> &indices, Database *db) {
    if (indices.empty())
        return false;

    uint8_t *entry = 0;
    unique_ptr<Subgame> sgSum;

    if (indices.size() == 1) {
        const Subgame *sg = subgames[indices.back()];
        assert(sg != nullptr);
        entry = db->get(sg->board(), sg->size());
    } else {
        vector<Subgame*> sumVec;
        sumVec.reserve(indices.size());

        for (const size_t &idx : indices) {
            Subgame *sg = subgames[idx];
            assert(sg != nullptr);
            sumVec.push_back(sg);
        }

        //Subgame *sgSum = Subgame::concatSubgames(sumVec);
        sgSum.reset(Subgame::concatSubgames(sumVec));
        entry = db->get(sgSum->board(), sgSum->size());
        //delete sgSum;
    }


    if (!entryValid(entry))
        return false;

    const bool isP = *db_get_outcome(entry) == OC_P;

    const Subgame *queriedGame;
    if (sgSum.get() != nullptr)
        queriedGame = sgSum.get();
    else {
        assert(indices.size() == 1);
        queriedGame = subgames[indices.back()];
    }

    if (isP || tryInflateLinkSmallest(*queriedGame, entry, db, subgames)) {
        for (const size_t &idx : indices) {
            Subgame *sg = subgames[idx];
            assert(sg != nullptr);
            delete sg;
            subgames[idx] = nullptr;
        }

        return true;
    }


    return false;
}

inline void simplify1Smallest(vector<Subgame*> &subgames, Database *db) {
    vector<size_t> indices;
    indices.reserve(1);

    for (size_t i = 0; i < subgames.size(); i++) {
        if (subgames[i] == nullptr)
            continue;

        indices.clear();
        indices.push_back(i);

        simplifySubgamesSmallest(subgames, indices, db);
    }
}

inline void simplify2Smallest(vector<Subgame*> &subgames, Database *db) {
    vector<size_t> indices;
    indices.reserve(2);

    for (size_t i = 0; i < subgames.size(); i++) {
        if (subgames[i] == nullptr)
            continue;
        for (size_t j = i + 1; j < subgames.size(); j++) {
            assert(subgames[i] != nullptr);

            if (subgames[j] == nullptr)
                continue;

            indices.clear();
            indices.push_back(i);
            indices.push_back(j);

            if (simplifySubgamesSmallest(subgames, indices, db))
                break;
        }
    }
}


inline void simplify3Smallest(vector<Subgame*> &subgames, Database *db) {
    auto sortFn = [](const Subgame *sg1, const Subgame *sg2) -> bool {
        if (sg1 == nullptr)
            return false;
        if (sg2 == nullptr)
            return true;

        return sg1->size() < sg2->size();
    };

    std::sort(subgames.begin(), subgames.end(), sortFn);

    vector<size_t> window;
    size_t windowSize = 0;

    auto tryReplaceStep = [&]() -> bool {
        assert(windowSize <= DB_MAX_BITS);

        const size_t nSubgames = window.size();

        if (nSubgames < 3)
            return false;

        const bool simplified = simplifySubgamesSmallest(subgames, window, db);

        if (!simplified)
            return false;

        window.clear();
        windowSize = 0;

        return true;
    };

    auto tryReplace = [&]() -> void {
        if (window.size() < 3)
            return;

        vector<size_t> reversed;
        reversed.reserve(window.size());

        for (auto it = window.rbegin(); it != window.rend(); it++)
            reversed.push_back(*it);

        window = std::move(reversed);

        while (window.size() >= 3 && !tryReplaceStep()) {
            windowSize -= (1 + subgames[window.back()]->size());
            window.pop_back();
        }

        window.clear();
        windowSize = 0;
    };

    for (size_t i = 0; i < subgames.size(); i++) {
        Subgame *sg = subgames[i];

        if (sg == nullptr)
            continue;

        if (sg->size() > DB_MAX_BITS)
            continue;

        const size_t contribution = sg->size() + (window.size() > 0);

        if (contribution + windowSize <= DB_MAX_BITS) {
            window.push_back(i);
            windowSize += contribution;
        } else {
            tryReplace();
            window.clear();

            window.push_back(i);
            windowSize = sg->size();
        }

    }
    tryReplace();
}


} // namespace dbUtil
} // namespace

void Solver::simplifyNewSmallest(uint8_t **board, size_t *boardLen) {
    vector<Subgame*> initialSubgames = generateSubgamesNew(*board, *boardLen);

    vector<Subgame*> replacements;
    vector<Subgame*> remainders;

    size_t initialZeroes = 0;

    // Filter out 0s, and games with no entries
    for (Subgame *sg : initialSubgames) {
        uint8_t *entry = 0;

        // No entry
        if (sg->size() <= DB_MAX_BITS)
            entry = db->get(sg->board(), sg->size());

        if (!entryValid(entry)) {
            replacements.push_back(sg);
            continue;
        }

        // Check for zero
        const bool isP = *db_get_outcome(entry) == OC_P;

        if (isP) {
            delete sg;
            initialZeroes++;
            continue;
        }

        remainders.push_back(sg);
    }

    assert(initialSubgames.size() ==
           initialZeroes + replacements.size() + remainders.size());


    // Do replacements
    dbUtil::simplify3Smallest(remainders, db);
    dbUtil::simplify2Smallest(remainders, db);
    dbUtil::simplify1Smallest(remainders, db);

    // Combine games
    vector<Subgame*> allGames;
    allGames.reserve(remainders.size() + replacements.size());
    for (Subgame *sg : remainders)
        allGames.push_back(sg);
    for (Subgame *sg : replacements)
        allGames.push_back(sg);

    // Prune G + -G
    pruneInversePairs(allGames);
    pruneNoMoveGames(allGames);

    // Mirror games
    for (Subgame *sg : allGames)
        if (sg != nullptr)
            sg->tryMirror();

    // Sort games
    std::sort(allGames.begin(), allGames.end(), Subgame::normalizeSortOrder);

    // Remove nulls
    while (!allGames.empty() && allGames.back() == nullptr)
        allGames.pop_back();

    // Stitch games back together
    //printBoard(*board, *boardLen);
    //cout << endl;

    stitchGames(allGames, board, boardLen);

    //printBoard(*board, *boardLen);
    //cout << endl;

    //cout << endl;

    // TODO: is this stitching optimal? Are there 0 move games still?
}

