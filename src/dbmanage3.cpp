#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <map>

#include "solver.h"
#include "utils.h"
#include "state.h"
#include "dbutils.h"

using namespace std;

Database *db;
Solver *solver;
map<triple<int, int, int>, vector<uint64_t> *> replacementMap;

void computeDominance(uint8_t *g1, size_t g1Size, uint64_t *dominance) {
    uint64_t domBlack = 0;
    uint64_t domWhite = 0;

    uint8_t g2[g1Size];
    size_t g2Size = g1Size;
    memcpy(g2, g1, g1Size);

    uint8_t undo1[UNDO_BUFFER_SIZE];
    uint8_t undo2[UNDO_BUFFER_SIZE];

    //Compute dominance for black
    {
        size_t moveCount;
        int *moves = getMoves(g1, g1Size, 1, &moveCount);

        for (size_t i = 0; i < moveCount; i++) {
            play(g1, undo1, moves[2 * i], moves[2 * i + 1]);
            for (size_t j = i + 1; j < moveCount; j++) {
                play(g2, undo2, moves[2 * j], moves[2 * j + 1]);
                negateBoard(g2, g2Size);

                size_t g3Size;
                uint8_t *g3 = addGames(g1, g1Size, g2, g2Size, &g3Size); 
                int bFirst = solver->solveID(g3, g3Size, 1);
                int wFirst = solver->solveID(g3, g3Size, 2);
                delete[] g3;


                if (bFirst == wFirst) {
                    if (bFirst == 1) { // I - J is positive for black --> I > J
                        domBlack |= (((uint64_t) 1) << j);
                    } else { // I - J is negative for black --> I < J
                        domBlack |= (((uint64_t) 1) << i);
                    }
                }


                negateBoard(g2, g2Size);
                undo(g2, undo2);
            }
            undo(g1, undo1);
        }

        if (moveCount) {
            delete[] moves;
        }

    }

    //Compute dominance for white
    {
        size_t moveCount;
        int *moves = getMoves(g1, g1Size, 2, &moveCount);

        for (size_t i = 0; i < moveCount; i++) {
            play(g1, undo1, moves[2 * i], moves[2 * i + 1]);
            for (size_t j = i + 1; j < moveCount; j++) {
                play(g2, undo2, moves[2 * j], moves[2 * j + 1]);
                negateBoard(g2, g2Size);

                size_t g3Size;
                uint8_t *g3 = addGames(g1, g1Size, g2, g2Size, &g3Size); 
                int bFirst = solver->solveID(g3, g3Size, 1);
                int wFirst = solver->solveID(g3, g3Size, 2);
                delete[] g3;

                if (bFirst == wFirst) {
                    if (bFirst == 2) { // I - J is positive for white --> I > J
                        domWhite |= (((uint64_t) 1) << j);
                    } else { // I - J is negative for white --> I < J
                        domWhite |= (((uint64_t) 1) << i);
                    }
                }


                negateBoard(g2, g2Size);
                undo(g2, undo2);
            }
            undo(g1, undo1);
        }

        if (moveCount) {
            delete[] moves;
        }

    }

    dominance[0] = domBlack;
    dominance[1] = domWhite;

}


