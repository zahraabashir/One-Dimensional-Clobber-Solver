#include <iostream>
#include "game.h"

using namespace std;

int main() {
    Game g1("BWBW");
    Game g2("WWWWB");

    cout << g1 << endl;

    g1.chars()[3] = 'B';
    cout << g1 << endl;

    cout << g1.chars() << endl;

    cout << g1.chars()[2] << endl;

    return 0;
}
