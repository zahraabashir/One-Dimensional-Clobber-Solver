#include "database.h"
#include <iostream>

int Database::getIdx(int len, char *board) {
    int idx = 0;
	int skipAccumulator = 0;

	int rowSize = 2;
	for (int i = 1; i < len; i++) {
		skipAccumulator += rowSize;
		rowSize *= 2;
	}

	int power = 1;
	for (int i = 0; i < len; i++) {
		idx += board[i] == 2 ? power : 0;
		power *= 2;
	}

	return idx + skipAccumulator;
}

Database::Database() {
    size = 0;

    int maxGame = 0;
    for (int length = 1; length <= DB_MAX_BITS; length++) {
        maxGame *= 2;
        maxGame += 1;

        size += maxGame + 1;
    }

    size *= DB_ENTRY_SIZE;

    data = (unsigned char *) calloc(size, 1);
}

Database::~Database() {
    free(data);
	//std::cout << "DB destructor" << std::endl;
}

unsigned char *Database::get(int len, char *board) {
    if (len > DB_MAX_BITS) {
        return 0;
    }

	int idx = getIdx(len, board);
    return &data[idx * DB_ENTRY_SIZE];
}

//void Database::set(int len, char *board, int outcome) {
//	int idx = getIdx(len, board);
//	char outcomeByte = outcome;
//    data[idx] = outcomeByte;
//}

void Database::load() {
    file = fopen("database.bin", "r+");
    fread(data, 1, size, file);
    fclose(file);
}

void Database::save() {
    file = fopen("database.bin", "r+");
    fwrite(data, 1, size, file);
    fclose(file);
}
