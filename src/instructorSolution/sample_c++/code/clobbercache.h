#ifndef clobbercache_H
#define clobbercache_H

#include "bits.h"
#include "game.h"

typedef std::pair<game, const game*> entry;

class clobberCache
{
public:
    #define CACHE_ENTRY_SIZE (13)
    // clobber strips up to CACHE_ENTRY_SIZE long, no empties

    static void computeCaches();
    static void computeStars();
    static const game* lookup(int size, bits code);  
    static const game* lookup(const game& g);
    
    /** If result found in cache: store in game **/
    static void tryLookupResult(game& g);

private:
    static void printStats();
    static entry* sClobberCache[CACHE_ENTRY_SIZE];
    static int sComputedUpTo;
};

#endif // clobbercache_H
