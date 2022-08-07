#pragma once

#include "options.h"
#include <cstdio>
#include <cstdint>
#include <vector>


#define OC_UNKNOWN 0
#define OC_B 1
#define OC_W 2
#define OC_P 3
#define OC_N 4


/*
char outcome
uint64_t domBlack
uint64_t domWhite
int8_t lowerBound
int8_t upperBound
int metric
int link
int gameNumber
uint64_t shape
*/

#define DB_ENTRY_SIZE (1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t) + 3 * sizeof(int) + sizeof(uint64_t))

#define DB_GET_OUTCOME(entry) (entry == 0 ? 0 : *((unsigned char *) entry))
#define DB_SET_OUTCOME(entry, value) *((unsigned char *) entry) = value


#define DB_GET_DOMINATED(entry, player) (entry == 0 ? 0 : ((uint64_t *) (&entry[1]))[player - 1])
#define DB_SET_DOMINATED(entry, player, mask) ((uint64_t *) (&entry[1]))[player - 1] = mask


#define DB_GET_BOUND(entry, index) ((int8_t *) (entry + 1 + 2 * sizeof(uint64_t)))[index]
#define DB_SET_BOUND(entry, index, bound) ((int8_t *) (entry + 1 + 2 * sizeof(uint64_t)))[index] = bound

#define DB_GET_UDMOVECOUNT(entry) *((int *) (entry + 1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t)))
#define DB_SET_UDMOVECOUNT(entry, count) *((int *) (entry + 1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t))) = count


#define DB_GET_LINK(entry) *((int *) (entry + 1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t) + sizeof(int)))
#define DB_SET_LINK(entry, link) *((int *) (entry + 1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t) + sizeof(int))) = link

#define DB_GET_LENGTH(entry) *((int * ) (entry + 1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t) + 2 * sizeof(int)))
#define DB_SET_LENGTH(entry, length) *((int * ) (entry + 1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t) + 2 * sizeof(int))) = length

#define DB_GET_NUMBER(entry) *((int * ) (entry + 1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t) + 3 * sizeof(int)))
#define DB_SET_NUMBER(entry, number) *((int * ) (entry + 1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t) + 3 * sizeof(int))) = number

uint64_t shapeToID(std::vector<int> &shape);
size_t countShape(const std::vector<int> &shape);

constexpr uint64_t _shiftAmount() {

    //2, 3, ..., DB_MAX_BITS (excludes 0,1)
    //2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
    uint64_t maxChunk = DB_MAX_BITS - 2;

    uint64_t shift = 0;
    for (int i = 0; (maxChunk >> i) != 0; i++) {
        shift += 1;
    }

    return shift;
}


struct ShapeNode {
    std::vector<int> shape;
    uint64_t id;
    int bits;
    size_t entryCount;
    std::vector<ShapeNode *> children;

    ~ShapeNode();
};


class Database {
  private:
    FILE *file;
    char *data;
    char *table;

    uint64_t shapeIndexEntries;
    uint64_t gameEntries;

    void initShapeTree();
    void initMemory();

    void genShapes(ShapeNode *node, int prev, const std::vector<int> &shape, int remaining);
    void visitShapeTree(ShapeNode *node);
    //void revisitShapeTree(ShapeNode *node, char **shapeIndex, uint64_t *tableOffset);

    void revisitShapeTree(ShapeNode *node, std::vector<ShapeNode *> &nodeList);

    uint64_t totalBytes;


    uint64_t searchShapeIndex(uint64_t shapeID);

  public:
    ShapeNode *shapeTree;

    Database();
    ~Database();

    void initData();

    uint64_t getIdx(int len, char *board);


    //Need to implement these so that the linker doesn't complain
    unsigned char *get(int len, char *board);
    unsigned char *getFromIdx(int idx); //TODO change this to uint64_t

    void load();
    void save();


};
