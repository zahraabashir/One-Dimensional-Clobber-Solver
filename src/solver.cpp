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

uint8_t *tt_get_best_moves(uint8_t *entry) {
    assert(_validEntry);
    if (entry == 0) {
        return 0;
    }

    return (uint8_t *) (entry + Offset<TTLayout, TT_BEST_MOVES>());
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
        pair<int, bool> result = searchID(board, len, n, p, 0);
        #endif

        if (result.second) {
            return result.first;
        }

        maxDepth += 1;
    }
}



pair<int, bool> Solver::searchID(uint8_t *board, size_t boardLen, int n, int p, int depth) {
    _validEntry = false;

    node_count += 1;

    size_t sboardLen = boardLen;
    uint8_t *sboard = new uint8_t[boardLen];
    memcpy(sboard, board, boardLen);

    if (depth > 0) {
        //simplify(&sboard, &sboardLen);
    }

    //lookup entry
    //if solved, return
    int code = getCode(sboard, sboardLen, n);
    uint8_t *blockPtr = getBlockPtr(code);
    uint8_t *entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n);

    uint8_t cachedOutcome = *tt_get_outcome(entryPtr);
    if (depth > 0 && cachedOutcome != OC_UNKNOWN) {
        delete[] sboard;
        return pair<int, bool>(cachedOutcome, true);
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


    if (depth > 0) {
        //Only Bs
        if ((outcomeMask & ~(1 << OC_B)) == 0 && counts[OC_B] > 0) {

            //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            if (true) {
                *tt_get_outcome(entryPtr) = BLACK;
                *tt_get_depth(entryPtr) = depth;
                *tt_get_heuristic(entryPtr) = 127 * (n == BLACK ? 1 : -1);
                *tt_get_valid(entryPtr) = true;
            }        
            delete[] sboard;
            return pair<int, bool>(OC_B, true);
        }

        //Only Ws
        if ((outcomeMask & ~(1 << OC_W)) == 0 && counts[OC_W] > 0) {

            //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            if (true) {
                *tt_get_outcome(entryPtr) = WHITE;
                *tt_get_depth(entryPtr) = depth;
                *tt_get_heuristic(entryPtr) = 127 * (n == WHITE ? 1 : -1);
                *tt_get_valid(entryPtr) = true;
            }        

            delete[] sboard;
            return pair<int, bool>(OC_W, true);
        }

        //Only one N
        if ((outcomeMask & ~(1 << OC_N)) == 0 && counts[OC_N] == 1) {
            //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            if (true) {
                *tt_get_outcome(entryPtr) = n;
                *tt_get_depth(entryPtr) = depth;
                *tt_get_heuristic(entryPtr) = 127;
                *tt_get_valid(entryPtr) = true;
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

            if (outcomes[i] == p) {
                for (int j = 0; j < lengths[i]; j++) {
                    sboardCopy[sg.first + j] = 0;
                }

                //Don't swap players or increase depth -- we haven't played a move
                pair<int, bool> result = searchID(sboardCopy, sboardCopyLen, n, p, depth);

                if (result.second && result.first == n) {

                    //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
                    entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n);
                    if (true) {
                        *tt_get_outcome(entryPtr) = n;
                        *tt_get_depth(entryPtr) = depth;
                        *tt_get_heuristic(entryPtr) = 127;
                        *tt_get_valid(entryPtr) = true;
                    }        
                    delete[] sboard;
                    return result;
                }


            }
        }
        #endif
    }


    //generate moves
    //check for terminal
    size_t moveCount;
    int *moves = getMoves(sboard, sboardLen, n, &moveCount);

    if (moveCount == 0) { //is terminal
        //cout << "No moves, " << p << " wins" << endl;
        completed += 1;
        //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n);
        if (true) {
            *tt_get_outcome(entryPtr) = p;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = -127;
            *tt_get_valid(entryPtr) = true;
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

        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n);
        int h = *tt_get_heuristic(entryPtr);
        if (!*tt_get_valid(entryPtr)) {
            h = -pMoveCount;
        }

        //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        if (true) {
            *tt_get_outcome(entryPtr) = EMPTY;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = h;
            *tt_get_valid(entryPtr) = true;
        }        

        delete[] moves;
        delete[] sboard;
        return pair<int, bool>(h, false);
    }

    //visit children, starting with best; find new best and update heuristic
    //if solved, save value and return


    vector<int> moveOrder;

    entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n);

    int bestMoves[3] = {-1, -1, -1};
    bool checkedBestMove = false;
    if (*tt_get_valid(entryPtr)) {
        //bestMove = BESTMOVE(entry);
        uint8_t *bms = tt_get_best_moves(entryPtr);
        for (int i = 0; i < 3; i++) {
            bestMoves[i] = bms[i];
            if (bms[0] == (uint8_t) -1) {
                bestMoves[i] = -1;
            }
        }
    }

    for (int i = 0; i < moveCount; i++) {
        if (moves[2 * i] == -1) {
            continue;
        }

        bool isBest = false;

        isBest = (i == bestMoves[0]) || (i == bestMoves[1]) || (i == bestMoves[2]);
        
        if (isBest || ((((uint64_t) 1) << moves[2 * i]) & opposingPositionMask) != 0) {
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

    for (int i = 2; i >= 0; i--) {
        if (bestMoves[i] != (uint8_t) -1 && moves[2 * bestMoves[i]] != -1) {
            moveOrder.push_back(bestMoves[i]);
        }
    }



    int bestVal = -127;

    uint8_t undoBuffer[UNDO_BUFFER_SIZE];

    int newBestMove = 0;

    bool allProven = true;

    // (i, score)
    vector<pair<int, int>> moveScores;


    //cout << "|| ";
    //printBoard(sboard, sboardLen);
    //cout << " " << playerNumberToChar(n) << " ||" << endl;
    //for (size_t i = 0; i < moveCount; i++) {
    //    cout << "(" << moves[2 * i] << " " << moves[2 * i + 1] << ")" << endl;
    //}

    //cout << "Move order: " << moveOrder << endl;

    for (auto it = moveOrder.rbegin(); it != moveOrder.rend(); it++) {
        int i = *it;

        if (i == -1) {
            continue;
        }

        //if (checkedBestMove && i == bestMove) {
        //    continue;
        //}
        
        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        if (from == -1) { //this move was pruned
            continue;
        }

        //cout << "Trying move " << from << " -> " << to << endl;

        play(sboard, undoBuffer, from, to);
        pair<int, bool> result = searchID(sboard, sboardLen, p, n, depth + 1);
        undo(sboard, undoBuffer);

        //cout << "Result of move: " << result.first << " " << result.second << endl;


        allProven &= result.second;

        if (result.second && result.first == n) {
            //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            if (true) {
                entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n);

                *tt_get_outcome(entryPtr) = n;
                tt_get_best_moves(entryPtr)[0] = i;
                *tt_get_depth(entryPtr) = depth;
                *tt_get_heuristic(entryPtr) = 127;
                *tt_get_valid(entryPtr) = true;
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
            moveScores.push_back({i, result.first});
        }

        //if (!checkedBestMove) {
        //    checkedBestMove = true;
        //    i = -1;
        //}
    }

    delete[] moves;


    entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n);

    if (allProven) {
        //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        if (true) {
            *tt_get_outcome(entryPtr) = p;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = -127;
            *tt_get_valid(entryPtr) = true;
        }

        delete[] sboard;
        return pair<int, bool>(p, true);
    }


    //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
    if (true) {
        *tt_get_outcome(entryPtr) = EMPTY;
        *tt_get_depth(entryPtr) = depth;
        *tt_get_heuristic(entryPtr) = bestVal;
        *tt_get_valid(entryPtr) = true;
        tt_get_best_moves(entryPtr)[0] = newBestMove;
        //BESTMOVE(entry) = newBestMove; //TODO
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

uint8_t *Solver::getEntryPtr(uint8_t *blockPtr, uint8_t *board, size_t len, int player) {
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
        uint8_t *eboard = *tt_get_board(entry);
        for (size_t i = 0; i < len; i++) {
            if (board[i] != eboard[i]) {
                continue;
            }
        }

        exists = true;
        break;
    }

    if (!exists) {
        idx = blockPtr[0];
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
        uint8_t *moves = tt_get_best_moves(entry);
        unsigned int *depth = tt_get_depth(entry);
        int8_t *heuristic = tt_get_heuristic(entry);
        bool *valid = tt_get_valid(entry);

        if (*elen != len) {
            if (*eboard != 0) {
                free(*eboard);
            }
            *eboard = (uint8_t *) malloc(len);
            *elen = len;
            memcpy(*eboard, board, len);

            *eplayer = player;
            *outcome = OC_UNKNOWN;
            moves[0] = -1;
            moves[1] = -1;
            moves[2] = -1;
            *depth = 0;
            *heuristic = 0;
            *valid = false;
        }

    }

    //cout << "Buffer ";
    //for (int i = 0; i < BLOCK_SIZE; i++) {
    //    cout << (int) blockPtr[i];
    //}
    //cout << endl;

    return entry;
}
