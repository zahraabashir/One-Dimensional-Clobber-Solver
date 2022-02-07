#include "solver.h"
#include "utils.h"

#include <iostream>
#include <cstdlib>
#include <cstring>

#define BOARD(te) te
#define PLAYER(te) *(te + boardSize)
#define OUTCOME(te) *(te + boardSize + 1)
int node_count = 0;
int best_from = 0;
int best_to = 0;

BasicSolver::BasicSolver(int rootPlayer, int boardSize) {
    this->rootPlayer = rootPlayer;
    this->rootOpponent = opponentNumber(rootPlayer);
    this->boardSize = boardSize;

    int bits = 24;
    codeLength = bits;
    //board, player, outcome
    tableEntrySize = boardSize + 2;

    bitMask = 0;
    for (int i = 0; i < bits; i++) {
        bitMask <<= 1;
        bitMask |= 1;
    }

    table = (char *) calloc((1 << bits) * 1 * tableEntrySize, 1);
}

BasicSolver::~BasicSolver() {
    free(table);
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

//char *BasicSolver::getTablePtr(int code) {
//    int idx = code & bitMask;
//    return table + (idx * tableEntrySize);
//}

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
