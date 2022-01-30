#pragma once

#include "state.h"


class BasicSolver {
  public:
    int rootPlayer;
    int rootOpponent;

    BasicSolver(int rootPlayer);
    int solveOr(State *state);
    int solveAnd(State *state);

};





