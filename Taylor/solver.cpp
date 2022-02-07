#include "solver.h"
#include "utils.h"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <limits>


int node_count = 0;
int best_from = 0;
int best_to = 0;

int collisions = 0;

BasicSolver::BasicSolver(int rootPlayer, int boardSize) {
    this->rootPlayer = rootPlayer;
    this->rootOpponent = opponentNumber(rootPlayer);
    this->boardSize = boardSize;

    int bits = 24;
    codeLength = bits;
    //board, player, outcome, heuristic, bestmove
    tableEntrySize = boardSize + 3 + 2 * sizeof(int);

    bitMask = 0;
    for (int i = 0; i < bits; i++) {
        bitMask <<= 1;
        bitMask |= 1;
    }

    size_t tableSize = 1;
    tableSize <<= (size_t) bits;
    tableSize *= (size_t) tableEntrySize;


    std::cout << "SIZE: " << tableSize << std::endl;
    std::cout << sizeof(size_t) << std::endl;

    table = (char *) calloc(tableSize, 1);
}

BasicSolver::~BasicSolver() {
    free(table);
}

bool BasicSolver::validateTableEntry(State *state, int p, char *entry) {
    bool found = false;
    if (PLAYER(entry) == p) {
        found = true;
        for (int i = 0; i < boardSize; i++) {
            if (entry[i] != state->board[i]) {
                found = false;
                break;
            }
        }
    }
    return found;
}

int BasicSolver::solveID(State *state, int p, int n) {
    maxCompleted = 10;

    int depth = 0;
    while (true) {
        maxDepth = depth;
        collisions = 0;

        completed = 0;
        maxCompleted += 5;

        std::pair<int, bool> result = searchID(state, p, n, 0);
        std::cout << depth << " " << collisions << std::endl;

        if (result.second) {
            return result.first;
        }

        depth += 1;
    }
}


std::pair<int, bool> BasicSolver::searchID(State *state, int p, int n, int depth) {
    //lookup entry
    //if solved, return
    int code = state->code(p);
    char *entry = getTablePtr(code);

    bool validEntry = validateTableEntry(state, p, entry);
    if (validEntry && OUTCOME(entry) != EMPTY) {
        return std::pair<int, bool>(OUTCOME(entry), true);
    }

    if (!validEntry && PLAYER(entry) != 0) {
        collisions++;
    }

    //generate moves
    //check for terminal
    size_t moveCount;
    int *moves = state->getMoves(p, n, &moveCount);

    if (moveCount == 0) { //is terminal
        completed += 1;
        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = n;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = 10000;
        }
        return std::pair<int, bool>(n, true);
    }


    //if deep, generate heuristic and return
    if (depth == maxDepth || completed >= maxCompleted) {
        completed += 1;

        size_t pMoveCount;
        int *pMoves = state->getMoves(n, p, &pMoveCount);

        if (pMoveCount > 0) {
            delete[] pMoves;
        }

        int h = (int) moveCount - (int) pMoveCount;
        if (validEntry) {
            h = std::max<int>(h, HEURISTIC(entry));
        }

        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = EMPTY;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = h;
        }        

        return std::pair<int, bool>(h, false);
    }

    //visit children, starting with best; find new best and update heuristic
    //if solved, save value and return
    int bestMove = 0;
    bool checkedBestMove = false;
    if (validEntry) {
        bestMove = BESTMOVE(entry);
    }

    int bestVal = -10000;

    char undoBuffer[sizeof(int) + 2 * sizeof(char)];

    int newBestMove = 0;

    bool allProven = true;

    for (int i = bestMove; i < moveCount; i++) {
        if (checkedBestMove && i == bestMove) {
            continue;
        }

        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        state->play(from, to, undoBuffer);
        std::pair<int, bool> result = searchID(state, n, p, depth + 1);
        state->undo(undoBuffer);

        allProven &= result.second;

        if (result.second && result.first == p) {
            if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
                memcpy(entry, state->board, boardSize);
                PLAYER(entry) = p;
                OUTCOME(entry) = p;
                BESTMOVE(entry) = i;
                DEPTH(entry) = depth;
                HEURISTIC(entry) = 10000;
            }

            delete[] moves;
            return std::pair<int, bool>(p, true);
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
        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = n;
            BESTMOVE(entry) = newBestMove; //these two values don't matter -- node result is known
            DEPTH(entry) = depth;
            HEURISTIC(entry) = -10000;

        }
        return std::pair<int, bool>(n, true);
    }


    if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        memcpy(entry, state->board, boardSize);
        PLAYER(entry) = p;
        OUTCOME(entry) = EMPTY;
        BESTMOVE(entry) = newBestMove;
        DEPTH(entry) = depth;
        HEURISTIC(entry) = bestVal;
    }
    return std::pair<int, bool>(bestVal, false);
}

