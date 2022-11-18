#include "solver.h"
#include "utils.h"
#include "state.h"
#include <cstring>
#include <iostream>
#include <algorithm>

using namespace std;

int node_count = 0; //nodes visited
int best_from = -1; //root player's move
int best_to = -1;
bool _validEntry = false;

uint8_t *tt_get_length(uint8_t *entry) {
    assert(_validEntry);
    if (entry == 0) {
        return 0;
    }
    return (uint8_t *) (entry + Offset<TTLayout, TT_LENGTH>());
}

uint8_t **tt_get_board(uint8_t *entry) {
    assert(_validEntry);
    if (entry == 0) {
        return 0;
    }
    return (uint8_t **) (entry + Offset<TTLayout, TT_BOARD>());
}

uint8_t *tt_get_player(uint8_t *entry) {
    assert(_validEntry);
    if (entry == 0) {
        return 0;
    }

    return (uint8_t *) (entry + Offset<TTLayout, TT_PLAYER>());
}

uint8_t *tt_get_outcome(uint8_t *entry) {
    assert(_validEntry);
    if (entry == 0) {
        return 0;
    }
    return (uint8_t *) (entry + Offset<TTLayout, TT_OUTCOME>());
}

int8_t *tt_get_best_moves(uint8_t *entry) {
    assert(_validEntry);
    if (entry == 0) {
        return 0;
    }

    return (int8_t *) (entry + Offset<TTLayout, TT_BEST_MOVES>());
}

unsigned int *tt_get_depth(uint8_t *entry) {
    assert(_validEntry);
    if (entry == 0) {
        return 0;
    }
    return (unsigned int *) (entry + Offset<TTLayout, TT_DEPTH>());
}

int8_t *tt_get_heuristic(uint8_t *entry) {
    assert(_validEntry);
    if (entry == 0) {
        return 0;
    }
    return (int8_t *) (entry + Offset<TTLayout, TT_HEURISTIC>());
}

bool *tt_get_valid(uint8_t *entry) {
    assert(_validEntry);
    if (entry == 0) {
        return 0;
    }

    return (bool *) (entry + Offset<TTLayout, TT_VALID>());
}


uint32_t *tt_get_hash(uint8_t *entry) {
    assert(_validEntry);
    if (entry == 0) {
        return 0;
    }
    return (uint32_t *) (entry + Offset<TTLayout, TT_HASH>());
}



vector<pair<int, int>> generateSubgames(uint8_t *board, size_t len) {
    vector<pair<int, int>> subgames;

    int start = -1;
    int end = -1;

    int foundMask = 0;

    for (int i = 0; i < len; i++) {
        if (start == -1 && board[i] != 0) {
            start = i;
            foundMask = 0;
        }
        
        if (board[i] != 0) {
            foundMask |= board[i];
        }

        if (start != -1 && board[i] == 0) {
            if (foundMask == 3) {
                subgames.push_back(pair<int, int>(start, i));
            }
            start = -1;
        }
    }

    if (start != -1 && foundMask == 3) {
        subgames.push_back(pair<int, int>(start, len));
    }

          
    return subgames;
}


Solver::Solver(size_t boardLen, Database *db) {
    _validEntry = false;
    this->boardLen = boardLen;
    this->db = db;
    this->rng = new default_random_engine(3.141);

    int codeBits = 24;
    if (boardLen > 38) {
        codeBits = 23;
    }

    //account for block size...
    for (int i = BLOCK_SIZE; i > 1; ) {
        codeBits -= 1;
        i >>= 1;
    }

    codeLength = codeBits;


    codeMask = 0;
    for (int i = 0; i < codeBits; i++) {
        codeMask <<= 1;
        codeMask |= 1;
    }

    blockCount = ((size_t) 1) << codeBits;
    //tableEntryCount = ((size_t) 1) << codeBits;

    totalBlockSize = (BLOCK_SIZE * tableEntrySize) + blockHeaderSize;
    tableSize = blockCount * totalBlockSize;

    DBOUT(
        cout << "Block count " << blockCount << endl;
    );



    //cout << "Entry size: " << tableEntrySize << endl;
    //cout << "Size: " << (double) tableSize / (1000.0 * 1000.0) << endl;
    //cout << "Board size: " << boardSize << endl;   

    table = (uint8_t *) calloc(tableSize, 1);
}

