#include "state.h"

#include "utils.h"
#include <iostream>


/*
class State {




    State();
    ~State();
    int code();
    bool operator==(const State &s);
    State *play(int from, int to);
    bool isTerminal();
    void expand();


};

std::ostream &operator<<(std::ostream &os, const State &s);
*/

void State::zeroPointers() {
    children = NULL;
    moves = NULL;
    moveCount = (size_t) -1;
    outcome = EMPTY;
}

State::State() {
    zeroPointers();
}


State::State(std::string board, int playerNumber) {
    zeroPointers();
    this->board = board;
    this->playerNumber = playerNumber;
    this->playerChar = playerNumberToChar(playerNumber);
}

State::~State() {
    if (children != nullptr) {
        for (size_t i = 0; i < moveCount; i++) {
            children[i].reset();
        }
        delete[] children;
    }

    if (moves != nullptr) {
        delete[] moves;
    }
}

int State::code() {
    int result = 0;
    int cumulativePower = 1;
    for (size_t i = 0; i < board.length(); i++) {
        result += cumulativePower * charToPlayerNumber(board[i]);
        cumulativePower *= 3;
    }
    return result;
}

bool State::operator==(const State &s) {
    return board == s.board;
}


State *State::play(int from, int to) {
    State *s = new State();
    s->board = board;


    s->board[to] = s->board[from];
    s->board[from] = '.';

    s->playerNumber = opponentNumber(playerNumber);
    s->playerChar = opponentChar(playerChar);

    return s;
}

const int *State::getMoves() {
    if (moveCount != (size_t) -1) {
        return moves;
    }

    generateMoves();
    return moves;
}


void State::generateMoves(int idx, int moveDepth) {
    if (moveCount != (size_t) -1) {
        return;
    }

    if (idx > board.length()) {
        if (moveDepth > 0) {
            moves = new int[moveDepth * 2];
        }
        moveCount = moveDepth;
        return;
    }

    char *c2 = &board[idx];
    bool move1 = false;
    bool move2 = false;

    if (*c2 == playerChar) {
        char *c1 = &board[idx - 1];
        char *c3 = &board[idx + 1];
        move1 = idx > 0 && *c1 == opponentChar(playerChar);
        move2 = *c3 == opponentChar(playerChar);
    }

    generateMoves(idx + 1, moveDepth + move1 + move2);

    if (move1) {
        moves[moveDepth * 2] = idx;
        moves[moveDepth * 2 + 1] = idx - 1;
    }
    if (move2) {
        moves[(moveDepth + move1) * 2] = idx;
        moves[(moveDepth + move1) * 2 + 1] = idx + 1;
    }
}

void State::expand() {
    generateMoves();
    if (moveCount == 0 || children != nullptr) {
        return;
    }

    children = new std::shared_ptr<State>[moveCount];

    for (size_t i = 0; i < moveCount; i++) {
        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        State *s = play(from, to);
        children[i].reset(s);
    }
}

bool State::isTerminal() {
    generateMoves();
    return moveCount == 0;
}


std::ostream &operator<<(std::ostream &os, const State &s) {
    os << "[" << s.board << " " << s.playerChar << "]";
    return os;
}
