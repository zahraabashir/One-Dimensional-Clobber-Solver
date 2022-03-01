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

    data = (unsigned char *) calloc(size, 1);
}

Database::~Database() {
    free(data);
	//std::cout << "DB destructor" << std::endl;
}

int Database::get(int len, char *board) {
    if (len > DB_MAX_BITS) {
        return 0;
    }

	int idx = getIdx(len, board);
    return data[idx];
}

void Database::set(int len, char *board, int outcome) {
	int idx = getIdx(len, board);
	char outcomeByte = outcome;
    data[idx] = outcomeByte;
}

void Database::load() {
    file = fopen("database.bin", "r+");

    //Decompress data
    size_t compressedSize = (size / 2) + (size & 1);
    unsigned char compressed[compressedSize];
    fread(compressed, 1, compressedSize, file);

    size_t dataIndex = 0;
    for (size_t i = 0; i < compressedSize; i++) {
        unsigned char byte = compressed[i];

        unsigned char b1 = byte & 0xF;
        data[dataIndex] = b1;
        dataIndex += 1;
 
        unsigned char b2 = (byte >> 4) & 0xF;
        if (dataIndex < size) {
            data[dataIndex] = b2;
        }
        dataIndex += 1;
        
    }


    fclose(file);
}

void Database::save() {
    file = fopen("database.bin", "r+");


    //Compress data
    size_t compressedSize = (size / 2) + (size & 1);
    unsigned char compressed[compressedSize];

    for (size_t i = 0; i < size; i += 2) {
        unsigned char b1 = 0;
        unsigned char b2 = 0;

        b1 = data[i];
 
        if (i + 1 < size) {
            b2 = data[i + 1];
        }

        unsigned char byte = ((b2 & 0xF) << 4) + (b1 & 0xF);

        compressed[i / 2] = byte;
    }


    fwrite(compressed, 1, compressedSize, file);
    fclose(file);
}
