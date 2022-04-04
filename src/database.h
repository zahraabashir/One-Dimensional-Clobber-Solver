#pragma once
#include <cstdio>


#define OC_UNKNOWN 0
#define OC_B 1
#define OC_W 2
#define OC_P 3
#define OC_N 4
#define VAL_UNK 1000000

#define DB_MAX_BITS 9
#define DB_MAX_DOMINANCE_BITS 8

#define DB_ENTRY_SIZE (1 + 2 * sizeof(uint64_t)+ sizeof(uint64_t))

#define DB_GET_OUTCOME(entry) (entry == 0 ? 0 : *((unsigned char *) entry))
#define DB_SET_OUTCOME(entry, value) *((unsigned char *) entry) = value

//#define DB_GET_DOMINATED(entry, player) (entry == 0 ? 0 : *((uint64_t *) (entry + 1 + (sizeof(uint64_t) * (player - 1)))))
//#define DB_SET_DOMINATED(entry, player, mask) *((uint64_t *) (entry + 1 + (sizeof(uint64_t) * (player - 1)))) = mask


#define DB_GET_DOMINATED(entry, player) (entry == 0 ? 0 : ((uint64_t *) (&entry[1]))[player - 1])
#define DB_SET_DOMINATED(entry, player, mask) ((uint64_t *) (&entry[1]))[player - 1] = mask


#define DB_GET_VALUE(entry) (entry == 0 ? 1000000 : ((uint64_t *) (&entry[2]))[0])
#define DB_SET_VALUE(entry, value) ((uint64_t *) (&entry[2]))[0]= value

class Database {
  private:
    FILE *file;
    unsigned char *data;


  public:
    size_t size;
    int getIdx(int len, char *board);

    Database();
    ~Database();
    unsigned char *get(int len, char *board);
    //void set(int len, char *board, int outcome);

    void load();
    void save();
};
