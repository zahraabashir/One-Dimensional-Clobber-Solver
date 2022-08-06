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

    db.load();
    db.save();


    return 0;
}
