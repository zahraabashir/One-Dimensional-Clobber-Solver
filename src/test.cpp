#include <iostream>
#include "utils.h"
#include "database3.h"

using namespace std;

int main() {
    Database db;
    db.init();

    cout << "-1 is " << (uint64_t) -1 << endl;

    uint8_t arr[] = {1, 2, 1, 2, 0, 2, 1};
    size_t len = sizeof(arr) / sizeof(uint8_t);


    uint64_t idx = db.getIdx(arr, len);
    cout << idx << endl;




    return 0;
}