void computeBounds(uint8_t *board, size_t boardLen, int8_t *bounds) {
    uint8_t compare[32];

    map<int, int> ltFlags; //-1 false, 0 unknown, 1 true
    map<int, int> gtFlags;

    bool found = false;

    for (int i = 0; abs(i) < 31; i = i >= 0 ? -(i + 1) : -i) {

        memset(compare, 0, 32);
        int player = i < 0 ? 1 : 2; //we're adding the negative of compare -- flip 1 and 2


        compare[0] = opponentNumber(player);

        for (int j = 0; j < abs(i); j++) {
            compare[j + 1] = player;
        }

        //char *g = addGames(strlen(board), board, strlen(compare), compare);
        size_t gLen;
        uint8_t *g = addGames(board, boardLen, compare, strlen((char *) compare), &gLen);

        //int boardSize = strlen(board) + 1 + strlen(compare);
        //int result1 = gameResult(db, g, boardSize, 1);
        //int result2 = gameResult(db, g, boardSize, 2);

        int result1 = solver->solveID(g, gLen, 1);
        int result2 = solver->solveID(g, gLen, 2);

        delete[] g;

        int outcomeClass;
        if (result1 == result2) {
            outcomeClass = result1;
        } else {
            if (result1 == 1) {
                outcomeClass = OC_N;
            } else {
                outcomeClass = OC_P;
            }
        }


        // C >= G --> 0 >= G - C --> G - C <= 0
        #if defined(STRICT_BOUNDS)
        gtFlags[i] = (outcomeClass == OC_W) ? 1 : -1;
        #else
        gtFlags[i] = (outcomeClass == OC_W || outcomeClass == OC_P) ? 1 : -1;
        #endif

        // C <= G --> 0 <= G - C --> G - C >= 0
        #if defined(STRICT_BOUNDS)
        ltFlags[i] = (outcomeClass == OC_B) ? 1 : -1;
        #else
        ltFlags[i] = (outcomeClass == OC_B || outcomeClass == OC_P) ? 1 : -1;
        #endif
 

        //Now check if we can determine the bound from this
        int low, high;
        bool foundLow = false;
        bool foundHigh = false;

        for (int j = -31; j < 31; j++) {
            if (ltFlags[j] == 1 && ltFlags[j + 1] == -1) {
                low = j;
                foundLow = true;
                break;
            }
            
        }

        for (int j = 31; j > -31; j--) {
            if (gtFlags[j] == 1 && gtFlags[j - 1] == -1) {
                high = j;
                foundHigh = true;
                break;
            }
        }

        if (foundLow && foundHigh) {
            bounds[0] = low;
            bounds[1] = high;
            found = true;
            break;
        }

    }

    if (!found || bounds[0] > bounds[1]) {
        cout << "Bounds not found..." << endl;
        cout << "{" << (int) bounds[0] << " " << (int) bounds[1] << "}" << endl;


        cout << "LT" << endl;
        for (int i = -32; i < 32; i++) {
            cout << "(" << i << " " << ltFlags[i] << ") ";
        }
        cout << endl;

        cout << "GT" << endl;
        for (int i = -32; i < 32; i++) {
            cout << "(" << i << " " << gtFlags[i] << ") ";
        }
        cout << endl;

        while (1) {
        }
    }

}

bool mirror(uint8_t *board, size_t boardLen, uint64_t shapeNumber, uint32_t gameNumber) {
    //TODO this creates a different database because bounds might not be stable...
    //return false;

    uint64_t idx = db->getIdx(board, boardLen);
    assert(idx);
    uint8_t *entry = db->getFromIdx(idx);
    assert(entry);


    //Negated game
    negateBoard(board, boardLen);
    uint64_t idx2 = db->getIdx(board, boardLen);
    assert(idx);
    uint8_t *entry2 = db->getFromIdx(idx2);
    assert(entry2);
    negateBoard(board, boardLen);

    if (*db_get_outcome(entry2) == 0) {
        return false;
    }

    //outcome
    uint8_t outcome = *db_get_outcome(entry2);
    if (outcome == OC_B) {
        outcome = OC_W;
    } else if (outcome == OC_W) {
        outcome = OC_B;
    }
    *db_get_outcome(entry) = outcome;

    //dominance
    uint64_t domBlack = db_get_dominance(entry2)[0];
    uint64_t domWhite = db_get_dominance(entry2)[1];
    db_get_dominance(entry)[0] = domWhite;
    db_get_dominance(entry)[1] = domBlack;

    //bounds
    int8_t low = db_get_bounds(entry2)[0];
    int8_t high = db_get_bounds(entry2)[1];
    swap(low, high);
    low *= -1;
    high *= -1;
    db_get_bounds(entry)[0] = low;
    db_get_bounds(entry)[1] = high;

    //metric
    uint64_t metric = *db_get_metric(entry2);
    *db_get_metric(entry) = metric;

    //link
    uint64_t link = *db_get_link(entry2);

    if (link == idx2) {
        *db_get_link(entry) = idx;
    } else {
        uint8_t *linkedEntry = db->getFromIdx(link);
        assert(linkedEntry);

        uint64_t linkedShape = *db_get_shape(linkedEntry);
        uint32_t linkedGameNumber = *db_get_number(linkedEntry);

        size_t linkedGameLen;
        uint8_t *linkedGame;
        makeGame(linkedShape, linkedGameNumber, &linkedGame, &linkedGameLen);
        negateBoard(linkedGame, linkedGameLen);

        uint64_t newLink = db->getIdx(linkedGame, linkedGameLen);
        assert(newLink);

        *db_get_link(entry) = newLink;

        delete[] linkedGame;
    }

    //shape
    *db_get_shape(entry) = shapeNumber;

    //number
    *db_get_number(entry) = gameNumber;

    return true;
}

