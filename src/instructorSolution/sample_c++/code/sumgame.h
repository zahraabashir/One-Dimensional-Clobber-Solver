#ifndef sumgame_H
#define sumgame_H

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <stack>
#include "game.h"

struct move
{
    int from;
    int to;
};

//---------------------------------------------------------------------------
class sumgame
{
public:
    sumgame();
    sumgame(const game& g);
    void add(game g);
    void setToPlay(int color);
    void play(game& g, int from, const int to);
    void undoMove();
    std::string print() const;
    bool negamaxBoolean();
    int numTotalGames() const;
    int numActiveGames() const;
    int nodesSearched() const;
    move getPV() const;

private:
    void deactivate(game* g);
    game* findInverse(const game& candidate);
    bool findInactive(const game* candidate) const;
    bool findStaticWinner(bool& success) const;

    int _toPlay;
    vector< std::pair<int, game*> > _gameRecord;
    vector<game> _subgames;
    friend std::ostream& operator<<(std::ostream& out, const sumgame& s);
    friend class sumRestoreChecker;
    int _nodesSearched;
    std::stack<move> _pv;
};
//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const sumgame& s);

inline int sumgame::numTotalGames() const
{
    return _subgames.size(); // todo active subgames only
}

inline int sumgame::numActiveGames() const
{
    int active = 0;
    for (auto const &g: _subgames)
        if (g.isActive())
            ++active;
    return active;
}


class sumRestoreChecker
{
public:
    sumRestoreChecker(const sumgame& s);
    ~sumRestoreChecker();
private:
    int _gameRecordSize;
    int _toPlay;
    int _numTotalGames;
    int _numActiveGames;
    const sumgame& _s;
};

inline sumRestoreChecker::sumRestoreChecker(const sumgame& s)
   : _gameRecordSize(s._gameRecord.size()),
     _toPlay(s._toPlay),
     _numTotalGames(s.numTotalGames()),
     _numActiveGames(s.numActiveGames()),
    _s(s)
{ }

inline sumRestoreChecker::~sumRestoreChecker()
{
    assert(_gameRecordSize == _s._gameRecord.size());
    assert(_toPlay == _s._toPlay);
    assert(_numTotalGames == _s.numTotalGames());
    assert(_numActiveGames == _s.numActiveGames());
}

#endif // sumgame_H
