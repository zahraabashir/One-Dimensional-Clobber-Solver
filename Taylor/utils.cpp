#include "utils.h"
#include <iostream>

#define DEBUG_OUTPUT

int opponentNumber(int n) {
    switch (n) {
        case BLACK:
            return WHITE;
            break;
        
        case WHITE:
            return BLACK;
            break;
    }

    #if defined(DEBUG_OUTPUT)
    std::cout << "Bad input to opponentNumber(): " << n << std::endl;
    #endif
    return EMPTY;
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
    #if defined(DEBUG_OUTPUT)
        std::cout << "Bad input to opponentChar(): " << c << std::endl;
    #endif

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

    #if defined(DEBUG_OUTPUT)
    std::cout << "Bad input to playerNumberToChar(): " << n << std::endl;
    #endif
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

    #if defined(DEBUG_OUTPUT)
    std::cout << "Bad input to charToPlayerNumber(): " << c << std::endl;
    #endif
    return EMPTY;
}

