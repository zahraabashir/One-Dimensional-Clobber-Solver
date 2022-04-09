#pragma once
#include <cstdio>


#define OC_UNKNOWN 0
#define OC_B 1
#define OC_W 2
#define OC_P 3
#define OC_N 4

/*
    DB_MAX_BITS was 20
    DB_MAX_DOMINANCE_BITS was 12
    DB_MAX_BOUND_BITS was 16
    

    WWWWWB  -5 = 4v *
    WWWWB   -4 = 3v
    WWWB    -3 = 2v *
    WWB     -2 = v
    WB      -1 = *
            0 = 0
    BW      1 = *
    BBW     2 = ^
    BBBW    3 = 2^ *
    BBBBW   4 = 3^
    BBBBBW  5 = 4^ * 
    ...

*/

//16,10,16
#define DB_MAX_BITS 12
#define DB_MAX_DOMINANCE_BITS 10
#define DB_MAX_BOUND_BITS 12

#define DB_ENTRY_SIZE (1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t) + 2 * sizeof(int))

#define DB_GET_OUTCOME(entry) (entry == 0 ? 0 : *((unsigned char *) entry))
#define DB_SET_OUTCOME(entry, value) *((unsigned char *) entry) = value

//#define DB_GET_DOMINATED(entry, player) (entry == 0 ? 0 : *((uint64_t *) (entry + 1 + (sizeof(uint64_t) * (player - 1)))))
//#define DB_SET_DOMINATED(entry, player, mask) *((uint64_t *) (entry + 1 + (sizeof(uint64_t) * (player - 1)))) = mask


#define DB_GET_DOMINATED(entry, player) (entry == 0 ? 0 : ((uint64_t *) (&entry[1]))[player - 1])
#define DB_SET_DOMINATED(entry, player, mask) ((uint64_t *) (&entry[1]))[player - 1] = mask


#define DB_GET_BOUND(entry, index) ((int8_t *) (entry + 1 + 2 * sizeof(uint64_t)))[index]
#define DB_SET_BOUND(entry, index, bound) ((int8_t *) (entry + 1 + 2 * sizeof(uint64_t)))[index] = bound

#define DB_GET_UDMOVECOUNT(entry) *((int *) (entry + 1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t)))
#define DB_SET_UDMOVECOUNT(entry, count) *((int *) (entry + 1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t))) = count


#define DB_GET_LINK(entry) *((int *) (entry + 1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t) + sizeof(int)))
#define DB_SET_LINK(entry, link) *((int *) (entry + 1 + 2 * sizeof(uint64_t) + 2 * sizeof(int8_t) + sizeof(int))) = link

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