Solver::~Solver() {
    free(table);
    delete rng;
}


int Solver::solveID(uint8_t *board, size_t len, int n) {
    int p = opponentNumber(n);

    doABPrune = false;
    maxCompleted = 1;
    limitCompletions = true;
    maxDepth = 0;

    while (true) {
        //collisions = 0;
        completed = 0;

        //used to be 100
        maxCompleted += 50;

        //used to be 150
        if (maxDepth >= 30) {
            limitCompletions = false;
        }

        if (maxDepth >= 18) { //was 10
            doABPrune = true;
        }

        #if defined(SOLVER_ALPHA_BETA)
        Bound alpha = Bound::min();
        Bound beta = Bound::max();
        Bound cb1, cb2;
        pair<int, bool> result = rootSearchID(state, p, n, 0, alpha, beta, cb1, cb2);
        #else
        pair<int, bool> result = rootSearchID(board, len, n, p, 0);
        #endif

        if (result.second) {
            return result.first;
        }

        maxDepth += 1;
    }
}

//void Solver::simplify(State *state, int depth) {
void Solver::simplify(uint8_t **board, size_t *boardLen) {
    //return;
    //cout << "+++" << endl;
    //printBoard(*board, *boardLen, true);

    //Get all subgames
    vector<pair<int, int>> _subgames = generateSubgames(*board, *boardLen);
    vector<pair<int, int>> subgames;
    for (const pair<int, int> &sg : _subgames) {
        subgames.push_back({sg.first, sg.second - sg.first});
    }


    vector<pair<uint8_t *, size_t>> replacements;
    // Look up subgames and add their inflated linked games to a vector
    for (size_t i = 0; i < subgames.size(); i++) {
        const pair<int, int> &sg = subgames[i];
        size_t sgOffset = sg.first;
        size_t sgLen = sg.second;


        uint8_t *entry = db->get(*board + sgOffset, sgLen); 
        if (entry == 0 || *db_get_outcome(entry) == 0) {
            uint8_t *gameChunk = new uint8_t[sgLen];
            memcpy(gameChunk, *board + sgOffset, sgLen);
            replacements.push_back({gameChunk, sgLen});
            continue;
        }

        //skip P positions
        if (*db_get_outcome(entry) == OC_P) {
            continue;
        }

        //inflate linked subgame
        uint64_t link = *db_get_link(entry);
        uint8_t *linkedEntry = db->getFromIdx(link);
        assert(linkedEntry);
        uint64_t linkedShapeNumber = *db_get_shape(linkedEntry);
        uint32_t linkedGameNumber = *db_get_number(linkedEntry);
        uint8_t *linkedGame;
        size_t linkedGameLen;
        makeGame(linkedShapeNumber, linkedGameNumber, &linkedGame, &linkedGameLen);

        replacements.push_back({linkedGame, linkedGameLen});
    }

    //Ignore chunks that are negatives of each other
    for (size_t I = 0; I < replacements.size(); I++) {
        for (size_t J = I + 1; J < replacements.size(); J++) {
            auto &r1 = replacements[I];
            auto &r2 = replacements[J];
            if (r1.second != r2.second) {
                continue;
            }
            if (r1.first == 0 || r2.first == 0) {
                continue;
            }

            size_t len = r1.second;
            uint8_t *b1 = r1.first;
            uint8_t *b2 = r2.first;

            int failState = 0;
            for (int i = 0; i < len; i++) {
                if ((b1[i] + b2[i]) != 3) {
                    failState += 1;
                    break;
                }
            }

            for (int i = 0; i < len; i++) {
                if ((b1[i] + b2[len - 1 - i]) != 3) {
                    failState += 2;
                    break;
                }
            }

            if (failState != 3) {
                delete[] b1;
                delete[] b2;
                r1.first = 0;
                r2.first = 0;
            }

        }
    }

    //Filter out 0s
    {

        vector<pair<uint8_t *, size_t>> replacements2;
        for (auto x : replacements) {
            replacements2.push_back(x);
        }

        replacements.clear();

        for (auto x : replacements2) {
            if (x.first != 0) {
                replacements.push_back(x);
            }
        }
        
        
    }
    

    //Combine chunks to get result
    delete[] *board;

    *boardLen = max(((int) replacements.size()) - 1, 0);
    for (const pair<uint8_t *, size_t> &r : replacements) {
        *boardLen += r.second;
    }

    *board = new uint8_t[*boardLen];

    sort(replacements.begin(), replacements.end(),
        [](const pair<uint8_t *, size_t> &r1, const pair<uint8_t *, size_t> &r2) {
            for (size_t i = 0; i < min(r1.second, r2.second); i++) {
                if (r1.first[i] == r2.first[i]) {
                    continue;
                }

                if (r1.first[i] < r2.first[i]) {
                    return true;
                } else {
                    return false;
                }
            }
            return r1.second < r2.second;
        }
    );

    size_t offset = 0;
    memset(*board, 0, *boardLen);
    for (const pair<uint8_t *, size_t> &r : replacements) {
        memcpy(*board + offset, r.first, r.second);
        offset += r.second + 1;
    }

    //Clean up
    for (pair<uint8_t *, size_t> &r : replacements) {
        delete[] r.first;
    }

    //Don't return 0 length board...
    if (*boardLen == 0) {
        delete[] *board;
        *boardLen = 1;
        *board = new uint8_t[1];
        (*board)[0] = 1;
    }

}




