#include "solver.h"
#include "utils.h"

BasicSolver::BasicSolver(int rootPlayer) {
    this->rootPlayer = rootPlayer;
    this->rootOpponent = opponentNumber(rootPlayer);
}

int BasicSolver::solveOr(State *state) {
    size_t moveCount;
    int *moves = state->getMoves(rootPlayer, rootOpponent, &moveCount);

    if (moveCount == 0) {
        return rootOpponent;
    }

    char undoBuffer[sizeof(int) + 2 * sizeof(char)];

    for (size_t i = 0; i < moveCount; i++) {
        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        state->play(from, to, undoBuffer);
        int result = solveAnd(state);
        state->undo(undoBuffer);

        if (result == rootPlayer) {
            delete[] moves;
            return rootPlayer;
        }
    }

    delete[] moves;
    return rootOpponent;
}


int BasicSolver::solveAnd(State *state) {
    size_t moveCount;
    int *moves = state->getMoves(rootOpponent, rootPlayer, &moveCount);

    if (moveCount == 0) {
        return rootPlayer;
    }

    char undoBuffer[sizeof(int) + 2 * sizeof(char)];

    for (size_t i = 0; i < moveCount; i++) {
        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        state->play(from, to, undoBuffer);
        int result = solveOr(state);
        state->undo(undoBuffer);

        if (result == rootOpponent) {
            delete[] moves;
            return rootOpponent;
        }
    }

    delete[] moves;
    return rootPlayer;
}


