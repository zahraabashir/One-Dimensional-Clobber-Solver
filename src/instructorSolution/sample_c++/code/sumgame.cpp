#include "sumgame.h"

#include <iostream>
#include "clobbercache.h"
#include "colors.h"
#include "game.h"

using std::cout;
using std::make_pair;
using std::pair;

std::ostream& operator<<(std::ostream& out, const sumgame& s)
{
    //out << "_subgames " << &s;
    for (const game & g: s._subgames)
    {
        if (g.isActive())
            out << g << ' '; // << &g << ' ';
    }

    /*
    out << "\n_gameRecord";
    for (const auto & item: s._gameRecord)
    {
        out << '\n' << item.first  << ' '
            << item.second << ' ';
    }
    */
    //out << '\n'; // << std::flush;
    return out;
}

sumgame::sumgame() : _toPlay(BLACK), _gameRecord(), _subgames()
{
    _subgames.reserve(100);
    _nodesSearched = 0;
}

sumgame::sumgame(const game& g) : _toPlay(BLACK), _gameRecord(), _subgames()
{
    _subgames.reserve(100);
    _subgames.push_back(g);
    if (! g.isComputed())
        clobberCache::tryLookupResult(_subgames.back());
}

void sumgame::setToPlay(int color)
{
    assertbw(color);
    _toPlay = color;
}

bool sumgame::findStaticWinner(bool& success) const
{
    bool hasPositive = false;
    bool hasNegative = false;
    bool hasNextPlayerWin = false;

    for (auto& g: _subgames)
    {
        if (g.isActive())
        {
            if (! g.isComputed()) // unknown subgame
                return false;
            assert(! g.isZero());
            if (g.isNextPlayerWin())
            {
                if (hasNextPlayerWin)
                    return false; // N+N = unknown
                else
                    hasNextPlayerWin = true;
            }
            else if (g.isPositive())
            {
                if (hasNegative)
                    return false; // L+R = unknown
                hasPositive = true;
            }
            else
            {
                assert(g.isNegative());
                if (hasPositive)
                    return false; // L+R = unknown
                hasNegative = true;
            }
        }
    }
    assert(! hasPositive || ! hasNegative);
    if (! (hasPositive || hasNegative || hasNextPlayerWin))
        return false; // empty sum, loss for toplay
        
    if (_toPlay == BLACK)
    {
        if (! hasNegative)
        {
            success = false; // negamax - opposite result
            return true;
        }
        else if (hasNextPlayerWin) // R+N = unknown
        {
            return false;
        }  
        else // ! hasNextPlayerWin, negative only
        {
            success = true;
            return true;            
        }  
    }
    else // _toPlay == WHITE
    {
        if (! hasPositive)
        {
            success = false;
            return true;
        }
        else if (hasNextPlayerWin) // L+N = unknown
        {
            return false;
        }  
        else // ! hasNextPlayerWin, positive only
        {
            success = true;
            return true;            
        }  
    }
}

bool sumgame::negamaxBoolean()
{
    static int i=0;
    sumRestoreChecker r(*this);
    assert(_gameRecord.size() <= 100); // **** what's the theoretical limit?
    ++i;
    ++_nodesSearched;
    if (i%100000 == 0)
    {
        // cout << *this << '\n';
        // cout << ".";
        // cout.flush();
    }
    for (auto& g: _subgames)
    {
        if (g.isActive())
        {
            for (gameMoveGenerator gg(g, _toPlay); gg; ++gg)
            {
                sumRestoreChecker r1(*this);
                play(g, gg.from(), gg.to());
                bool success = false;
                bool found = findStaticWinner(success);
                //if (found)
                //    cout << "findStaticWinner " << *this << success << '\n';
                if (! found)
                    success = not negamaxBoolean();
                undoMove();
                if (success)
                {
                    move pv;
                    pv.from = gg.from();
                    pv.to = gg.to();
                    _pv.push(pv);
                    return true;
                }
            }
        }
    }
    return false;
}

game* sumgame::findInverse(const game& candidate)
{
    for (auto &g: _subgames)
        if (g.isActive() && g.isInverse(candidate))
            return &g;
    return 0;
}

bool sumgame::findInactive(const game* candidate) const
{
    for (auto const &g: _subgames)
    {
        if (&g == candidate)
        {
            assert(! g.isActive());
            return true;
        }
    }
    return false;
}

const bool WRITE_SUMGAME_PLAY = false;
const int START_MARKER = 0;
const int ADD_MARKER = 1;
const int DEACTIVATE_MARKER = 2;

void sumgame::deactivate(game* g)
{
    if (WRITE_SUMGAME_PLAY)
        cout << "sumgame::deactivate " << g << '\n';
    assert(g->isActive());
    g->setActive(false);
    _gameRecord.push_back(make_pair(DEACTIVATE_MARKER, g));
}


void sumgame::add(game g)
{
    if (WRITE_SUMGAME_PLAY)
        cout << "Start sumgame::add " << *this << " add game " << g << '\n';
    _subgames.push_back(g);
    assert(g.isActive());
    _gameRecord.push_back(make_pair(ADD_MARKER, 
                          &_subgames.back()));
    if (WRITE_SUMGAME_PLAY)
        cout << "End sumgame::add " << *this << '\n';
}


void sumgame::play(game& g, int from, const int to)
{
    if (WRITE_SUMGAME_PLAY)
        cout << "Start play sum " << *this 
             << " in subgame " << g
             << " from " << from
             << " to " << to
             << " by " << ColorCode[_toPlay]
             << '\n';
    const int opp = Opp(_toPlay);
    vector<game> candidateGames = g.play(from, to, _toPlay, opp);

    _gameRecord.push_back(make_pair(START_MARKER, nullptr));
    deactivate(&g);
    
    for (auto c: candidateGames)
    {
        game* inverse = findInverse(c);
        if (inverse)
        {
            //cout << "Candidate" << c 
            //     << " has inverse " << *inverse << '\n';
            deactivate(inverse);
        }
        else
        {
            //clobberCache::tryLookupResult(c);
            assert (! c.isComputedZero());
            add(c);
            //cout << " add Candidate" << c << '\n';
        }
    }
    _toPlay = opp;
    if (WRITE_SUMGAME_PLAY)
        cout << "End play sumgame " << *this << '\n';
}

void sumgame::undoMove()
{
    if (WRITE_SUMGAME_PLAY)
        cout << "Start undoMove " << *this << '\n';
    const int opp = Opp(_toPlay);
    assert(! _gameRecord.empty());
    for (;;)
    {
        auto p = _gameRecord.back();
        _gameRecord.pop_back();
        if (p.first == START_MARKER)
            break;
        if (p.first == ADD_MARKER)
        {
            assert(p.second == &(_subgames.back()));
            _subgames.pop_back();
        }
        else
        {
            assert(p.first == DEACTIVATE_MARKER);
            game* g = p.second;
            assert(findInactive(g));
            g->setActive(true);
            if (WRITE_SUMGAME_PLAY)
                cout << "Re-activate " << g << '\n';
        }
    }
    _toPlay = opp;
    if (WRITE_SUMGAME_PLAY)
        cout << "End undoMove " << *this << '\n';
}

int sumgame::nodesSearched() const
{
    return _nodesSearched;
}

move sumgame::getPV() const
{
    move m = {-1, -1};
    if(!_pv.empty())
        return _pv.top();
    else
        return m;
}