#include <iostream>
#include "game.h"

using namespace std;

int main() {
    Game g1("BWBW");
    Game g2("WWWWB");


    Game g3 = g1 + g2;
    cout << g3 << " " << g3.size << endl;

    Game g4 = g1 - g2;
    cout << g4 << " " << g4.size << endl;

    Game g5;
    cin >> g5;
    cout << g5;
    


    return 0;
}