pair<int, bool> Solver::rootSearchID(uint8_t *board, size_t boardLen, int n, int p, int depth) {
    _validEntry = false;
    node_count += 1;

    DBOUT(
        cout << endl;
        printBoard(board, boardLen, false);
        cout << " " << playerNumberToChar(n) << " " << depth << endl;

    );

    size_t sboardLen = boardLen;
    uint8_t *sboard = new uint8_t[boardLen];
    memcpy(sboard, board, boardLen);

    //lookup entry
    //if solved, return
    int code = getCode(sboard, sboardLen, n);
    uint32_t hash2 = getHash2(sboard, sboardLen, n);
    uint8_t *blockPtr = getBlockPtr(code);
    uint8_t *entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 0);

    bool validEntry = entryPtr != 0;

    if (validEntry && *tt_get_outcome(entryPtr) != 0) {
        delete[] sboard;
        return pair<int, bool>(*tt_get_outcome(entryPtr), true);
    }

    //generate moves
    //check for terminal
    size_t moveCount;
    int *moves = getMoves(sboard, sboardLen, n, &moveCount);

    if (moveCount == 0) { //is terminal
        completed += 1;
        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
        if (true) {
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = p;
            tt_get_best_moves(entryPtr)[0] = 0;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = -127;
        }

        delete[] sboard;
        return pair<int, bool>(p, true);
    }


    //Delete dominated moves
    #if defined(SOLVER_DELETE_DOMINATED_MOVES)
    vector<pair<int, int>> sg = generateSubgames(sboard, sboardLen);

    for (int i = 0; i < sg.size(); i++) {
        int start = sg[i].first;
        int end = sg[i].second; //index after end
        int len = end - start;

        uint8_t *dbEntry = db->get(&sboard[start], len);

        uint64_t dominated = dbEntry ? db_get_dominance(dbEntry)[n - 1] : 0;

        int moveIndex = 0;
        for (int j = 0; j < moveCount; j++) {
            int from = moves[2 * j];
            int to = moves[2 * j + 1];

            if (from >= start && from < end) { //found move
                if ((dominated >> moveIndex) & ((uint64_t) 1)) {
                    //cout << "FOUND" << endl;
                    moves[2 * j] = -1;
                    moves[2 * j + 1] = -1;
                }
                moveIndex++;
            }
        }

    }
    #endif




    //if deep, generate heuristic and return
    if (depth == maxDepth || (limitCompletions && (completed >= maxCompleted))) {
        completed += 1;

        size_t pMoveCount;
        int *pMoves = getMoves(sboard, sboardLen, p, &pMoveCount);

        if (pMoveCount > 0) {
            delete[] pMoves;
        }

        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);

        int h = *tt_get_heuristic(entryPtr);
        if (!validEntry) {
            h = -pMoveCount;
        }

        //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        if (true) {
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = EMPTY;
            tt_get_best_moves(entryPtr)[0] = 0;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = h;
        }        

        delete[] moves;
        delete[] sboard;
        return pair<int, bool>(h, false);
    }

    //visit children, starting with best; find new best and update heuristic
    //if solved, save value and return

    int bestMove = 0;
    bool checkedBestMove = false;
    if (validEntry) {
        bestMove = tt_get_best_moves(entryPtr)[0];
    }

    int bestVal = -127;
    uint8_t undoBuffer[UNDO_BUFFER_SIZE];
    int newBestMove = 0;
    bool allProven = true;


    for (int i = bestMove; i < moveCount; i++) {
        if (checkedBestMove && i == bestMove) {
            continue;
        }


        
        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        if (from == -1) { //this move was pruned
            if (!checkedBestMove) {
                checkedBestMove = true;
                i = -1;
            }
            continue;
        }


        play(sboard, undoBuffer, from, to);
        pair<int, bool> result = searchID(sboard, sboardLen, p, n, depth + 1);
        undo(sboard, undoBuffer);

        allProven &= result.second;

        if (result.second && result.first == n) {
            //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            if (true) {
                entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
                *tt_get_valid(entryPtr) = true;
                *tt_get_outcome(entryPtr) = n;
                tt_get_best_moves(entryPtr)[0] = i;
                *tt_get_depth(entryPtr) = depth;
                *tt_get_heuristic(entryPtr) = 127;
            }
            best_from = from;
            best_to = to;

            delete[] moves;
            delete[] sboard;
            return pair<int, bool>(n, true);
        }

        if (!result.second) {
            result.first *= -1;
            if (result.first > bestVal) {
                newBestMove = i;
                bestVal = result.first;
            }
        }

        if (!checkedBestMove) {
            checkedBestMove = true;
            i = -1;
        }
    }

    delete[] moves;



    if (allProven) {
        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
        if (true) {
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = p;
            tt_get_best_moves(entryPtr)[0] = newBestMove;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = -127;
        }
        best_from = -1;
        best_to = -1;
        delete[] sboard;
        return pair<int, bool>(p, true);
    }


    if (true) {
        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
        *tt_get_valid(entryPtr) = true;
        *tt_get_outcome(entryPtr) = EMPTY;
        tt_get_best_moves(entryPtr)[0] = newBestMove;
        *tt_get_depth(entryPtr) = depth;
        *tt_get_heuristic(entryPtr) = bestVal;
    }
    delete[] sboard;
    return pair<int, bool>(bestVal, false);
}

