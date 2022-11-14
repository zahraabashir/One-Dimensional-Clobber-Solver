#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>

#include "database3.h"
#include "utils.h"

using namespace std;


template <class T>
void printBits(T val) {
    for (int i = 8 * sizeof(T) - 1; i >= 0; i--) {
        cout << ((val >> i) & 1);
    }
}

bool sameEntry(uint8_t *e1, uint8_t *e2) {
    return memcmp(e1, e2, DBLayout::size()) == 0;
}

void diffMode(string &file1, string &file2) {
    Database db1;
    db1.loadFrom(file1.c_str());

    Database db2;
    db2.loadFrom(file2.c_str());

    vector<vector<int>> shapeList = makeShapes();

    sort(shapeList.begin(), shapeList.end(),
        [](const vector<int> &s1, const vector<int> &s2) {
            int bits1 = s1.size() - 1;
            int bits2 = s2.size() - 1;

            for (int chunk : s1) {
                bits1 += chunk;
            }

            for (int chunk : s2) {
                bits2 += chunk;
            }

            return bits1 < bits2;
        }
    );

    //Iterate over all shapes
    for (const vector<int> &shape : shapeList) {
        uint64_t shapeNumber = shapeToNumber(shape);

        //iterate over all games
        int gameBits = 0;
        for (int chunk : shape) {
            gameBits += chunk;
        }

        uint32_t minGame = 0;
        uint32_t gameCount = 1 << gameBits;

        for (uint32_t gameNumber = minGame; gameNumber < gameCount; gameNumber++) {
            //Get game and print it
            uint8_t *board;
            size_t boardLen;
            makeGame(shapeNumber, gameNumber, &board, &boardLen);
            printBoard(board, boardLen, true);

            uint8_t *e1 = db1.get(board, boardLen);
            uint8_t *e2 = db2.get(board, boardLen);


            if (!sameEntry(e1, e2)) {
                cout << "This entry doesn't match" << endl;
            }

            delete[] board;
        }
    }

    cout << "Done!" << endl;

}

int main(int argc, char **argv) {
    string fileName = "database3.bin";

    if (argc == 2) {
        fileName = string(argv[1]);
    }

    if (argc == 3) {
        string file1(argv[1]);
        string file2(argv[2]);
        diffMode(file1, file2);
        return 0;
    }

    cout << "Loading from " << fileName << endl;

    Database db;
    db.loadFrom(fileName.c_str());

    string query;
    while (cin >> query) {
        uint8_t board[query.size()];

        for (int i = 0; i < query.size(); i++) {
            board[i] = charToPlayerNumber(query[i]);
        }

        uint64_t idx = db.getIdx(board, query.size());
        uint8_t *entry = (idx != 0) ? db.getFromIdx(idx) : 0;

        if (!entry) {
            cout << "Not found" << endl;
            continue;
        }

        //Print entry contents
        cout << "IDX: " << idx << endl;

        cout << "Outcome: " << (int) *db_get_outcome(entry) << endl;

        uint64_t domBlack = db_get_dominance(entry)[0];
        uint64_t domWhite = db_get_dominance(entry)[1];
        cout << "domBlack: " << domBlack << endl;
        printBits(domBlack);
        cout << endl;
        cout << "domWhite: " << domWhite << endl;
        printBits(domWhite);
        cout << endl;

        int8_t low = db_get_bounds(entry)[0];
        int8_t high = db_get_bounds(entry)[1];
        cout << "Low bound: " << (int) low << endl;
        cout << "High bound: " << (int) high << endl;

        cout << "Metric: " << *db_get_metric(entry) << endl;

        //link
        uint64_t link = *db_get_link(entry);
        cout << "Link: " << link << endl;
        uint8_t *linkedEntry = db.getFromIdx(link);

        if (linkedEntry) {
            uint64_t linkedShape = *db_get_shape(linkedEntry);
            uint32_t linkedNumber = *db_get_number(linkedEntry);

            size_t linkedSize;
            uint8_t *linkedGame;

            makeGame(linkedShape, linkedNumber, &linkedGame, &linkedSize);

            cout << "Linked entry's game:" << endl;
            printBoard(linkedGame, linkedSize);
            cout << endl;

            delete[] linkedGame;
        } else {
            cout << "Linked entry was invalid" << endl;
        }

        //Shape
        uint64_t shapeNumber = *db_get_shape(entry);
        vector<int> inflatedShape = numberToShape(shapeNumber);
        cout << "Shape: " << inflatedShape << " (" << shapeNumber << ")" << endl;

        //Number
        uint32_t number = *db_get_number(entry);
        cout << "Number: " << number << endl;

        //Inflated game
        size_t inflatedSize;
        uint8_t *inflated;

        makeGame(shapeNumber, number, &inflated, &inflatedSize);

        cout << "Inflated game reported by entry: " << endl;
        printBoard(inflated, inflatedSize);
        cout << endl;

        delete[] inflated;

        cout << endl;
    }

}
