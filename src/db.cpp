#include "database.h"
#include <iostream>
#include "utils.h"

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
    file = fopen("database.bin", "r+");
}

Database::~Database() {
    fclose(file);
	//std::cout << "DB destructor" << std::endl;
}

int Database::get(int len, char *board) {
	int idx = getIdx(len, board);

	//std::cerr << idx << " ";

	char outcome = 0;

	fseek(file, idx, SEEK_SET);
	fread(&outcome, 1, 1, file);

    if (idx == 67914) {
        for (int i = 0; i < len; i++) {
            std::cout << playerNumberToChar(board[i]);
        }
        std::cout << std::endl << (int) outcome << std::endl;
        while (1) {}
    }

	return outcome;
}

void Database::set(int len, char *board, int outcome) {
	int idx = getIdx(len, board);

	char outcomeByte = outcome;

	fseek(file, idx, SEEK_SET);
	fwrite(&outcome, 1, 1, file);
}
