#include <iostream>
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

    vector<int> emptyShape;
    genShapes(shapeTree, -1, emptyShape, DB_MAX_BITS);

    printTree(shapeTree);

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

void Database::initMemory() {
    shapeIndexEntries = 0;
    gameEntries = 0;

    visitShapeTree(shapeTree);

    cout << "Shape index entries: " << shapeIndexEntries << endl;
    cout << "Game entries: " << gameEntries << endl;
    


    // shape index
    // lookup tree 
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

ShapeNode::~ShapeNode() {
    for (ShapeNode *n : children) {
        delete n;
    }
}

Database::Database() {
    initShapeTree();
    initMemory();
}


Database::~Database() {
    delete shapeTree;
}

void Database::load() {
}

void Database::save() {
}




unsigned char *Database::get(int len, char *board) {
    return NULL;
}

unsigned char *Database::getFromIdx(int idx) {
    return NULL;
}
