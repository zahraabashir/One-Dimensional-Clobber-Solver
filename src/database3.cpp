#include "database3.h"
#include "miscTypes.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <tuple>
#include <cassert>

using namespace std;

uint8_t *db_get_outcome(const uint8_t *entry) {
    if (entry == 0) {
        return 0;
    }
    return (uint8_t *) (entry + Offset<DBLayout, DB_OUTCOME>());
}

uint64_t *db_get_dominance(const uint8_t *entry) {
    if (entry == 0) {
        return 0;
    }
    return (uint64_t *) (entry + Offset<DBLayout, DB_DOMINANCE>());
}

int8_t *db_get_bounds(const uint8_t *entry) {
    if (entry == 0) {
        return 0;
    }
    return (int8_t *) (entry + Offset<DBLayout, DB_BOUNDS>());
}

uint64_t *db_get_metric(const uint8_t *entry) {
    if (entry == 0) {
        return 0;
    }
    return (uint64_t *) (entry + Offset<DBLayout, DB_METRIC>());
}

uint64_t *db_get_link(const uint8_t *entry) {
    if (entry == 0) {
        return 0;
    }
    return (uint64_t *) (entry + Offset<DBLayout, DB_LINK>());
}

uint64_t *db_get_shape(const uint8_t *entry) {
    if (entry == 0) {
        return 0;
    }
    return (uint64_t *) (entry + Offset<DBLayout, DB_SHAPE>());
}

uint32_t *db_get_number(const uint8_t *entry) {
    if (entry == 0) {
        return 0;
    }
    return (uint32_t *) (entry + Offset<DBLayout, DB_NUMBER>());
}

uint8_t *db_get_simplest_moves(const uint8_t *entry) {
    if (entry == 0) {
        return 0;
    }
    return (uint8_t *) (entry + Offset<DBLayout, DB_SIMPLEST_MOVES>());
}

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

uint64_t shapeDataToNumber(const vector<tuple<int, const uint8_t *>> &shape) {
    uint64_t num = 0;

    for (auto it = shape.rbegin(); it != shape.rend(); it++) {
        int chunk = std::get<0>(*it);

        num = (num << shapeNumberShift());
        num |= (chunk - 1);
    }

    return num;
}


