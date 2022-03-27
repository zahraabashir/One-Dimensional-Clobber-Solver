#ifndef bits_H
#define bits_H

#include <algorithm>    // std::transform
#include <bitset>
#include <string>
#include "colors.h"

#define MAX_SIZE (40)

#define bits uint_least64_t

//---------------------------------------------------------------------------
inline bits bit(int index)
{
    return 1l << index;
}

inline int testBit(bits bitmap, int index)
{
    return !!(bitmap & bit(index)); // !! reduces anything >0 to 1.
}

inline bits setBit(bits bitmap, int index)
{
    return bitmap | bit(index);
}

inline bits mask(int size)
{
    return (1l << size) - 1;
}

inline bits reverse(bits code, int size) // naive reverse
{
    bits r = 0;
    for (int i = 0, j = size - 1; i < size; ++i, --j)
        if (testBit(code, i))
            r = setBit(r, j);
    return r;
}

inline std::string printBitset(bits code, int size)
{
    std::string str01 = std::bitset<MAX_SIZE>(code).to_string()
                                                   .substr(MAX_SIZE - size);
    std::string result;
    result.resize(str01.size()); // allocate space
    std::transform(str01.begin(), str01.end(), result.begin(), mapToChar);
    return result;
}

inline std::ostream& bitsStream(std::ostream& out, bits code, int size)
{
    out << printBitset(code, size);
    return out;
}

//---------------------------------------------------------------------------

#endif // bits_H
