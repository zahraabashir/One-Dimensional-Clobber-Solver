#include "options.h"
#include <cstdio>
#include <stdint.h>
#include <vector>

#define OC_UNKNOWN 0
#define OC_B 1
#define OC_W 2
#define OC_P 3
#define OC_N 4

class Database {
  private:
    FILE *file; // read/write from here
    uint8_t *data; // load database into this array

    const static size_t headerSize = 8; //TODO

    const static size_t indexEntrySize = 2 * sizeof(uint64_t);
    size_t indexEntryCount;

    static constexpr size_t entrySize = 8; //TODO
    size_t entryCount;

    uint8_t *index;

  public:
    size_t size;

    Database();
    ~Database();

    void init();
    void save();
    void load();

    uint64_t getIdx(uint8_t *board, size_t len);

    uint8_t *get(uint8_t *board, size_t len);
    uint8_t *getFromIdx(uint64_t idx);
};


uint64_t shapeToNumber(const std::vector<int> &shape);
std::vector<int> numberToShape(uint64_t number);