void addToReplacementMap(int low, int high, int outcome, uint64_t link) {
    triple<int, int, int> mapTriple(low, high, outcome);

    vector<uint64_t> *vec = replacementMap[mapTriple];
    if (!vec) {
        vec = new vector<uint64_t>();
        replacementMap[mapTriple] = vec;
    }
    vec->push_back(link);
}

void doPass(const vector<vector<int>> &shapeList, int pass, int batchLen) {
    //Iterate over all shapes
    for (const vector<int> &shape : shapeList) {
        uint64_t shapeNumber = shapeToNumber(shape);

        //iterate over all games
        int gameBits = 0;
        for (int chunk : shape) {
            gameBits += chunk;
        }

        uint32_t minGame = 0;
        uint32_t gameCount = 1 << gameBits;


        size_t _inducedSize = 0;
        _inducedSize += shape.size() - 1;
        for (int chunk : shape) {
            _inducedSize += chunk;
        }

        if ((_inducedSize != batchLen) && (pass == 1)) {
            continue;
        }
        if ((_inducedSize > RMAP_SIZE) && (pass == 0)) {
            continue;
        }

        for (uint32_t gameNumber = minGame; gameNumber < gameCount; gameNumber++) {
            //Get game and print it
            uint8_t *board;
            size_t boardLen;
            makeGame(shapeNumber, gameNumber, &board, &boardLen);
            cout << "Shape " << shape << " (" << shapeNumber << ") ";
            printBoard(board, boardLen, true);


            //Get entry
            uint64_t idx = db->getIdx(board, boardLen);
            assert(idx); 
            //cout << "idx " << idx << endl;
            uint8_t *entry = db->getFromIdx(idx);
            assert(entry);
            assert(*db_get_outcome(entry) == 0);

            if (gameNumber * 2 > gameCount && mirror(board, boardLen, shapeNumber, gameNumber)) {
                if (boardLen <= RMAP_SIZE && pass == 0) { //&& shape.size() == 1) {
                    int low = db_get_bounds(entry)[0];
                    int high = db_get_bounds(entry)[1];
                    int outcome = *db_get_outcome(entry);
                    addToReplacementMap(low, high, outcome, idx);
                }

                delete[] board;
                continue;
            }


            //Set trivial values (self link, shape, number)
            *db_get_link(entry) = idx;
            *db_get_shape(entry) = shapeNumber;
            *db_get_number(entry) = gameNumber;

            //Compute outcome class
            int result1 = solver->solveID(board, boardLen, BLACK);
            int result2 = solver->solveID(board, boardLen, WHITE);

            uint8_t outcomeClass = OC_UNKNOWN;

            if (result1 == result2) {
                outcomeClass = result1;
            } else {
                if (result1 == BLACK) {
                    outcomeClass = OC_N;
                } else {
                    outcomeClass = OC_P;
                }
            }


            *db_get_outcome(entry) = outcomeClass;
            //cout << (int) outcomeClass << " " << (int) *db_get_outcome(entry) << endl;


            //compute dominance
            if (boardLen <= DB_MAX_DOMINANCE_BITS) {
                uint64_t dominance[2];
                computeDominance(board, boardLen, dominance);

                db_get_dominance(entry)[0] = dominance[0];
                db_get_dominance(entry)[1] = dominance[1];
            }
            
            //compute bounds
            int8_t bounds[2];
            if (boardLen <= DB_MAX_BOUND_BITS) {
                computeBounds(board, boardLen, bounds);
                db_get_bounds(entry)[0] = bounds[0];
                db_get_bounds(entry)[1] = bounds[1];
                cout << (int) bounds[0] << " <> " << (int) bounds[1] << endl;
            }
            
            //compute metric

            if (boardLen <= DB_MAX_DOMINANCE_BITS) {
                size_t bMoveCount;
                size_t wMoveCount;
                int *bMoves = getMoves(board, boardLen, 1, &bMoveCount);
                int *wMoves = getMoves(board, boardLen, 2, &wMoveCount);

                uint64_t domBlack = db_get_dominance(entry)[0];
                uint64_t domWhite = db_get_dominance(entry)[1];

                int domCountB = sumBits(domBlack);
                int domCountW = sumBits(domWhite);

                uint64_t baseMetric = bMoveCount + wMoveCount - domCountB - domCountW;

                uint8_t undoBuffer[UNDO_BUFFER_SIZE];
                uint64_t metricSum = 0;
                size_t moveCounts[] = {bMoveCount, wMoveCount};
                int *moveArrs[] = {bMoves, wMoves};
                uint64_t doms[] = {domBlack, domWhite};

                //play moves
                for (int i = 0; i < 2; i++) {
                    size_t moveCount = moveCounts[i];
                    int *moves = moveArrs[i];
                    uint64_t dom = doms[i];

                    for (size_t i = 0; i < moveCount; i++) {
                        int from = moves[2 * i];
                        int to = moves[2 * i + 1];

                        if ((dom >> i) & 1) {
                            continue;
                        }

                        play(board, undoBuffer, from, to);
                        uint8_t *nextEntry = db->get(board, boardLen);

                        if (nextEntry) {
                            if (*db_get_outcome(nextEntry) == 0) {
                                cout << "Failed to make metric (haven't processed children yet)..." << endl;
                                while (1) {}
                            }

                            uint64_t childMetric = *db_get_metric(nextEntry);
                            if (metricSum + childMetric < metricSum) {
                                cout << "Metric overflow" << endl;
                                while (1) {}
                                metricSum = ((uint64_t) -1);
                            } else {
                                metricSum += childMetric;
                            }
                        }

                        undo(board, undoBuffer);
                    }
                }


                if (bMoveCount) {
                    delete[] bMoves;
                }

                if (wMoveCount) {
                    delete[] wMoves;
                }

                uint64_t metric = baseMetric + metricSum;
                *db_get_metric(entry) = metric;
            }


            //add to map
            if (boardLen <= RMAP_SIZE && pass == 0) { //&& shape.size() == 1) {
                addToReplacementMap(bounds[0], bounds[1], outcomeClass, idx);
            }


            delete[] board;
        }


    }
}

