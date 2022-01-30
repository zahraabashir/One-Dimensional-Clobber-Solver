#pragma once

#include <ostream>

#define EMPTY 0
#define BLACK 1
#define WHITE 2

#define E 0
#define B 1
#define W 2

int opponentNumber(int n);
char opponentChar(char c);
char playerNumberToChar(int n);
int charToPlayerNumber(char c);

template <class T1, class T2>
std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &p) {
    os << "(" << p.first << " " << p.second << ")";
    return os;
}
