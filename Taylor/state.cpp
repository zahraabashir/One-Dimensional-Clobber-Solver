#include "state.h"

#include "utils.h"
#include <iostream>
#include <cstring>

void State::zeroPointers() {
    board = nullptr;
    children = nullptr;
    moves = nullptr;
    moveCount = (size_t) -1;
    outcome = EMPTY;
}

void State::generateMoves(int idx, int moveDepth) {
    if (moveCount != (size_t) -1) {
        return;
    }

    if (idx > boardSize) {
        if (moveDepth > 0) {
            moves = new int[moveDepth * 2];
        }
        moveCount = moveDepth;
        return;
    }

    int *c2 = &board[idx];
    bool move1 = false;
    bool move2 = false;

    if (*c2 == playerNumber) {
        int *c1 = &board[idx - 1];
        int *c3 = &board[idx + 1];
        move1 = idx > 0 && *c1 == opponentNumber(playerNumber);
        move2 = *c3 == opponentNumber(playerNumber);
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

State::State() {
    zeroPointers();
}

State::State(std::string board, int playerNumber) {
    zeroPointers();

    this->boardSize = board.length();
    this->board = new int[this->boardSize];
    for (int i = 0; i < this->boardSize; i++) {
        this->board[i] = charToPlayerNumber(board[i]);
    }

    this->playerNumber = playerNumber;
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

    if (board != nullptr) {
        delete[] board;
    }
}

int State::code() {
    int result = 0;
    int cumulativePower = 1;
    for (size_t i = 0; i < boardSize; i++) {
        result += cumulativePower * board[i];
        cumulativePower *= 3;
    }
    return result;
}

bool State::operator==(const State &s) {
    if (boardSize == s.boardSize) {
        for (int i = 0; i < boardSize; i++) {
            if (board[i] != s.board[i]) {
                return false;
            }
        }
        return true;
    }

    return false;
}

State *State::play(int from, int to) {
    State *s = new State();

    s->board = new int[boardSize];
    memcpy(s->board, board, sizeof(int) * boardSize);

    s->board[to] = s->board[from];
    s->board[from] = EMPTY;

    s->boardSize = boardSize;
    s->playerNumber = opponentNumber(playerNumber);

    return s;
}

const int *State::getMoves() {
    if (moveCount != (size_t) -1) {
        return moves;
    }

    generateMoves();
    return moves;
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
    os << "[" << s.board << " " << playerNumberToChar(s.playerNumber) << "]";
    return os;
}
