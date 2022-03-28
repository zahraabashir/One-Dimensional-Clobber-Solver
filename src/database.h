#pragma once
#include <cstdio>


#define OC_UNKNOWN 0
#define OC_B 1
#define OC_W 2
#define OC_P 3
#define OC_N 4

#define DB_MAX_BITS 16

#define DB_ENTRY_SIZE 1 + 2 * sizeof(uint64_t)

#define DB_GET_OUTCOME(entry) entry != 0 ? *((unsigned char *) entry) : 0;
#define DB_SET_OUTCOME(entry, value) *((unsigned char *) entry) = value;

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
