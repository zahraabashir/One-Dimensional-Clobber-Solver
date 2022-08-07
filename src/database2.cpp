#include <iostream>
#include <stdio.h>
#include <algorithm>

#include "database2.h"

using namespace std;

size_t countShape(const vector<int> &shape) {
    size_t count = 0;


    size_t bits = 0;
    for (int chunkSize : shape) {
        bits += chunkSize;
    }

    uint64_t minGame = 0;

    //maxGame is (2 ** bits) - 1
    uint64_t maxGame = (1 << bits) - 1;


    count = maxGame - minGame + 1;

    return count;
}

void Database::genShapes(ShapeNode *node, int prev, const vector<int> &shape, int remaining) {
    if (remaining <= 0) {
        return;
    }

    int bound = prev == -1 ? remaining : std::min(remaining, prev);

    for (int i = 2; i <= bound; i++) {
        vector<int> s = shape;
        s.push_back(i);

        ShapeNode *node2 = new ShapeNode();
        node2->shape = s;
        node2->id = shapeToID(node2->shape);
        node2->bits = 0;
        for (int chunkSize : node2->shape) {
            node2->bits += chunkSize;
        }
        node2->entryCount = countShape(s);
        node->children.push_back(node2);

        genShapes(node2, i, s, remaining - i - 1);
    }

}

void printTree(ShapeNode *node) {
    vector<int> &shape = node->shape;

    if (shape.size() > 0) {
        cout << "[";
        for (int i = 0; i < shape.size(); i++) {
            cout << shape[i];
            if (i + 1 < shape.size()) {
                cout << ", ";
            }
        }
        cout << "] ";
        cout << "(" << node->entryCount << ") ";
        cout << "#" << node->id;
        cout << endl;
    }

    for (ShapeNode *child : node->children) {
        printTree(child);
    }
}

void Database::initShapeTree() {
    shapeTree = new ShapeNode();
    shapeTree->id = 0;
    shapeTree->bits = 0;
    shapeTree->entryCount = 0;

    vector<int> emptyShape;
    genShapes(shapeTree, -1, emptyShape, DB_MAX_BITS);

    //printTree(shapeTree);

}

void Database::visitShapeTree(ShapeNode *node) {
    gameEntries += node->entryCount;

    if (node->shape.size() != 0) {
        shapeIndexEntries += 1;
    }

    for (ShapeNode *child : node->children) {
        visitShapeTree(child);
    }
}

//void Database::revisitShapeTree(ShapeNode *node, char **shapeIndex, uint64_t *tableOffset) {
//    if (node->shape.size() > 0) {
//        (*shapeIndex)[0] = node->id;
//        (*shapeIndex)[1] = *tableOffset;
//
//        *shapeIndex += 2 * sizeof(uint64_t);
//        *tableOffset += node->entryCount * DB_ENTRY_SIZE;
//    }
//
//    for (ShapeNode *child : node->children) {
//        revisitShapeTree(child, shapeIndex, tableOffset);
//    }
//}

void Database::revisitShapeTree(ShapeNode *node, vector<ShapeNode *> &nodeList) {
    if (node->shape.size() > 0) {
        nodeList.push_back(node);
    }

    for (ShapeNode *child : node->children) {
        revisitShapeTree(child, nodeList);
    }
}

bool nodeListSort(const ShapeNode *x1, const ShapeNode *x2) {
    return x1->id < x2->id;
}

void Database::initMemory() {
    shapeIndexEntries = 0;
    gameEntries = 0;

    visitShapeTree(shapeTree);

    cout << "Shape index entries: " << shapeIndexEntries << endl;
    cout << "Game entries: " << gameEntries << endl;

    uint64_t shapeIndexHeaderBytes = sizeof(uint32_t);
    uint64_t shapeIndexBytes = shapeIndexEntries * (2 * sizeof(uint64_t));
    uint64_t gameEntryBytes = DB_ENTRY_SIZE * gameEntries;

    totalBytes = shapeIndexHeaderBytes + shapeIndexBytes + gameEntryBytes;
   


    cout << "Total bytes: " << totalBytes << endl;

    //data = new char[totalBytes];
    data = (char *) calloc(totalBytes, 1);

    *((uint32_t *) data) = shapeIndexEntries;

    uint64_t tableOffset = 0;
    uint64_t *shapeIndex = (uint64_t *) (data + sizeof(uint32_t));

    vector<ShapeNode *> nodeList;

    //revisitShapeTree(shapeTree, &shapeIndex, &tableOffset);
    revisitShapeTree(shapeTree, nodeList);
    sort(nodeList.begin(), nodeList.end(), nodeListSort);

    for (ShapeNode *node : nodeList) {
        shapeIndex[0] = node->id;
        shapeIndex[1] = tableOffset;


        shapeIndex += 2;
        tableOffset += node->entryCount * DB_ENTRY_SIZE;
    }

    table = data + sizeof(uint32_t) + (*((uint32_t *) data)) * (2 * sizeof(uint64_t));

    // shape index (map from shape ID to entries)
    // entries
}

