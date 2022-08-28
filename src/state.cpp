#include "state.h"

#include "utils.h"
#include <iostream>
#include <cstring>
#include <algorithm>

int *State::generateMoves(const int &player, const int &opponent, size_t *moveCount, int idx, int moveDepth) {
    if (idx >= boardSize) {
        if (moveDepth > 0) {
            *moveCount = moveDepth;
            return new int[moveDepth * 2];
        }
        *moveCount = 0;
        return nullptr;
    }

    char *c2 = &board[idx];
    bool move1 = false;
    bool move2 = false;

    if (*c2 == player) {
        char *c1 = &board[idx - 1];
        char *c3 = &board[idx + 1];
        move1 = idx > 0 && *c1 == opponent;
        move2 = (idx + 1 < boardSize) && *c3 == opponent; //check probably not needed -- null terminator
    }

    int *buffer = generateMoves(player, opponent, moveCount, idx + 1, moveDepth + move1 + move2);

    if (move1) {
        buffer[moveDepth * 2] = idx;
        buffer[moveDepth * 2 + 1] = idx - 1;
    }
    if (move2) {
        buffer[(moveDepth + move1) * 2] = idx;
        buffer[(moveDepth + move1) * 2 + 1] = idx + 1;
    }

    return buffer;
}

State::State() {
}

State::State(std::string board, int player) {
    this->boardSize = board.length();
    this->board = new char[this->boardSize];
    //this->player = player;

    for (int i = 0; i < this->boardSize; i++) {
        this->board[i] = charToPlayerNumber(board[i]);
    }

}

State::~State() {
    if (this->boardSize != 0) {
        delete[] board;
    }
}

#include <md4.h>
#define USEMD

#if defined(USEMD)

int State::code(int player) {
    uint8_t data[boardSize + 1];

    for (int i = 0; i < boardSize; i++) {
        data[i] = board[i] + 1;
    }
    data[boardSize] = player;

    MD4_CTX ctx;
    MD4Init(&ctx);
    MD4Update(&ctx, data, boardSize + 1);

    uint8_t hash[MD4_DIGEST_LENGTH];

    MD4Final(hash, &ctx);

    return *((int *) hash);
}

#else

int State::code(int player) {
    int result = 1 * (player - 1);
    int cumulativePower = 2;

    for (size_t i = 0; i < boardSize; i++) {
        result += cumulativePower * board[i];
        cumulativePower *= 3;
    }
    return result;
}

#endif



/*
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
*/

void State::play(int from, int to, char *undoBuffer) {
    int minIdx = std::min<int>(from, to);
    ((int *) undoBuffer)[0] = minIdx;
    ((char *) undoBuffer)[0 + sizeof(int)] = board[minIdx];
    ((char *) undoBuffer)[0 + sizeof(int) + 1] = board[minIdx + 1];

    board[to] = board[from];
    board[from] = EMPTY;
}

void State::undo(char *undoBuffer) {
    int start = ((int *) undoBuffer)[0];
    board[start] = (char) undoBuffer[0 + sizeof(int)];
    board[start + 1] = (char) undoBuffer[0 + sizeof(int) + 1];
}

int *State::getMoves(const int &player, const int &opponent, size_t *moveCount) {
    return generateMoves(player, opponent, moveCount, 0, 0);
}

std::ostream &operator<<(std::ostream &os, const State &s) {
    //os << "[" << s.board << " " << playerNumberToChar(s.player) << "]";
    os << "[" << s.board << " " << "]";
    return os;
}
