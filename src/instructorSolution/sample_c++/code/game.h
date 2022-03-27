#ifndef game_H
#define game_H

#include <iostream>
#include <string>
#include <vector>
#include "bits.h"

using std::cout;
using std::vector;

enum gameStatus
{ P, N, L, R
};

//---------------------------------------------------------------------------

class game
{
public:
    game(); // empty game, size 0
    game(bits bitMap, int size);
    bool operator==(const game& rhs) const;
    bool operator!=(const game& rhs) const;
    
    char gameStatus() const;
    bool isComputed() const;
    bool isActive() const;
    bool isComputedZero() const;
    bool isZero() const;
    bool isPositive() const;
    bool isNegative() const;
    bool isNextPlayerWin() const;
    bool isInverse(const game& other) const;
    int size() const;
    bits bitmap() const;

    bool inRange(int index) const;
    bool isColor(int i, int color) const;
    std::string print() const;
    
    void setActive(bool status);
    void compute();
    void copyResultFrom(const game& other);
    
    vector<game> play(const int from, const int to,
                      const int toPlay, const int opp) const;

    static game bwgame(int size); // alternating string "bwbw..."
    
private:
    void assertInRange(int index) const;
    void computeInverseCodes();
    bool isSingleColor() const; // private since only happens during play
    bits _bitMap, _inversecode1, _inversecode2;
    int _size;
    bool _blackWinGoingFirst, _whiteWinGoingFirst,
         _blackWinComputed, _whiteWinComputed,
         _isActive;
};


//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const game& g);
//---------------------------------------------------------------------------

inline game::game() : _bitMap(0), 
                      _inversecode1(0), _inversecode2(0),
                      _size(0),
                      _blackWinGoingFirst(false), _whiteWinGoingFirst(false),
                      _blackWinComputed(true), _whiteWinComputed(true),
                      _isActive(true)
{ }

inline game::game(bits bitMap, int size) : _bitMap(bitMap), 
                  _inversecode1(0), _inversecode2(0),
                  _size(size),
                  _blackWinGoingFirst(false), _whiteWinGoingFirst(false),
                  _blackWinComputed(false), _whiteWinComputed(false),
                  _isActive(true)
{
    assert(size > 1);
    assert(size <= MAX_SIZE);
    if(bitMap >= (static_cast<bits>(1) << size))
        std::cout << "bitmap " << bitMap << " size " << size << std::endl;
    assert(bitMap < (static_cast<bits>(1) << size));
    computeInverseCodes();
}

inline bool game::operator==(const game& rhs) const
{
    return _size == rhs._size
        && _bitMap == rhs._bitMap;
        // not checking the flags and inverse codes here, under the assumption that 
        // they will only be computed for the first such game
}

inline bool game::operator!=(const game& rhs) const
{
    return ! (*this == rhs);
}

inline int game::size() const
{
    return _size;
}

inline bits game::bitmap() const
{
    return _bitMap;
}

inline void game::computeInverseCodes()
{
    assert(_inversecode1 == 0);
    assert(_inversecode2 == 0);
    _inversecode1 = ~_bitMap & mask(_size);
    _inversecode2 = reverse(_inversecode1, _size);
}

inline bool game::isComputed() const
{
    assert(_blackWinComputed == _whiteWinComputed);
    return _blackWinComputed;
}

inline void game::copyResultFrom(const game& other)
{
    assert(_isActive);
    assert(other._isActive);
    assert(! _blackWinComputed);
    assert(! _whiteWinComputed);
    assert(other.isComputed());
    _blackWinComputed = _whiteWinComputed = true;
    _blackWinGoingFirst = other._blackWinGoingFirst;
    _whiteWinGoingFirst = other._whiteWinGoingFirst;
}

inline bool game::isZero() const
{
    assert(isComputed());
    return ! _blackWinGoingFirst && ! _whiteWinGoingFirst;
}

inline bool game::isComputedZero() const
{
    return isComputed() && isZero();
}


inline bool game::isPositive() const
{
    assert(isComputed());
    return _blackWinGoingFirst && ! _whiteWinGoingFirst;
}

inline bool game::isNegative() const
{
    assert(isComputed());
    return ! _blackWinGoingFirst && _whiteWinGoingFirst;
}

inline bool game::isNextPlayerWin() const
{
    assert(isComputed());
    return _blackWinGoingFirst && _whiteWinGoingFirst;
}

inline bool game::inRange(int index) const
{
    return index >= 0 && index < _size;
}

inline void game::assertInRange(int index) const
{
    assert(index >= 0);
    assert(index < _size);
}

inline bool game::isColor(int i, int color) const
{
    assert(i >= 0);
    assert(i < _size);
    assertbw(color);
    return testBit(_bitMap, i) == color;
}

inline bool game::isActive() const
{
    return _isActive;
}

inline bool game::isInverse(const game& other) const
{
    return _size == other._size
        && (   _bitMap == other._inversecode1
            || _bitMap == other._inversecode2
           );
}

inline void game::setActive(bool status)
{
    _isActive = status;
}

inline bool game::isSingleColor() const
{ // all 0 or all 1
    return _bitMap == 0 || _bitMap == mask(_size);
}

//---------------------------------------------------------------------------

class gameMoveGenerator
{
public:
    gameMoveGenerator(const game& g, int color);
    void operator++();
    operator bool() const;
    int from() const;
    int to() const;

private:
    bool isMove(int point, int dir, int color) const;
    const game& _g;
    int _color;
    int _current;
    int _dir; // +-1
};

inline gameMoveGenerator::gameMoveGenerator(const game& g, int color) :
    _g(g), _color(color),
    _current(0), _dir(1)
{
    if (! isMove(_current, _dir, _color))
        ++(*this);
}

inline bool gameMoveGenerator::isMove(int point, int dir, int color) const
{
    return _g.isColor(point, color)
        && _g.isColor(point + dir, Opp(color));
}

inline void gameMoveGenerator::operator++()
{
    int score = 4 * _current + _dir + 2;
    for (; 
         score <= 4 * _g.size() - 5; score += 2)
    {
        int old_current = _current;
        int old_dir = _dir;
        _current = (score + 1) / 4;
        _dir = score - 4 * _current;
        assert(_dir == -1 || _dir == 1);
        if (old_dir == -1)
        {
            assert(_dir == 1);
            assert(_current == old_current);
        }
        else
        {
            assert(_dir == -1);
            assert(_current == old_current + 1);
        }
        if (isMove(_current, _dir, _color))
            break;
    }
    if (score > 4 * _g.size() - 5)
        _current = _g.size();
}

inline gameMoveGenerator::operator bool() const
{
    return _current < _g.size();
}

inline int gameMoveGenerator::from() const
{
    return _current;
}

inline int gameMoveGenerator::to() const
{
    return _current + _dir;
}


void testGenerator();
//---------------------------------------------------------------------------

void printVisitOrder();
void initVisitOrder();

#endif // game_H
