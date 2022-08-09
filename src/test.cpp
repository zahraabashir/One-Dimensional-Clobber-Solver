#include <iostream>
#include "game.h"
#include "utils.h"

using namespace std;

int main() {


    Bitvector b;

    cout << b << endl << endl;

    b[0] = 1;

    cout << b << endl << endl;
    

    b[2] = 1;

    cout << b << endl << endl;

    b[127] = 1;
    cout << b << endl << endl;

    b[63] = 1;
    cout << b << endl << endl;

    b[64] = 1;
    cout << b << endl << endl;

    b[65] = 1;
    cout << b << endl << endl;

    Bitvector b2;

    cout << b2 << endl << endl;

    b2 = b;
    cout << b2 << endl << endl;

    


    return 0;
}