// to print the first best move
int BasicSolver::solveRoot(State *state, int p, int n) {

    //Look up state in table
    int code = state->code(p);
    char *entry = getTablePtr(code);

    bool found = false;
    if (PLAYER(entry) == p) {
        found = true;
        for (int i = 0; i < boardSize; i++) {
            if (entry[i] != state->board[i]) {
                found = false;
                break;
            }
        }
    }

    if (found) {
        return OUTCOME(entry);
    }

    size_t moveCount;
    int *moves = state->getMoves(p, n, &moveCount);

    if (moveCount == 0) {
        memcpy(entry, state->board, boardSize);
        PLAYER(entry) = p;
        OUTCOME(entry) = n;
        node_count += 1;
        return n;
    }

    char undoBuffer[sizeof(int) + 2 * sizeof(char)];

    for (size_t i = 0; i < moveCount; i++) {
        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        state->play(from, to, undoBuffer);
        node_count += 1;
        int result = solve(state, n, p);
        state->undo(undoBuffer);

        if (result == p) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = p;
            best_from = from;
            best_to=to;
            delete[] moves;
            return p;
        }
    }

    memcpy(entry, state->board, boardSize);
    PLAYER(entry) = p;
    OUTCOME(entry) = n;
    best_from=-1;
    best_to=-1;

    delete[] moves;
    return n;
}

int BasicSolver::solve(State *state, int p, int n) {

    //Look up state in table
    int code = state->code(p);
    char *entry = getTablePtr(code);

    bool found = false;
    if (PLAYER(entry) == p) {
        found = true;
        for (int i = 0; i < boardSize; i++) {
            if (entry[i] != state->board[i]) {
                found = false;
                break;
            }
        }
    }

    if (found) {
        return OUTCOME(entry);
    }

    size_t moveCount;
    int *moves = state->getMoves(p, n, &moveCount);

    if (moveCount == 0) {
        memcpy(entry, state->board, boardSize);
        PLAYER(entry) = p;
        OUTCOME(entry) = n;
        // node_count += 1;
        return n;
    }

    char undoBuffer[sizeof(int) + 2 * sizeof(char)];

    for (size_t i = 0; i < moveCount; i++) {
        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        state->play(from, to, undoBuffer);
        node_count += 1;
        int result = solve(state, n, p);
        state->undo(undoBuffer);

        if (result == p) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = p;

            delete[] moves;
            return p;
        }
    }

    memcpy(entry, state->board, boardSize);
    PLAYER(entry) = p;
    OUTCOME(entry) = n;

    delete[] moves;
    return n;
}

char *BasicSolver::getTablePtr(int code) {
    int idx = code & bitMask;
    for (int i = 1; ; i++) {
        int shift = i * codeLength;

        if (shift >= sizeof(int) * 8) {
            break;
        }

        int add = code >> shift;
        add &= bitMask;
        code += add;
    }
    code &= bitMask;

    return table + (idx * tableEntrySize);
}
