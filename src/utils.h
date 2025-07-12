#pragma once

#include "options.h"

#include <ostream>
#include <type_traits>
#include <vector>
#include <cassert>
#include <cstdint>
#include <chrono>

#define EMPTY 0
#define BLACK 1
#define WHITE 2

#define E 0
#define B 1
#define W 2

//#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define DBOUT( x ) x
#else
#define DBOUT( x )
#endif


//Return number of opposing player (1 --> 2), (2 --> 1)
inline int opponentNumber(int n) { //Inline means this needs to be in the header
    if (n == 1) {
        return 2;
    }
    if (n == 2) {
        return 1;
    }

    assert(n == 0);
    return n;
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

template <class T>
int sign(const T &val) {
    return T(0) > val ? -1 : 1;
}

template <class T>
int sumBits(T val) {
    static_assert(std::is_unsigned_v<T>);

    int sum = 0;
    
    while (val != 0) {
        sum += (val & T(1));
        val >>= 1;
    }

    return sum;
}

template <class T1, class T2, class T3>
struct triple {
    T1 first;
    T2 second;
    T3 third;

    triple(const T1 &_first, const T2 &_second, const T3 &_third): 
        first(_first), second(_second), third(_third) {
    }
};

template <class T1, class T2, class T3>
bool operator<(const triple<T1, T2, T3> &t1, const triple<T1, T2, T3> &t2) {
    if (t1.first < t2.first) {
        return true;
    }
    if (t1.first > t2.first) {
        return false;
    }

    if (t1.second < t2.second) {
        return true;
    }
    if (t1.second > t2.second) {
        return false;
    }

    if (t1.third < t2.third) {
        return true;
    }
    if (t1.third > t2.third) {
        return false;
    }

    return false;
}

void negateBoard(uint8_t *board, size_t length);

template <class T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec) {
    os << "[";
    for (int i = 0; i < vec.size(); i++) {
        os << vec[i];
        if (i + 1 < vec.size()) {
            os << ", ";
        }
    }
    os << "]";

    return os;
}

template <>
inline std::ostream &operator<<(std::ostream &os, const std::vector<uint8_t> &vec) {
    os << '[';

    for (int i = 0; i < vec.size(); i++)
        os << playerNumberToChar(vec[i]);

    os << ']';

    return os;
}




void printBoard(const uint8_t *board, size_t len, bool newline = false);


void makeGame(uint64_t shapeNumber, uint32_t gameNumber, 
    uint8_t **board, size_t *len);

void makeGame(const std::vector<int> &shape, uint32_t gameNumber, 
    uint8_t **board, size_t *len);

uint8_t *addGames(uint8_t *g1, size_t g1Size,
    uint8_t *g2, size_t g2Size, size_t *newSize);



inline uint64_t msSinceEpoch()
{
    using namespace std::chrono;

    milliseconds t =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return t.count();
}

