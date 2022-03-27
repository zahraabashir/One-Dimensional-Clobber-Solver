#ifndef colors_H
#define colors_H

#include <cassert>

const int BLACK = 0;
const int WHITE = 1;
const int EMPTY = 2;

const char ColorCode[] = {'X', 'o', '.'};

// Map from characters '0' and '1' to X and o
inline char mapToChar(int c)
{
    return ColorCode[c - '0'];
}
//---------------------------------------------------------------------------

inline bool IsBlackWhite(int c)
{
    return c == BLACK || c == WHITE;
}

inline void assertbw(int color)
{
    assert(IsBlackWhite(color));
}

inline int Opp(int c)
{
    assertbw(c);
    return BLACK + WHITE - c;
}

//---------------------------------------------------------------------------

#endif // colors_H
