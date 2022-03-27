#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "bits.h"
#include "clobbercache.h"
#include "colors.h"
#include "game.h"
#include "sumgame.h"

using std::cout;
using std::flush;
using std::vector;
using std::string;
//---------------------------------------------------------------------------

const bool DEBUG_PRINT = false;
//---------------------------------------------------------------------------

sumgame parseInput(string input)
{
    int size = input.size();
    int start = 0;
    int end = input.find('.');
    int numSubgames = 0;
    sumgame output;
    while(true)
    {
        string currentGame = input.substr(start, end - start);
        
        bits bitmap = 0;
        for(int i = 0; i < currentGame.size(); ++i)
        {
            int colour;
            // cout << currentGame[i] << "\n";
            assert(currentGame[i] == 'B' || currentGame[i] == 'W');
            if(currentGame[i] == 'B')
                colour = BLACK;
            else
                colour = WHITE;
            bitmap = (bitmap << 1) + colour;
        }
        output.add(game(bitmap, currentGame.size()));
        start = end + 1;
        if(end == -1)
            break;
        end = input.find('.', start);
    }

    return output;
}

bool compute(int size, int color)
{
    sumgame s(game::bwgame(size));
    s.setToPlay(color);
    clock_t t = clock();
    bool result = s.negamaxBoolean();
    t = clock() - t;
    cout << "\nSize " << size
         << ' ' << ColorCode[color] << " to play "
         //<< " Sumgame " << s
         << " Result "  << std::boolalpha << result 
         << " Time " << t << " sec " << t/CLOCKS_PER_SEC
         << std::endl;
    return result;
}

int main(int argc, char** argv)
{       
    //testGenerator();

    initVisitOrder();
    //printVisitOrder();
    
    clobberCache::computeCaches();
    clobberCache::computeStars();

    // Snippet of original test code
    // sumgame testgame(game(2,3));
    // testgame.add(game(3,3));
    // testgame.setToPlay(BLACK);
    // cout << "test " << testgame << "\n";
    // game g(2,3);
    // g.compute();
    // cout << "before " << g;
    // clobberCache::tryLookupResult(g);
    // cout << " after " << g << '\n';
    // return 0;

    // IO for CMPUT 655 assignment 1
    ////////////////////////////////////////////////////
    //sumgame testgame = parseInput("BW.WBW.BWBWB");
    sumgame solveThis = parseInput(argv[1]);
    int inputLength = string(argv[1]).length();
    
    int toPlay;
    if(string(argv[2]).size() == 1)
    {
        if(argv[2][0] == 'B')
            toPlay = BLACK;
        else if(argv[2][0] == 'W')
            toPlay = WHITE;
    }
    solveThis.setToPlay(toPlay);
    
    // solve now
    clock_t t = clock();
    // cout << solveThis << std::endl;
    bool result = solveThis.negamaxBoolean();
    t = clock() - t;
    string winner("BW");
    int index = result ? toPlay : Opp(toPlay);
    move bestMove = solveThis.getPV();
    cout << winner[index] << " ";
    if(result)
        cout << (inputLength - 1) - bestMove.from << "-" << (inputLength - 1) - bestMove.to << " ";
    else
        cout << "None ";
    cout << std::fixed << std::setprecision(4) << float(t)/float(CLOCKS_PER_SEC) << " "
         << solveThis.nodesSearched()
         << std::endl;
    return 0;
    ////////////////////////////////////////////////////
    
    // Sample program to solve BW game up to MAX_SIZE
    // for (int size = 2; size <= MAX_SIZE; ++size) //5 MAX_SIZE
    // {
    //     bool resultBlack = compute(size, BLACK);
    //     bool resultWhite;
    //     if (size % 2 == 0) // symmetric position, result for B and W will be the same
    //         resultWhite = resultBlack;
    //     else
    //         resultWhite = compute(size, WHITE);
    //     if (resultBlack) 
    //         if (resultWhite) cout << "First player win\n";
    //         else cout << "Black win\n";
    //     else
    //         if (resultWhite) cout << "White win\n";
    //         else cout << "Second player win\n";
    // }
    return 0;
}

