#include <iostream>

#include "database2.h"

using namespace std;

int main() {
    cout << "Shift amount for shapes is " << _shiftAmount() << endl;


    vector<int> shape;

    shape.push_back(3);
    cout << countShape(shape) << endl;

    shape.push_back(2);
    cout << countShape(shape) << endl;

    shape.push_back(3);
    cout << countShape(shape) << endl;


    Database db;
    db.initData();

    char board[] = {1, 2, 2, 0, 2, 1, 0};
    unsigned char *ptr = db.get(6, board);

    cout << ((uint64_t *) ptr) << endl;


    return 0;
}