void makeLinks(const vector<vector<int>> &shapeList, int batchLen) {
    //do second pass to find links
    for (const vector<int> &shape : shapeList) {
        //if (shape.size() != 1) {
        //    continue;
        //}

        if (shape.size() > 2) {
            continue;
        }


        size_t _inducedSize = 0;
        _inducedSize += shape.size() - 1;
        for (int chunk : shape) {
            _inducedSize += chunk;
        }

        if (_inducedSize != batchLen) {
            continue;
        }

        uint64_t shapeNumber = shapeToNumber(shape);

        //iterate over all games
        int gameBits = 0;
        for (int chunk : shape) {
            gameBits += chunk;
        }

        uint32_t minGame = 0;
        uint32_t gameCount = 1 << gameBits;

        for (uint32_t gameNumber = minGame; gameNumber < gameCount; gameNumber++) {
            //Get game and print it
            uint8_t *board;
            size_t boardLen;

            makeGame(shapeNumber, gameNumber, &board, &boardLen);
            cout << "Shape (pass 2) " << shape << " (" << shapeNumber << ") ";
            printBoard(board, boardLen, true);

            if (boardLen > DB_MAX_SUB_BITS) {
                delete[] board;
                continue;
            }



            //Get entry
            uint64_t idx = db->getIdx(board, boardLen);
            assert(idx); 
            //cout << "idx " << idx << endl;
            uint8_t *entry = db->getFromIdx(idx);
            assert(entry);
            assert(*db_get_outcome(entry) != 0);


            //TODO enable mirroring later

//            if (gameNumber * 2 > gameCount && mirror(board, boardLen, shapeNumber, gameNumber)) {
//                delete[] board;
//                continue;
//            }


            //Find game in the replacement map and search all possible replacements
            triple<int, int, int> mapTriple(db_get_bounds(entry)[0], db_get_bounds(entry)[1], *db_get_outcome(entry));

            #if defined(DB_SORT)
            uint64_t simplestIdx = simplifyIdx(mapTriple, idx);
            if (simplestIdx != idx) {
                cout << "FOUND BETTER" << endl;
            }
            *db_get_link(entry) = simplestIdx;
            delete[] board;
            continue;
            #endif


            auto vecIt = replacementMap.find(mapTriple);
            if (vecIt == replacementMap.end()) {
                delete[] board;
                continue;
            }
            vector<uint64_t> *vec = vecIt->second;            


            //vector<uint64_t> *vec = replacementMap[mapTriple];
            if (!vec) {
                delete[] board;
                continue;
            }


            uint64_t bestMetric = *db_get_metric(entry);
            uint64_t bestLink = *db_get_link(entry);
            for (size_t i = 0; i < vec->size(); i++) {
                uint64_t idx = (*vec)[i];
                uint8_t *entry2 = db->getFromIdx(idx);
                assert(entry);
                uint64_t metric = *db_get_metric(entry2);
                if (metric >= bestMetric) {
                    continue;
                }

                //Check equality of games
                uint64_t gameShape = *db_get_shape(entry2);
                uint32_t gameNumber = *db_get_number(entry2);
                size_t gameLen;
                uint8_t *game;
                makeGame(gameShape, gameNumber, &game, &gameLen);
                negateBoard(game, gameLen);

                //if (gameLen > boardLen) {
                //    delete[] game;
                //    continue;
                //}

                size_t sumSize;
                uint8_t *gameSum = addGames(board, boardLen, game, gameLen, &sumSize);

                int outcome1 = solver->solveID(gameSum, sumSize, 1);
                int outcome2 = solver->solveID(gameSum, sumSize, 2);

                if (outcome1 == 2 && outcome2 == 1) {
                    bestMetric = metric;
                    bestLink = idx;
                }

                delete[] gameSum;
                delete[] game;
            }
            if (bestLink != *db_get_link(entry)) {
                cout << "Found better: " << bestLink << " (" << *db_get_metric(entry) << " -> " << bestMetric << ")" << endl;
            }
            *db_get_link(entry) = bestLink;

            delete[] board;
        }
    }

}


int main() {
    //Initialize: DB, solver, shapes
    db = new Database();
    db->init();

    solver = new Solver(DB_MAX_BITS, db);

    vector<vector<int>> shapeList = makeShapes();

    sort(shapeList.begin(), shapeList.end(),
        [](const vector<int> &s1, const vector<int> &s2) {
            int bits1 = s1.size() - 1;
            int bits2 = s2.size() - 1;

            for (int chunk : s1) {
                bits1 += chunk;
            }

            for (int chunk : s2) {
                bits2 += chunk;
            }


            if (bits1 == bits2) {
                return s1.size() > s2.size();
            }

            return bits1 < bits2;
        }
    );


    doPass(shapeList, 0, 0);

    #if defined(DB_SORT)
    cout << "SORTING" << endl;
    makeSortedMap();
    cout << "VERIFYING ORDERING" << endl;
    checkSorting();
    cout << "DONE" << endl;
    #endif


    for (int i = 2; i <= DB_MAX_BITS; i++) {
        if (i > RMAP_SIZE) {
            doPass(shapeList, 1, i);
        }

        if (i <= DB_MAX_SUB_BITS) {
            makeLinks(shapeList, i);
        }
    }




    db->save();

    delete solver;
    delete db;
}
