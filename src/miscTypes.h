#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <iostream>

#include "utils.h"

////////////////////////////////////////////////// class Subgame
struct Subgame {
    // Constructor
    Subgame();
    Subgame(const uint8_t *arr, size_t len);

    size_t size() const;
    uint8_t *board();
    const uint8_t *board() const;

    const std::vector<uint8_t> &boardVecConst() const;
    std::vector<uint8_t> &boardVec();

    const uint8_t &operator[](size_t idx) const;
    uint8_t &operator[](size_t idx);

    void tryMirror();

    Subgame *getNormalizedGame() const;
    uint64_t getHash() const;

    std::vector<Subgame*> getChildren(int player, uint64_t dominance) const;
    std::vector<Subgame*> getNormalizedChildren(int player, uint64_t dominance) const;

    static Subgame *concatSubgames(const std::vector<Subgame*> &subgames);
    static bool normalizeSortOrder(const Subgame *sg1, const Subgame *sg2);
    static bool isVisuallyInversePair(const Subgame *sg1, const Subgame *sg2);

private:
    friend std::ostream &operator<<(std::ostream &os, const Subgame &sg);

    std::vector<uint8_t> _data;
};

////////////////////////////////////////////////// Subgame methods
// Constructor
inline Subgame::Subgame() {
}

inline Subgame::Subgame(const uint8_t *arr, size_t len) {
    _data.reserve(len);

    for (size_t i = 0; i < len; i++)
        _data.push_back(arr[i]);
}

inline size_t Subgame::size() const {
    return _data.size();
}

inline uint8_t *Subgame::board() {
    return _data.data();
}

inline const uint8_t *Subgame::board() const {
    return _data.data();
}

inline const std::vector<uint8_t> &Subgame::boardVecConst() const {
    return _data;
}

inline std::vector<uint8_t> &Subgame::boardVec() {
    return _data;
}

inline const uint8_t &Subgame::operator[](size_t idx) const {
    assert(idx < _data.size());
    return _data[idx];
}

inline uint8_t &Subgame::operator[](size_t idx) {
    assert(idx < _data.size());
    return _data[idx];
}

inline std::ostream &operator<<(std::ostream &os, const Subgame &sg) {
    const std::vector<uint8_t> &vec = sg.boardVecConst();

    os << '(';

    for (uint8_t tile : vec)
        os << playerNumberToChar(tile);

    os << ')';
    return os;
}

////////////////////////////////////////////////// struct DominancePair
struct DominancePair {
    uint64_t domBlack;
    uint64_t domWhite;
};

inline void setDominated(uint64_t &mask, size_t moveIdx) {
    assert(moveIdx < 64);
    mask |= (uint64_t(1) << moveIdx);
}

inline bool getDominated(uint64_t &mask, size_t moveIdx) {
    assert(moveIdx < 64);
    return ((mask >> moveIdx) & 0x1);
}


////////////////////////////////////////////////// struct BoundsPair
struct BoundsPair {
    int8_t low;
    int8_t high;
};

//////////////////////////////////////////////////
std::vector<Subgame*> makeGameNew(uint64_t shapeNumber, uint32_t gameNumber);

//////////////////////////////////////////////////
inline std::ostream &operator<<(std::ostream &os, const std::vector<Subgame*> &vec) {
    os << '{';

    const size_t nGames = vec.size();

    for (size_t i = 0; i < nGames; i++) {
        const Subgame *sg = vec[i];

        if (sg == nullptr)
            os << "NULL";
        else
            os << *sg;

        if (i + 1 < nGames)
            os << ", ";
    }

    os << '}';
    return os;
}

std::vector<Subgame*> generateSubgamesNew(const uint8_t *board, size_t len);
