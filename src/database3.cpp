#include "database3.h"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

constexpr uint64_t shapeNumberMask() {
    uint64_t mask = 0;

    uint64_t maxChunkStored = DB_MAX_BITS - 1;

    while ((mask & maxChunkStored) != maxChunkStored) {
        mask <<= 1;
        mask |= 1;
    }

    return mask;
}

constexpr uint64_t shapeNumberShift() {
    uint64_t mask = shapeNumberMask();

    uint64_t shift = 0;

    while (mask) {
        shift += 1;
        mask >>= 1;
    }

    return shift;
}

uint64_t shapeToNumber(const vector<int> &shape) {
    uint64_t num = 0;

    for (auto it = shape.rbegin(); it != shape.rend(); it++) {
        int chunk = *it;

        num = (num << shapeNumberShift());
        num |= (chunk - 1);
    }

    return num;
}

vector<int> numberToShape(uint64_t number) {
    vector<int> shape;

    while (number) {
        int chunk = (number & shapeNumberMask()) + 1;
        shape.push_back(chunk);

        number >>= shapeNumberShift();
    }

    return shape;
}

void _makeShape(vector<vector<int>> &shapeList, vector<int> currentShape, int budget) {
    int maxLegalChunk = budget;

    if (currentShape.size() > 0) {
        maxLegalChunk = min(budget, currentShape.back());
    }

    for (int nextChunk = 2; nextChunk <= maxLegalChunk; nextChunk++) {
        vector<int> newShape = currentShape;
        newShape.push_back(nextChunk);

        shapeList.push_back(newShape);

        _makeShape(shapeList, newShape, budget - nextChunk - 1);
    }
}

vector<vector<int>> makeShapes() {
    vector<vector<int>> shapeList;

    int budget = DB_MAX_BITS;
    vector<int> emptyShape;
    _makeShape(shapeList, emptyShape, budget);

    sort(shapeList.begin(), shapeList.end(),
        [](const vector<int> &s1, const vector<int> &s2) {
            return shapeToNumber(s1) < shapeToNumber(s2);
        }
    );

    return shapeList;
}


Database::Database() {
    cout << "Making DB" << endl;


    init();

}

void Database::init() {

    //Generate shapes
    vector<vector<int>> shapeList = makeShapes();

    //for (const vector<int> &shape : shapeList) {
    //    for (int chunk : shape) {
    //        cout << chunk << " ";
    //    }
    //    cout << " | " << shapeToNumber(shape) << " -- ";

    //    cout << (numberToShape(shapeToNumber(shape)) == shape) << endl;
    //}


    indexEntryCount = shapeList.size();



    /*
        Find size, then allocate space
    
        db header
        shape index
        entries
    */
    size = 0;
    entryCount = 0;

    //header
    size += headerSize;

    //shape index
    size += indexEntryCount * indexEntrySize;

    //entries
    for (const vector<int> &shape : shapeList) {
        size_t sum = 0;
        for (int chunk : shape) {
            sum += chunk;
        }
        entryCount += (1 << sum); //this many entries for this shape
    }

    size += entryCount * entrySize;

    cout << "DB allocating " << size << " bytes" << endl;

    //Allocate space
    data = (uint8_t *) calloc(size, 1);

    /*
        Fill header/index
    */

    //Header...
    *((size_t *) data) = indexEntryCount;

    //Index...
    index = data + headerSize;

    size_t cumulativeEntries = 0;
    for (size_t i = 0; i < shapeList.size(); i++) {
        const vector<int> &shape = shapeList[i];

        uint64_t shapeNumber = shapeToNumber(shape);
        uint64_t offset = (headerSize + indexEntrySize * indexEntryCount) + cumulativeEntries * entrySize;

        uint8_t *entry = (index + i * indexEntrySize);

        //cout << "Writing to " << (int) (entry - index) << endl;

        ((uint64_t *) entry)[0] = shapeNumber;
        ((uint64_t *) entry)[1] = offset;

        size_t sum = 0;
        for (int chunk : shape) {
            sum += chunk;
        }

        cumulativeEntries += (1 << sum);
    }
}

Database::~Database() {
    cout << "Destroying DB" << endl;

    if (data != NULL) {
        free(data);
    }
}

//NOTE: doesn't update header
void Database::save() {
    cout << "DB SAVE" << endl;
    file = fopen("database3.bin", "r+");
    fwrite(data, 1, size, file);
    fclose(file);
}

void Database::load() {
    cout << "DB LOAD" << endl;

    file = fopen("database3.bin", "r+");

    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    cout << "Database loading " << size << " bytes" << endl;
    fseek(file, 0L, SEEK_SET);

    data = (uint8_t *) calloc(size, 1);

    fread(data, 1, size, file);
    fclose(file);
}
