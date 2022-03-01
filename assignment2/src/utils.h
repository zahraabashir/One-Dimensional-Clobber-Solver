#pragma once

#include <ostream>

#define EMPTY 0
#define BLACK 1
#define WHITE 2

#define E 0
#define B 1
#define W 2

//Return number of opposing player (1 --> 2), (2 --> 1)
inline int opponentNumber(int n) { //Inline means this needs to be in the header
    return (n % 2) + 1;
}

// (1 --> 1), (2 --> -1)
int playerSign(int p);

// ('B' --> 'W'), ('W' --> 'B')
char opponentChar(char c);

// (0 --> '.'), (1 --> 'B'), (2 --> 'W')
char playerNumberToChar(int n);

// ('.' --> 0), ('B' --> 1), ('W' --> 2)
int charToPlayerNumber(char c);

//ostream operator for std::pair
template <class T1, class T2>
std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &p) {
    os << "(" << p.first << " " << p.second << ")";
    return os;
}