pair<int, bool> Solver::searchID(uint8_t *board, size_t boardLen, int n, int p, int depth) {
    _validEntry = false;

    DBOUT(
        cout << endl;
        printBoard(board, boardLen, false);
        cout << " " << playerNumberToChar(n) << " " << depth << endl;

    );


    node_count += 1;

    size_t sboardLen = boardLen;
    uint8_t *sboard = new uint8_t[boardLen];
    memcpy(sboard, board, boardLen);

    simplify(&sboard, &sboardLen);

    DBOUT(
        cout << "SIMPLIFY:" << endl;
        printBoard(sboard, sboardLen, false);
        cout << " " << playerNumberToChar(n) << " " << depth << endl;

    );




    //lookup entry
    //if solved, return
    int code = getCode(sboard, sboardLen, n);
    uint32_t hash2 = getHash2(sboard, sboardLen, n);
    uint8_t *blockPtr = getBlockPtr(code);
    uint8_t *entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 0);

    bool validEntry = entryPtr != 0;

    if (validEntry && *tt_get_outcome(entryPtr) != 0) {
        delete[] sboard;
        return pair<int, bool>(*tt_get_outcome(entryPtr), true);
    }

    //generate subgames, look them up in the database
    vector<pair<int, int>> subgames = generateSubgames(sboard, sboardLen);

    //sort subgames by length
    sort(subgames.begin(), subgames.end(),
        [](const pair<int, int> &a, const pair<int, int> &b) {
            return (a.second - a.first) > (b.second - b.first);
        }
    );

    vector<int> lengths;
    vector<int> outcomes;

    //count outcomes
    int counts[5];
    for (int i = 0; i < 5; i++) {
        counts[i] = 0;
    }

    int outcomeMask = 0;


    //this is OK because 64 is the maximum board size
    uint64_t opposingPositionMask = 0;

    for (auto it = subgames.begin(); it != subgames.end(); it++) {
        int length = it->second - it->first;
        lengths.push_back(length);

        uint8_t *dbEntry = db->get(&sboard[it->first], length);
        int outcome = dbEntry ? *db_get_outcome(dbEntry) : OC_UNKNOWN;
        outcomes.push_back(outcome);

        counts[outcome] += 1;
        outcomeMask |= (1 << outcome);

        if ((outcome == OC_B && n == WHITE) || (outcome == OC_W && n == BLACK)) {
            uint64_t mask = 1;

            for (int i = 1; i < length; i++) {
                mask <<= 1;
                mask |= 1;
            }

            mask <<= it->first;
            opposingPositionMask |= mask;
        }
    }


    //Only Bs
    if ((outcomeMask & ~(1 << OC_B)) == 0 && counts[OC_B] > 0) {
        //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        if (true) {
            entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = BLACK;
            tt_get_best_moves(entryPtr)[0] = 0;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = 127 * (n == BLACK ? 1 : -1);
        }        
        delete[] sboard;
        return pair<int, bool>(OC_B, true);
    }

    //Only Ws
    if ((outcomeMask & ~(1 << OC_W)) == 0 && counts[OC_W] > 0) {
        //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        if (true) {
            entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = WHITE;
            tt_get_best_moves(entryPtr)[0] = 0;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = 127 * (n == WHITE ? 1 : -1);
        }        

        delete[] sboard;
        return pair<int, bool>(OC_W, true);
    }

    //Only one N
    if ((outcomeMask & ~(1 << OC_N)) == 0 && counts[OC_N] == 1) {
        //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        if (true) {
            entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = n;
            tt_get_best_moves(entryPtr)[0] = 0;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = 127;
        }        
        delete[] sboard;
        return pair<int, bool>(n, true); //current player wins
    }


    //Use differences
    #if defined(SOLVER_DELETE_SUBGAMES)

    for (int i = 0; i < subgames.size(); i++) {
        uint8_t sboardCopy[sboardLen];
        size_t sboardCopyLen = sboardLen;
        memcpy(sboardCopy, sboard, sboardLen);

        pair<int, int> &sg = subgames[i];

        if (outcomes[i] == n) {
            for (int j = 0; j < lengths[i]; j++) {
                sboardCopy[sg.first + j] = 0;
            }

            //Don't swap players or increase depth -- we haven't played a move
            pair<int, bool> result = searchID(sboardCopy, sboardCopyLen, n, p, depth);

            if (result.second && result.first == n) {

                //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
                entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
                if (true) {
                    *tt_get_valid(entryPtr) = true;
                    *tt_get_outcome(entryPtr) = n;
                    tt_get_best_moves(entryPtr)[0] = 0;
                    *tt_get_depth(entryPtr) = depth;
                    *tt_get_heuristic(entryPtr) = 127;
                }        
                delete[] sboard;
                return result;
            }


        }
    }
    #endif


    //generate moves
    //check for terminal
    size_t moveCount;
    int *moves = getMoves(sboard, sboardLen, n, &moveCount);

    if (moveCount == 0) { //is terminal
        completed += 1;
        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
        if (true) {
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = p;
            tt_get_best_moves(entryPtr)[0] = 0;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = -127;
        }

        delete[] sboard;
        return pair<int, bool>(p, true);
    }


    //Delete dominated moves
    #if defined(SOLVER_DELETE_DOMINATED_MOVES)
    vector<pair<int, int>> sg = generateSubgames(sboard, sboardLen);

    for (int i = 0; i < sg.size(); i++) {
        int start = sg[i].first;
        int end = sg[i].second; //index after end
        int len = end - start;

        uint8_t *dbEntry = db->get(&sboard[start], len);

        uint64_t dominated = dbEntry ? db_get_dominance(dbEntry)[n - 1] : 0;

        int moveIndex = 0;
        for (int j = 0; j < moveCount; j++) {
            int from = moves[2 * j];
            int to = moves[2 * j + 1];

            if (from >= start && from < end) { //found move
                if ((dominated >> moveIndex) & ((uint64_t) 1)) {
                    //cout << "FOUND" << endl;
                    moves[2 * j] = -1;
                    moves[2 * j + 1] = -1;
                }
                moveIndex++;
            }
        }

    }
    #endif




    //if deep, generate heuristic and return
    if (depth == maxDepth || (limitCompletions && (completed >= maxCompleted))) {
        completed += 1;

        size_t pMoveCount;
        int *pMoves = getMoves(sboard, sboardLen, p, &pMoveCount);

        if (pMoveCount > 0) {
            delete[] pMoves;
        }

        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
        int h = *tt_get_heuristic(entryPtr);
        if (!validEntry) {
            h = -pMoveCount;
        }

        //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        if (true) {
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = EMPTY;
            tt_get_best_moves(entryPtr)[0] = 0;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = h;
        }        

        delete[] moves;
        delete[] sboard;
        return pair<int, bool>(h, false);
    }

    //visit children, starting with best; find new best and update heuristic
    //if solved, save value and return


    vector<int> moveOrder;

    int bestMove = -1;
    bool checkedBestMove = false;

    if (validEntry) {
        bestMove = tt_get_best_moves(entryPtr)[0];
    }

    for (int i = 0; i < moveCount; i++) {
        if (moves[2 * i] == -1) {
            continue;
        }
        
        if (i == bestMove || ((((uint64_t) 1) << moves[2 * i]) & opposingPositionMask) != 0) {
            continue;
        }
        moveOrder.push_back(i);
    }

    shuffle(moveOrder.begin(), moveOrder.end(), *rng);

    for (int i = 0; i < moveCount; i++) {
        if (moves[2 * i] == -1) {
            continue;
        }

        if ((((uint64_t) 1) << moves[2 * i]) & opposingPositionMask) {
            moveOrder.push_back(i);
        }
    }

    if (bestMove != -1) {
        if (moves[2 * bestMove] != -1) {
            moveOrder.push_back(bestMove);
        } else {
            bestMove = -1;
        }   
    }   
    


    int bestVal = -127;

    uint8_t undoBuffer[UNDO_BUFFER_SIZE];

    int newBestMove = 0;

    bool allProven = true;


    for (auto it = moveOrder.rbegin(); it != moveOrder.rend(); it++) {
        int i = *it;

        if (checkedBestMove && i == bestMove) {
            continue;
        }

        int from = moves[2 * i];
        int to = moves[2 * i + 1];


        if (from == -1) { //this move was pruned
            continue;
        }

        play(sboard, undoBuffer, from, to);
        pair<int, bool> result = searchID(sboard, sboardLen, p, n, depth + 1);
        undo(sboard, undoBuffer);



        allProven &= result.second;

        if (result.second && result.first == n) {
            //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            if (true) {
                entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
                *tt_get_valid(entryPtr) = true;
                *tt_get_outcome(entryPtr) = n;
                tt_get_best_moves(entryPtr)[0] = i;
                *tt_get_depth(entryPtr) = depth;
                *tt_get_heuristic(entryPtr) = 127;
            }

            delete[] moves;
            delete[] sboard;
            return pair<int, bool>(n, true);
        }

        if (!result.second) {
            result.first *= -1;
            if (result.first > bestVal) {
                newBestMove = i;
                bestVal = result.first;
            }
        }

        if (!checkedBestMove) {
            checkedBestMove = true;
            i = -1;
        }
    }

    delete[] moves;



    if (allProven) {
        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
        if (true) {
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = p;
            tt_get_best_moves(entryPtr)[0] = newBestMove;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = -127;
        }

        delete[] sboard;
        return pair<int, bool>(p, true);
    }


    if (true) {
        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
        *tt_get_valid(entryPtr) = true;
        *tt_get_outcome(entryPtr) = EMPTY;
        tt_get_best_moves(entryPtr)[0] = newBestMove;
        *tt_get_depth(entryPtr) = depth;
        *tt_get_heuristic(entryPtr) = bestVal;
    }
    delete[] sboard;
    return pair<int, bool>(bestVal, false);
}



