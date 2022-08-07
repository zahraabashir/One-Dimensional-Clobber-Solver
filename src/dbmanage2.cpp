#include <iostream>
#include <vector>
#include <algorithm>

#include "database2.h"
#include "utils.h"

using namespace std;

Database *db = NULL;

void getShapeList(ShapeNode *node, vector<ShapeNode *> &list) {
    if (node->shape.size() > 0) {
        list.push_back(node);
    }

    for (ShapeNode *child : node->children) {
        getShapeList(child, list);
    }
}

bool shapeListSort(const ShapeNode *n1, const ShapeNode *n2) {
    if (n1->bits < n2->bits) {
        return true;
    }

    for (int i = 0; i < min(n1->shape.size(), n2->shape.size()); i++) {
        if (n1->shape[i] < n2->shape[i]) {
            return true;
        }

        if (n1->shape[i] > n2->shape[i]) {
            return false;
        }
    }

    return false;
}


void processBoard(int len, char *board, char *boardText) {
    unsigned char *entry = db->get(len, board);

    cout << ((uint64_t *) entry) << endl;

    if (entry == 0) {
        cout << "Failed to find entry for: " << boardText << endl;
        while (1) {
        }
    }

    if (DB_GET_OUTCOME(entry) != 0) {
        cout << "Outcome not 0 for: " << boardText << " (was " << DB_GET_OUTCOME(entry) << ")" << endl;
        while (1) {
        }
    }

    DB_SET_OUTCOME(entry, 1);
}

int main() {
    db = new Database();
    db->initData();

    //Get all shapes
    vector<ShapeNode *> shapeList;
    getShapeList(db->shapeTree, shapeList);
    sort(shapeList.begin(), shapeList.end(), shapeListSort);


    for (const ShapeNode *node : shapeList) {
        const vector<int> &shape = node->shape;

        int boardSize = node->bits + shape.size(); //shape.size() and not shape.size() - 1 because of null terminator

        char board[boardSize];

        char boardText[boardSize];
        boardText[boardSize - 1] = 0;

        int minGame = 0;
        int maxGame = (1 << node->bits) - 1;

        for (int number = minGame; number <= maxGame; number++) {
            board[boardSize - 1] = 0;
            boardText[boardSize - 1] = 0;

            {
                int shapeIdx = 0;
                int currentChunk = 0;
                int shift = 0;

                for (int i = 0; i < node->bits + shape.size() - 1; i++) {
                    if (currentChunk >= shape[shapeIdx]) {
                        currentChunk = 0;
                        shapeIdx += 1;
                        board[i] = 0;
                        boardText[i] = playerNumberToChar(board[i]);

                        continue;
                    }

                    currentChunk += 1;

                    board[i] = ((number >> shift) & 1) == 0 ? 1 : 2;
                    boardText[i] = playerNumberToChar(board[i]);
                    shift += 1;
                }
            }

            cout << shape << " |" << node->bits <<  "| (" << number << "/" << maxGame << ")" << endl;
            cout << boardText << endl;
            //Do something with this game
            processBoard(boardSize - 1, board, boardText);
            cout << endl;
        }

    }



    db->save();

    delete db;

    return 0;
}