vector<int> numberToShape(uint64_t number) {
    vector<int> shape;

    while (number) {
        int chunk = (number & shapeNumberMask()) + 1;
        assert(chunk > 1);
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
    //cout << "Making DB" << endl;
    _useIndirectLinks = false;
    _defaultIndirectLinksSize1 = 0;
    _defaultIndirectLinks1 = nullptr;
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

    //cout << "DB allocating " << size << " bytes" << endl;

    //Allocate space
    data = (uint8_t *) calloc(size, 1);

    /*
        Fill header/index
    */

    //Header...
    ((size_t *) data)[0] = indexEntryCount;
    ((size_t *) data)[1] = entryCount;

    //Index...
    index = (uint64_t *) (data + headerSize);

    size_t cumulativeEntries = 0;
    for (size_t i = 0; i < shapeList.size(); i++) {
        const vector<int> &shape = shapeList[i];

        uint64_t shapeNumber = shapeToNumber(shape);
        uint64_t offset = (headerSize + indexEntrySize * indexEntryCount) + cumulativeEntries * entrySize;

        uint64_t *entry = (uint64_t *) (((uint8_t *) index) + i * indexEntrySize);

        //cout << "Writing to " << (int) (entry - index) << endl;

        entry[0] = shapeNumber;
        entry[1] = offset;

        size_t sum = 0;
        for (int chunk : shape) {
            sum += chunk;
        }

        cumulativeEntries += (1 << sum);
    }
}

Database::~Database() {
    if (data != NULL) {
        free(data);
    }

    if (_defaultIndirectLinks1 != nullptr)
        delete[] _defaultIndirectLinks1;
}

//NOTE: doesn't update header
void Database::save(const char *fileName) {
    cout << "DB SAVE" << endl;
    file = fopen(fileName, "r+");
    fwrite(data, 1, size, file);
    fclose(file);
}

/*
void Database::load() {
    //cout << "DB LOAD" << endl;

    file = fopen("database3.bin", "r+");

    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    //cout << "Database loading " << size << " bytes" << endl;
    fseek(file, 0L, SEEK_SET);

    data = (uint8_t *) calloc(size, 1);

    fread(data, 1, size, file);
    fclose(file);

    indexEntryCount = ((size_t *) data)[0];
    entryCount = ((size_t *) data)[1];
    index = (uint64_t *) (data + headerSize);
}
*/

void Database::loadFrom(const char *fileName) {
    //cout << "DB LOAD" << endl;

    file = fopen(fileName, "r+");

    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    //cout << "Database loading " << size << " bytes" << endl;
    fseek(file, 0L, SEEK_SET);

    data = (uint8_t *) calloc(size, 1);

    fread(data, 1, size, file);
    fclose(file);

    indexEntryCount = ((size_t *) data)[0];
    entryCount = ((size_t *) data)[1];
    index = (uint64_t *) (data + headerSize);
}

// (chunkSize, start)
vector<tuple<int, const uint8_t *>> computeShapeData(const uint8_t *board, size_t len) {
    vector<tuple<int, const uint8_t *>> shape;

    int chunk = 0;
    const uint8_t *ptr = board;    

    for (size_t i = 0; i < len; i++) {
        if (board[i] != 0) {
            if (chunk == 0) { //new chunk
                ptr = board + i;
            }

            chunk += 1;
        } else {
            if (chunk > 1) { //complete chunk
                shape.push_back(tuple<int, const uint8_t *>(chunk, ptr));
            }
            chunk = 0;
        }
    }

    if (chunk > 1) {
        shape.push_back(tuple<int, const uint8_t *>(chunk, ptr));
    }

    return shape;
}

vector<int> getShape(const uint8_t *board, size_t len) {
    vector<int> shape;

    int chunk = 0;

    for (size_t i = 0; i < len; i++) {
        if (board[i] != 0) {
            if (chunk == 0) { //new chunk
                //ptr = board + i;
            }

            chunk += 1;
        } else {
            if (chunk >= 1) { //complete chunk
                shape.push_back(chunk);
            }
            chunk = 0;
        }
    }

    if (chunk >= 1) {
        shape.push_back(chunk);
    }

    return shape;
}

uint64_t Database::getIdxDirect(const uint8_t *board, size_t len) {
    if (len > DB_MAX_BITS) {
        return DB_NOT_FOUND;
    }

    //Compute shape of given game and sort it
    vector<tuple<int, const uint8_t *>> shapeData = computeShapeData(board, len);
    sort(shapeData.begin(), shapeData.end(),
        [](const tuple<int, const uint8_t *> &s1, const tuple<int, const uint8_t *> &s2) {
            return std::get<0>(s1) > std::get<0>(s2);
        }
    );

    if (shapeData.size() == 0) {
        return DB_NOT_FOUND;
    }

    //Get shape number
    uint64_t snum = shapeDataToNumber(shapeData);

    //Binary search index to find table section
    int low = 0;
    int high = indexEntryCount - 1;
    uint64_t sectionOffset = DB_NOT_FOUND;


    while (low <= high) {
        int i = (low + high) / 2;

        uint64_t x = index[2 * i];

        if (x < snum) {
            low = i + 1;
        } else if (x > snum) {
            high = i - 1;
        } else {
            sectionOffset = index[2 * i + 1];
            break;
        }
    }

    if (sectionOffset == DB_NOT_FOUND) {
        cerr << "DB NOT FOUND for valid size -- shouldn't happen" << endl;
        cout << len << endl;
        cout << "|";
        for (size_t i = 0; i < len; i++)
            cout << (unsigned int) board[i];
        cout << "|" << endl;

        assert(sectionOffset != DB_NOT_FOUND);
        return DB_NOT_FOUND;
    }


    //Compute offset into table section
    uint64_t relativeOffset = 0;
    uint64_t cumulativePower = 1;

    for (const tuple<int, const uint8_t *> &sd : shapeData) {
        const int &chunk = std::get<0>(sd);
        const uint8_t *ptr = std::get<1>(sd);

        for (int i = 0; i < chunk; i++) {
            assert(*ptr > 0 && *ptr <= 2);
            relativeOffset += cumulativePower * ((*ptr) - 1);
            cumulativePower *= 2;
            ptr += 1;
        }
    }

    return sectionOffset + relativeOffset * entrySize;
}

uint64_t Database::getIdxDirect(const Subgame &sg) {
    return getIdxDirect(sg.board(), sg.size());
}

uint8_t *Database::get(const uint8_t *board, size_t len) {
    uint64_t idx = getIdxDirect(board, len);
    //cout << "IDX " << idx << endl;
    if (idx == DB_NOT_FOUND) {
        return 0;
    }

    return data + idx;
}

uint8_t *Database::get(const Subgame &sg) {
    return get(sg.board(), sg.size());
}

uint8_t *Database::getFromIdx(uint64_t idx) {
    if (idx == DB_NOT_FOUND) {
        return 0;
    }

    if (!_useIndirectLinks)
        return data + idx;

    const IndirectLink *indirect = ((const IndirectLink*) idx);
    return data + indirect->directLink;
}

uint8_t *Database::getFromIndirectIdx(const IndirectLink &indirect) {
    return getFromIdx((uint64_t) &indirect);
}

void Database::enableIndirectLinks() {
    assert(!_useIndirectLinks);
    assert(_defaultIndirectLinks1 == nullptr);

    _useIndirectLinks = true;
    _defaultIndirectLinksSize1 = entryCount;
    _defaultIndirectLinks1 = new IndirectLink[_defaultIndirectLinksSize1];
}

void Database::finalizeIndirectLinks() {
    assert(_useIndirectLinks);
    assert(_defaultIndirectLinks1 != nullptr);

    GameGenerator gen;

    while (gen) {
        GeneratedGame genGame = gen.generate();
        ++gen;

        uint8_t *entry = get(*genGame.game);

        {
            uint64_t link = *db_get_link(entry);
            IndirectLink *indirectLink1 = (IndirectLink*) link;
            uint64_t directLink1 = indirectLink1->directLink;
            *db_get_link(entry) = directLink1;
        }
    }

    _useIndirectLinks = false;
}
