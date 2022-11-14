#include <iostream>

#include "database3.h"

using namespace std;

int main(int argc, char **argv) {
    string fileName = "database3.bin";

    if (argc >= 2) {
        fileName = string(argv[1]);
    }

    cout << "Loading from " << fileName << endl;

    Database db;
    db.loadFrom(fileName.c_str());

}
