#include "zobrist.h"
#include <ctime>
#include <vector>
#include <cassert>
#include <random>

namespace {

////////////////////////////////////////////////// class RandomGenerator
class RandomGenerator {
public:
    RandomGenerator();
    uint64_t getRandomNumber();

private:
    std::mt19937_64 _rng;

    static std::uniform_int_distribution<unsigned long long> _dist64;
};

////////////////////////////////////////////////// RandomGenerator implementation
std::uniform_int_distribution<unsigned long long> RandomGenerator::_dist64(
    1, std::numeric_limits<uint64_t>::max());

inline RandomGenerator::RandomGenerator()
{
    _rng.seed(std::time(0));
}

inline uint64_t RandomGenerator::getRandomNumber()
{
    return _dist64(_rng);
}

////////////////////////////////////////////////// class RandomTable
class RandomTable {
public:
    RandomTable();

    uint64_t get(uint8_t val, size_t pos);
    const std::vector<uint64_t> &getTable();

    void growTable(size_t target);
private:

    std::vector<uint64_t> _numbers;
    size_t _currentSize;

    static RandomGenerator _generator;
};

////////////////////////////////////////////////// RandomTable implementation
RandomGenerator RandomTable::_generator;

inline RandomTable::RandomTable()
{
    _currentSize = 0;

    growTable(4);
}

inline uint64_t RandomTable::get(uint8_t val, size_t pos)
{
    assert(0 <= val && val <= 2);
    growTable(pos + 1);

    const size_t idx = pos * 3 + val;
    assert(idx < _numbers.size());

    return _numbers[idx];
}

const std::vector<uint64_t> &RandomTable::getTable() {
    return _numbers;
}

inline void RandomTable::growTable(size_t target)
{
    if (target < _currentSize)
        return;

    while (_currentSize < target) {
        _currentSize++;
        _numbers.push_back(_generator.getRandomNumber());
        _numbers.push_back(_generator.getRandomNumber());
        _numbers.push_back(_generator.getRandomNumber());
    }

    assert(_currentSize * 3 == _numbers.size());
}

//////////////////////////////////////////////////


} // namespace

using namespace std;

uint64_t getZobristHash(int player, const uint8_t* board, size_t len) {
    static RandomTable table;

    table.growTable(len + 2);

    const vector<uint64_t> &numbers = table.getTable();

    uint64_t val = 0;

    val ^= numbers[player];

    size_t idx = 3;

    for (size_t i = 0; i < len; i++) {
        val ^= numbers[idx + board[i]];
        idx += 3;
    }
        //val ^= table.get(board[i], i + 1);

    return val;
}
