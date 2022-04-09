#include "utils.h"
#include <iostream>

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

    return EMPTY;
}


size_t gameLength(size_t bufferSize, char *game) {
    size_t length = 0;

    for (size_t i = 0; i < bufferSize; i++) {
        if (game[i] != 0) {
            length = i;
        }
    }

    return length;
}
