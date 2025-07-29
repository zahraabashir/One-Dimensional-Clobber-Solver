#include "state.h"

#include "utils.h"
#include "zobrist.h"
#include <iostream>
#include <cstring>
#include <algorithm>



int *_getMoves(const uint8_t *board, size_t len, int player, int opponent, size_t *moveCount, size_t idx, size_t moveDepth) {
    if (idx >= len) {
        if (moveDepth > 0) {
            *moveCount = moveDepth;
            return new int[moveDepth * 2];
        }
        *moveCount = 0;
        return nullptr;
    }

    const uint8_t *c2 = &board[idx];
    bool move1 = false;
    bool move2 = false;

    if (*c2 == player) {
        const uint8_t *c1 = &board[idx - 1];
        const uint8_t *c3 = &board[idx + 1];
        move1 = idx > 0 && *c1 == opponent;
        move2 = (idx + 1 < len) && *c3 == opponent;
    }

    int *buffer = _getMoves(board, len, player, opponent, moveCount, idx + 1, moveDepth + move1 + move2);

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

int *getMoves(const uint8_t *board, size_t len, int player, size_t *moveCount) {
    return _getMoves(board, len, player, opponentNumber(player), moveCount, 0, 0);
}

size_t _getMoveCount(const uint8_t *board, size_t len, int player, int opponent, size_t idx, size_t moveDepth) {
    if (idx >= len) {
        if (moveDepth > 0) {
            return moveDepth;
        }
        return 0;
    }

    const uint8_t *c2 = &board[idx];
    bool move1 = false;
    bool move2 = false;

    if (*c2 == player) {
        const uint8_t *c1 = &board[idx - 1];
        const uint8_t *c3 = &board[idx + 1];
        move1 = idx > 0 && *c1 == opponent;
        move2 = (idx + 1 < len) && *c3 == opponent;
    }

    return _getMoveCount(board, len, player, opponent, idx + 1, moveDepth + move1 + move2);
}

size_t getMoveCount(const uint8_t *board, size_t len, int player) {
    return _getMoveCount(board, len, player, opponentNumber(player), 0, 0);
}



//#include "MurmurHash3.h"
uint64_t getCode(uint8_t *board, size_t len, int player) {
    ///*

    return getZobristHash(player, board, len);
    //uint32_t val;
    //MurmurHash3_x86_32(board, len, player + 84654894654, &val);
    //return val;
    //*/

    /*
    assert(player == 1 || player == 2);
    int result = 1 * (player - 1);
    int cumulativePower = 2;

    for (size_t i = 0; i < len; i++) {
        result += cumulativePower * board[i];
        cumulativePower *= 3;
    }
    return result;
    */
}

//uint32_t getHash2(uint8_t *board, size_t len, int player) {
//    return getZobristHash(player, board, len);
//    //uint32_t val;
//    //MurmurHash3_x86_32(board, len, player + 654651298435, &val);
//    //return val;
//}



void play(uint8_t *board, uint8_t *undoBuffer, int from, int to) {
    assert(board[to] != 0);
    assert(board[from] != 0);
    assert(board[from] != board[to]);

    int minIdx = std::min<int>(from, to);
    ((int *) undoBuffer)[0] = minIdx;
    ((uint8_t *) undoBuffer)[0 + sizeof(int)] = board[minIdx];
    ((uint8_t *) undoBuffer)[0 + sizeof(int) + 1] = board[minIdx + 1];
    board[to] = board[from];
    board[from] = EMPTY;
}

void undo(uint8_t *board, uint8_t *undoBuffer) {
    int start = ((int *) undoBuffer)[0];
    board[start] = (uint8_t) undoBuffer[0 + sizeof(int)];
    board[start + 1] = (uint8_t) undoBuffer[0 + sizeof(int) + 1];
}


