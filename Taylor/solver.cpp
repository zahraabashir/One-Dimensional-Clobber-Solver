#include "solver.h"
#include "utils.h"


/*
    BasicSolver(int rootPlayer);
    bool solveOr(State *state);
    bool solveAnd(State *state);
*/

BasicSolver::BasicSolver(int rootPlayer) {
    this->rootPlayer = rootPlayer;
    this->rootOpponent = opponentNumber(rootPlayer);
}


int BasicSolver::solveOr(State *state) {
    if (state->isTerminal()) {
        state->outcome = opponentNumber(state->playerNumber);
        return state->outcome;
    }

    state->expand();

    for (size_t i = 0; i < state->moveCount; i++) {
        int result = solveAnd(state->children[i].get());

        if (result == rootPlayer) {
            state->outcome = rootPlayer;
            return rootPlayer;
        }
    }
    state->outcome = rootOpponent;
    return rootOpponent;
}

int BasicSolver::solveAnd(State *state) {
    if (state->isTerminal()) {
        state->outcome = opponentNumber(state->playerNumber);
        return state->outcome;
    }

    state->expand();

    for (size_t i = 0; i < state->moveCount; i++) {
        int result = solveOr(state->children[i].get());

        if (result == rootOpponent) {
            state->outcome = rootOpponent;
            return rootOpponent;
        }
    }
    state->outcome = rootPlayer;
    return rootPlayer;
}

