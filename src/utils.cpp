#include "utils.h"
#include <iostream>
#include <vector>
#include <cstring>
#include "database3.h"

using namespace std;

int playerSign(int p) {
    return p == 1 ? 1 : -1;
}

char opponentChar(char c) {
    switch (c) {
        case 'B':
            return 'W';
            break;

        case 'W':
            return 'B';
            break;
    }

    assert(c == '.');
    return '.';
}

char playerNumberToChar(int n) {
    switch (n) {
        case BLACK:
            return 'B';
            break;

        case WHITE:
            return 'W';
            break;
    }

    assert(n == 0);
    return '.';
}

int charToPlayerNumber(char c) {
    switch (c) {
        case 'B':
            return BLACK;
            break;

        case 'W':
            return WHITE;
            break;
    }

    assert(c == '.');
    return EMPTY;
}


void negateBoard(uint8_t *board, size_t length) {
    for (size_t i = 0; i < length; i++) {
        board[i] = opponentNumber(board[i]);
    }
}

void printBoard(const uint8_t *board, size_t len, bool newline) {
    //cout << "\"";
    for (size_t i = 0; i < len; i++) {
        cout << playerNumberToChar(board[i]);
    }
    //cout << "\"";

    if (newline) {
        cout << endl;
    }
}


void makeGame(uint64_t shapeNumber, uint32_t gameNumber, 
        uint8_t **board, size_t *len) {
    makeGame(numberToShape(shapeNumber), gameNumber, board, len);
}

void makeGame(const vector<int> &shape, uint32_t gameNumber, 
        uint8_t **board, size_t *len) {

    *len = 0;


    if (shape.size() == 0) {
        *board = new uint8_t[0];
        return;
    }

    *len += shape.size() - 1;
    for (int chunk : shape) {
        *len += chunk;
    }

    *board = new uint8_t[*len];

    int shift = 0;
    int chunkIdx = 0;
    int chunkOffset = 0;


    for (int i = 0; i < *len; i++) {
        int chunk = shape[chunkIdx];

        if (chunkOffset < chunk) {
            (*board)[i] = ((gameNumber >> shift) & 1) + 1;
            shift += 1;
            chunkOffset += 1;
        } else {
            (*board)[i] = 0;
            chunkOffset = 0;
            chunkIdx += 1;
        }
    }
}

uint8_t *addGames(uint8_t *g1, size_t g1Size,
    uint8_t *g2, size_t g2Size, size_t *newSize) {


    bool hasSpace = false;
    if (g1Size != 0 && g2Size != 0) {
        hasSpace = true;
    }

    *newSize = g1Size + g2Size + hasSpace;

    uint8_t *result = new uint8_t[*newSize];

    for (size_t i = 0; i < *newSize; i++) {
        if (i < g1Size) {
            result[i] = g1[i];
            continue;
        }

        if (i == g1Size && hasSpace) {
            result[i] = 0;
            continue;
        }

        result[i] = g2[i - g1Size - hasSpace];
    }

    return result;
}
