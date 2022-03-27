#include "game.h"

#include <cassert>
#include "colors.h"
#include "clobbercache.h"
#include "sumgame.h"

using std::cout;
using std::vector;

//---------------------------------------------------------------------------
namespace {
int visitOrder[MAX_SIZE+1][MAX_SIZE+1]; // try center-first strategy
}

void printVisitOrder()
{
    for (int size = 1; size <= MAX_SIZE; ++size)
    {
        cout << "\nSize " << size << ":";
        for (int index = 0; index < size; ++index)
            cout << " " << visitOrder[size][index];
    }
}
void initVisitOrder()
{
    for (int size = 1; size <= MAX_SIZE; ++size)
    {
        int mid = size/2;
        visitOrder[size][0] = mid;
        int i=1;
        for (int delta = 1; delta <= mid; ++delta)
        {
            assert(mid - delta >= 0);
            visitOrder[size][i] = mid - delta;
            ++i;
            if (mid + delta < size)
            {
                visitOrder[size][i] = mid + delta;
                ++i;
            }
        }
    }
}

//---------------------------------------------------------------------------

game game::bwgame(int size) // alternating string "bwbw..."
{
    bits bitmap = 0;
    for (int color = BLACK, i=0; i < size; color = Opp(color), ++i)
        bitmap = (bitmap << 1) + color;
    return game(bitmap, size);

}

char game::gameStatus() const
{
    if (_blackWinComputed && _whiteWinComputed)
    {
        if (_blackWinGoingFirst)
            return _whiteWinGoingFirst ? 'F' : 'B';
        else
            return _whiteWinGoingFirst ? 'W' : '0';
    }
    else
        return '?'; // unknown
}

void game::compute()
{
    assert(! _blackWinComputed);
    assert(! _whiteWinComputed);
    sumgame s(*this);
    s.setToPlay(BLACK);
    _blackWinGoingFirst = s.negamaxBoolean();
    s.setToPlay(WHITE);
    _whiteWinGoingFirst = s.negamaxBoolean();
    _blackWinComputed = _whiteWinComputed = true;
}

vector<game> game::play(const int from, const int to,
                        const int toPlay, const int opp) const
{
    assert(isActive());
    assert(isColor(from, toPlay));
    assertInRange(from);
    assertInRange(to);
    assert(from == to + 1 || from == to - 1);
    assert(isColor(to, opp));
    
    game g1, g2;
    vector<game> subgames;
    if (from < to) // move left
    {
        if (from > 1) // low part = 0...from-1, size = from; >1: single stones are always dead
            g1 = game(_bitMap & mask(from), from);
        if (to < _size - 1) // high part = to...size-1, 
            // ^1 since the to-square color is flipped and it is at index 0 in the new code
            g2 = game((_bitMap >> to) ^ 1, _size - to);
    }
    else // (from > to), move right
    {
        if (from > 1) // low part = 0...to, size = from; >1: single stones are always dead
            g1 = game((_bitMap & mask(from)) ^ bit(to), from); // flip to-bit
        if (_size - from > 2) // high part = from + 1...size-1
            g2 = game(_bitMap >> (from + 1), _size - from - 1);
    }
    
    // cout << "play g " << *this << " from " << from << " to " << to << '\n';
    if (! g1.isComputedZero() && ! g1.isSingleColor())
    {
        //cout << "g1 before " << g1;
        clobberCache::tryLookupResult(g1);
        //cout << " g1 after " << g1;
    }
    if (! g2.isComputedZero() && ! g2.isSingleColor())
    {
        //cout << "\ng2 before " << g2;
        clobberCache::tryLookupResult(g2);
        //cout << " g2 after " << g2 << std::endl;
    }
    
    if (! g1.isComputedZero() && ! g2.isComputedZero() && g1.isInverse(g2))
        return vector<game>(); // cancel each other    
    if (! g1.isComputedZero() && ! g1.isSingleColor())
    {
        subgames.push_back(g1);
        assert(g1.isActive());

    }
    if (! g2.isComputedZero() && ! g2.isSingleColor())
    {
        subgames.push_back(g2);
        assert(g2.isActive());
    }
    return subgames;    
}

std::string game::print() const
{
    return printBitset(_bitMap, _size);
}

std::ostream& operator<<(std::ostream& out, const game& g)
{
    out /*<< g.size() << ' '*/ 
    << g.print()
    //<< " Status " << g.gameStatus()  /*<< ' ' << g.isActive()*/
    ;
    return out;
}

//---------------------------------------------------------------------------
void testGenerator()
{
    game g = game::bwgame(2);
    {
    gameMoveGenerator gg(g, BLACK);
    assert(gg);
    assert(gg.from() == 1);
    assert(gg.to() == 0);
    ++gg;
    assert(! gg);
    }
    
    {
    gameMoveGenerator gg(g, WHITE);
    assert(gg);
    assert(gg.from() == 0);
    assert(gg.to() == 1);
    ++gg;
    assert(! gg);
    }
    
}