uint64_t shapeToID(vector<int> &shape) {
    constexpr uint64_t shift = _shiftAmount();

    uint64_t id = 0;

    uint64_t exponent = 1;
    for (int chunk : shape) {
        id += chunk * exponent;
        exponent <<= shift;
    }

    return id;
}

uint64_t shapeDataToID(const vector<pair<int, char *>> &shapeData) {
    constexpr uint64_t shift = _shiftAmount();

    uint64_t id = 0;

    uint64_t exponent = 1;
    for (const pair<int, char *> &chunk : shapeData) {
        id += chunk.first * exponent;
        exponent <<= shift;
    }

    return id;
}

ShapeNode::~ShapeNode() {
    for (ShapeNode *n : children) {
        delete n;
    }
}

void Database::initData() {
    initShapeTree();
    initMemory();
}

Database::Database() {
    data = NULL;
    shapeTree = NULL;
}


Database::~Database() {
    if (data != NULL) {
        free(data);
    }

    if (shapeTree != NULL) {
        delete shapeTree;
    }
}

void Database::load() {
    file = fopen("database2.bin", "r+");

    fseek(file, 0L, SEEK_END);
    totalBytes = ftell(file);
    fseek(file, 0L, SEEK_SET);

    data = new char[totalBytes];

    fread(data, 1, totalBytes, file);
    fclose(file);

    table = data + sizeof(uint32_t) + (*((uint32_t *) data)) * (2 * sizeof(uint64_t));
}

void Database::save() {
    file = fopen("database2.bin", "r+");
    fwrite(data, 1, totalBytes, file);
    fclose(file);
}

vector<pair<int, int>> getChunks(int len, char *board) {
    vector<pair<int, int>> chunks;

    int start = -1;

    for (int i = 0; i < len; i++) {
        if (board[i] != 0 && start == -1) {
            start = i;
        }

        if (board[i] == 0 && start != -1) {
            chunks.push_back(pair<int, int>(start, i));
            start = -1;
        }

    }

    if (start != -1) {
        chunks.push_back(pair<int, int>(start, len));
    }

    return chunks;
}

bool shapeDataSort(const pair<int, char *> &x1, const pair<int, char *> &x2) {
    return x1.first > x2.first;
}

uint64_t Database::searchShapeIndex(uint64_t shapeID) {
    uint64_t *base = (uint64_t *) (data + sizeof(uint32_t));
    uint32_t elements = *((uint32_t *) data);

    int low = 0;
    int high = elements - 1;

    while (low <= high) {
        int i = (low + high) / 2;


        uint64_t id = base[i * 2];



        if (id == shapeID) {
            return base[i * 2 + 1];
        }

        if (id > shapeID) { //need to look lower
            high = i - 1;
        } else { //need to look higher
            low = i + 1;
        }
    }

    //Not found; this shouldn't happen...
    return ((uint64_t) 0) - 1;
}

uint64_t Database::getIdx(int len, char *board) {
    vector<pair<int, int>> chunks = getChunks(len, board);
    vector<pair<int, char *>> shapeData;

    for (const pair<int, int> &sg : chunks) {
        int l = sg.second - sg.first;
        char *p = board + sg.first;

        shapeData.push_back(pair<int, char *>(l, p));
    }

    sort(shapeData.begin(), shapeData.end(), shapeDataSort);

    uint64_t id = shapeDataToID(shapeData);
    uint64_t offset = searchShapeIndex(id); //offset to get to the correct table section

    if (offset == (((uint64_t) 0) - 1)) {
        return offset;
    }

    int index = 0;
    int power = 1;
    for (const pair<int, char *> &chunk : shapeData) {
        for (int i = 0; i < chunk.first; i++) {
            index += chunk.second[i] == 2 ? power : 0;
            power *= 2;
        }
    }

    return offset + (index * DB_ENTRY_SIZE);
}

unsigned char *Database::get(int len, char *board) {
    if (len > DB_MAX_BITS) {
        return 0;
    }

    uint64_t idx = getIdx(len, board);

    if (idx == ((uint64_t) 0) - 1) {
        return 0;
    }

    return (unsigned char *) (table + idx);
}

//This function is used by simplify() in the solver to get entries from links
unsigned char *Database::getFromIdx(int idx) {
    return (unsigned char *) (table + idx);
}
