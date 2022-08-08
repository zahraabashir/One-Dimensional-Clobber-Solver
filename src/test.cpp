#include <iostream>
#include "game.h"

using namespace std;

int main() {
    Game g1("BWBW");
    Game g2("WWWWB");

    cout << g1 << endl;
    cout << g1.chr(0) << endl;
    g1.chr(1) = 'B';
    cout << g1 << endl;

    cout << ((int) g1[0]) << endl;


    return 0;
}
