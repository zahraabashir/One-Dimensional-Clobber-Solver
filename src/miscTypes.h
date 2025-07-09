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
    Subgame(uint8_t *arr, size_t len);

    size_t size() const;
    uint8_t *board();
    const uint8_t *board() const;

    const std::vector<uint8_t> &boardVecConst() const;
    std::vector<uint8_t> &boardVec();

    bool isActive() const;
    void setIsActive(bool newActive);

    const uint8_t &operator[](size_t idx) const;
    uint8_t &operator[](size_t idx);

    void tryMirror();

    static Subgame *concatSubgames(const std::vector<Subgame*> &subgames);
    static bool normalizeSortOrder(const Subgame *sg1, const Subgame *sg2);
    static bool isVisuallyInversePair(const Subgame *sg1, const Subgame *sg2);

private:
    friend std::ostream &operator<<(std::ostream &os, const Subgame &sg);

    std::vector<uint8_t> _data;
    bool _isActive;
};

////////////////////////////////////////////////// Subgame methods
// Constructor
inline Subgame::Subgame(): _isActive(true) {
}

inline Subgame::Subgame(uint8_t *arr, size_t len): _isActive(true) {
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

inline bool Subgame::isActive() const {
    return _isActive;
}

inline void Subgame::setIsActive(bool newActive) {
    _isActive = newActive;
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


