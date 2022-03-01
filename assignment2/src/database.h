#pragma once
#include <cstdio>


#define OC_UNKNOWN 0
#define OC_B 1
#define OC_W 2
#define OC_P 3
#define OC_N 4

#define DB_MAX_BITS 18

class Database {
  private:
    FILE *file;
    unsigned char *data;


  public:
    size_t size;
    int getIdx(int len, char *board);

    Database();
    ~Database();
    int get(int len, char *board);
    void set(int len, char *board, int outcome);

    void load();
    void save();
};
