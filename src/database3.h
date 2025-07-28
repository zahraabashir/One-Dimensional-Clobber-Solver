#pragma once

#include "options.h"
#include "miscTypes.h"
#include <cstdio>
#include <stdint.h>
#include <vector>

#define OC_UNKNOWN 0
#define OC_B 1
#define OC_W 2
#define OC_P 3
#define OC_N 4

#define DB_NOT_FOUND ((uint64_t) -1)

template <class T>
struct _SZ {
    static constexpr size_t size() {
        return sizeof(T);
    }
};

#define sz(T) (_SZ<T>::size())

enum {
    DB_OUTCOME = 0,
    DB_DOMINANCE,
    DB_BOUNDS,
    DB_METRIC,
    DB_LINK,
    DB_SHAPE,
    DB_NUMBER,
    DB_SIMPLEST_MOVES,
};

struct DBLayout {
    static constexpr size_t arr[] = {
        sz(uint8_t),        // outcome
        sz(uint64_t[2]),    // domBlack/domWhite
        sz(int8_t[4]),     // low/high bounds
        sz(uint64_t),       // simplicity metric
        sz(uint64_t),       //link
        sz(uint64_t),       //shape
        sz(uint32_t),       //number
        sz(uint8_t[2]),     // simplest moves
    };

    static constexpr size_t N = sizeof(arr) / sizeof(size_t);

    //size of one entry
    static constexpr size_t size() {
        size_t sum = 0;

        for (size_t i = 0; i < N; i++) {
            sum += arr[i];
        }

        return sum;
    }
};

template <class Layout, size_t index>
struct Offset {
    constexpr operator size_t() {
        static_assert(index < Layout::N, "Offset template bad index");
        return Layout::arr[index - 1] + Offset<Layout, index - 1>();
    }
};

template <class Layout>
struct Offset<Layout, 0> {
    constexpr operator size_t() {
        static_assert(0 < Layout::N, "Offset template bad index");
        return 0;
    }
};


uint8_t *db_get_outcome(const uint8_t *entry);
uint64_t *db_get_dominance(const uint8_t *entry);
int8_t *db_get_bounds(const uint8_t *entry);
uint64_t *db_get_metric(const uint8_t *entry);
uint64_t *db_get_link(const uint8_t *entry);
uint64_t *db_get_shape(const uint8_t *entry);
uint32_t *db_get_number(const uint8_t *entry);
uint8_t *db_get_simplest_moves(const uint8_t *entry);

struct IndirectLink {

    IndirectLink(): directLink(-1) {
    }

    IndirectLink(uint64_t directLink): directLink(directLink) {
    }

    uint64_t directLink;
};

class Database {
  private:
  public:
    FILE *file; // read/write from here
    uint8_t *data; // load database into this array
    uint64_t *index;

    /*
        header contains:
        size_t indexEntryCount
        size_t entryCount
    */
    const static size_t headerSize = 2 * sizeof(size_t);

    const static size_t indexEntrySize = 2 * sizeof(uint64_t);
    size_t indexEntryCount;

    static constexpr size_t entrySize = DBLayout::size();
    size_t entryCount;

  private:
    bool _useIndirectLinks;
    size_t _defaultIndirectLinksSize1;
    IndirectLink *_defaultIndirectLinks1;

  public:
    IndirectLink &getDefaultIndirectLink1(size_t entryNumber);

    size_t size;

    Database();
    ~Database();

    void init();
    void save(const char *fileName);
    //void load();
    void loadFrom(const char *fileName);

    uint64_t getIdxDirect(const uint8_t *board, size_t len);
    uint64_t getIdxDirect(const Subgame &sg);

    uint8_t *get(const uint8_t *board, size_t len);
    uint8_t *get(const Subgame &sg);

    uint8_t *getFromIdx(uint64_t idx);
    uint8_t *getFromIndirectIdx(const IndirectLink &indirect);

    void enableIndirectLinks();

    void finalizeIndirectLinks();

};

uint64_t shapeToNumber(const std::vector<int> &shape);
std::vector<int> numberToShape(uint64_t number);
std::vector<std::vector<int>> makeShapes();

inline IndirectLink &Database::getDefaultIndirectLink1(size_t entryNumber) {
    assert(_useIndirectLinks && entryNumber < _defaultIndirectLinksSize1);
    return _defaultIndirectLinks1[entryNumber];
}

