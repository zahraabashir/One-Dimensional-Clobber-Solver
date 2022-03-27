#include "clobbercache.h"
#include <cassert>
#include "game.h"
#include "sumgame.h"

using std::cout;
//---------------------------------------------------------------------------

const bool DEBUG_PRINT_CACHE = false;

// smallest size is 2, stored at index 0
const int OFFSET = 2;
//---------------------------------------------------------------------------

entry* clobberCache::sClobberCache[CACHE_ENTRY_SIZE];
int clobberCache::sComputedUpTo = 0;

//---------------------------------------------------------------------------

const game* clobberCache::lookup(const game& g)
{
    return lookup(g.size(), g.bitmap());
}

const game* clobberCache::lookup(int size, bits bitmap)
{
    assert(bitmap < (1l << size));
    if (size <= sComputedUpTo)
    {
        const entry& e = sClobberCache[size - OFFSET][bitmap];
        //return e.second? e.second : &e.first;
        if (e.second)
        {
            // if ((size % 2 == 0) || (size - e.second->size() > 5))
            //     cout << "Replace size " << size << " by " << *e.second << '\n'
            //          << std::flush;
            return e.second;
        }
        return &e.first;
    }
    return nullptr;
}

void clobberCache::tryLookupResult(game& g)
{
    const game* entry = lookup(g);
    if (entry)
    {
        g = *entry;
    }
}

void clobberCache::computeCaches()
{
    for (int index = 0; index < CACHE_ENTRY_SIZE; sComputedUpTo = index, ++index)
    {
        const int size = index + OFFSET;
        if (DEBUG_PRINT_CACHE)
            cout << "Compute cache for size " << size << '\n';
        sClobberCache[index] = new entry[1 << size];
        entry* const cacheLine = sClobberCache[index];
        for (int i = 0; i < (1l << size); ++i)
        {
            cacheLine[i].first = game(i, size);
            assert(cacheLine[i].second == nullptr);
            cacheLine[i].first.compute();
            if (DEBUG_PRINT_CACHE && size < 0)
                cout << "Entry " << i << ' ' 
                     << cacheLine[i].first << '\n';
        }
    }
}

bool checkZero(const game& g, const game& g2)
{
    sumgame s(g);
    s.add(g2);
    s.setToPlay(BLACK);
    if (s.negamaxBoolean())
        return false;
    s.setToPlay(WHITE);
    if (! s.negamaxBoolean())
    {
        //cout << "zero sum: " << s << '\n';
        return true;
    }
    return false;
}


void clobberCache::printStats()
{
    int count = 0;
    for (int index = 0; index <= sComputedUpTo; ++index)
    {
        const int size = index + OFFSET;
        const entry* const cacheLine = sClobberCache[index];
        for (int i = 0; i < (1l << size); ++i)
            if (cacheLine[i].second)
                ++count;
    }
    cout << count<< " simplified games\n";
}

void clobberCache::computeStars()
{
    const game& star = sClobberCache[2 - OFFSET][1].first;
    const game& up = sClobberCache[3 - OFFSET][1].first;
    const game& down = sClobberCache[3 - OFFSET][3].first;
    assert(up.isPositive());
    assert(down.isNegative());
    
    //cout << "star " << star << '\n';
    //cout << "up " << up << '\n';
    //cout << "down " << down << "\n\n";
    
    for (int index = 0; index <= sComputedUpTo; ++index)
    {
        const int size = index + OFFSET;
        if (DEBUG_PRINT_CACHE)
            cout << "Stars for board size " << size << '\n';
        entry* const cacheLine = sClobberCache[index];
        
        int zeroes = 0;
        int stars = 0;
        int ups = 0;
        int downs = 0;
        
        for (int i = 0; i < (1l << size); ++i)
        {
            const game& g = cacheLine[i].first;
            assert (g.isComputed());
            if (g.isZero())
                ++zeroes;
            else if (g.isNextPlayerWin())
            {
                if (checkZero(g, star))
                {
                    ++stars;
                    if (size > 2)
                        cacheLine[i].second = &star;
                }
            }
            else if (g.isPositive())
            {
                if (checkZero(g, down))
                {
                    ++ups;
                    if (size > 3)
                        cacheLine[i].second = &up;
                }
            }
            else
            {
                assert(g.isNegative());
                if (checkZero(g, up))
                {
                    ++downs;
                    if (size > 3)
                        cacheLine[i].second = &down;
                }
            }
        }
        if (DEBUG_PRINT_CACHE)
            cout << (1l << size) << " total "
                 << zeroes << " zeroes "
                 << stars << " stars "
                 << ups << " ups "
                 << downs << " downs "
                 << ((1l << size) - zeroes - stars - ups - downs)
                 << " other\n";
    }
    // printStats();
}

