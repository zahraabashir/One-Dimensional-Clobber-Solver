#include <ostream>

#define EMPTY 0
#define BLACK 1
#define WHITE 2

#define E 0
#define B 1
#define W 2

int opponentNumber(int);
char opponentChar(char);
char playerNumberToChar(int);
int charToPlayerNumber(char);


template <class T1, class T2>
std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &p) {
    os << "(" << p.first << " " << p.second << ")";
    return os;
}