uint8_t *Solver::getBlockPtr(int code) {
    int idx = code & codeMask;
    for (int i = 1; ; i++) {
        int shift = i * codeLength;

        if (shift >= sizeof(int) * 8) {
            break;
        }

        int add = code >> shift;
        add &= codeMask;
        code += add;
    }
    code &= codeMask;

    return table + (idx * totalBlockSize);
}

/*
    0 -- read only
    1 -- allocate
    2 -- read + run policy
*/
uint8_t *Solver::getEntryPtr(uint8_t *blockPtr, uint8_t *board, size_t len, int player, uint32_t hash, int mode) {
    _validEntry = true;
    bool exists = false;
    if (len == 0) {
        return 0;
    }

    //initialize
    if (blockPtr[0] == 0 && blockPtr[1] == 0) {
        for (int i = 0; i < BLOCK_SIZE; i++) {
            blockPtr[i] = i;
        }
    }

    uint8_t *first = blockPtr + blockHeaderSize;

    //Search all for a match
    int idx = BLOCK_SIZE - 1;
    for (; idx >= 0; idx--) {
        uint8_t *entry = first + idx * tableEntrySize;

        //Check hash
        if (hash != *tt_get_hash(entry)) {
            continue;
        }

        //Check size
        uint8_t esize = *tt_get_length(entry);
        if ((size_t) esize != len) {
            continue;
        }

        //Check player
        uint8_t eplayer = *tt_get_player(entry);
        if ((int) eplayer != player) {
            continue;
        }

        //Check content...
        exists = true;
        uint8_t *eboard = *tt_get_board(entry);
        for (size_t i = 0; i < len; i++) {
            if (board[i] != eboard[i]) {
                exists = false;
                break;
            }
        }

        if (exists) {
            break;
        }

    }

    if (!exists) {
        if (mode == 0) {
            return 0;
        }
        idx = blockPtr[0];
        uint8_t *entry0 = first + idx * tableEntrySize;
        if (*tt_get_valid(entry0) && true) {
            //find best value
            double bestValue = numeric_limits<double>::infinity();
            idx = 0;

            for (int i = 0; i < BLOCK_SIZE; i++) {
                int slot = blockPtr[i];
                uint8_t *entry = first + slot * tableEntrySize;

                unsigned int depth = *tt_get_depth(entry);
                int age = BLOCK_SIZE - i;

                //double value = ((double) -depth) - 1.0 * ((double) age) / ((double) BLOCK_SIZE);

                //double value = -depth * ((double) age / (double) BLOCK_SIZE);
                double value = -(depth * depth) * ((double) age / (double) BLOCK_SIZE);

                if (value < bestValue) {
                    bestValue = value;
                    idx = slot;
                }
            }
        }
    }
    //idx now indicates the slot we use


    //Update most recently used...
    for (int i = 0; i < BLOCK_SIZE; i++) {
        if (blockPtr[i] == idx) {
            memmove(blockPtr + i, blockPtr + i + 1, BLOCK_SIZE - i - 1); 
            blockPtr[BLOCK_SIZE - 1] = idx;
        }
    }

    uint8_t *entry = first + idx * tableEntrySize;

    if (!exists) {
        uint8_t *elen = tt_get_length(entry);
        uint8_t **eboard = tt_get_board(entry);
        uint8_t *eplayer = tt_get_player(entry);
        uint8_t *outcome = tt_get_outcome(entry);
        int8_t *moves = tt_get_best_moves(entry);
        unsigned int *depth = tt_get_depth(entry);
        int8_t *heuristic = tt_get_heuristic(entry);
        bool *valid = tt_get_valid(entry);
        uint32_t *ehash = tt_get_hash(entry);

        if (*elen != len) {
            if (*eboard != 0) {
                free(*eboard);
            }
            *eboard = (uint8_t *) malloc(len);
            *elen = len;

        }

        memcpy(*eboard, board, len);

        *eplayer = player;
        *outcome = OC_UNKNOWN;
        for (int i = 0; i < STORED_BEST_MOVES; i++) {
            moves[i] = -1;
        }
        *depth = 0;
        *heuristic = 0;
        *valid = false;
        *ehash = hash;
    }

    //cout << "Buffer ";
    //for (int i = 0; i < BLOCK_SIZE; i++) {
    //    cout << (int) blockPtr[i];
    //}
    //cout << endl;


    //cout << "Exists " << exists << endl;

    return entry;
}
