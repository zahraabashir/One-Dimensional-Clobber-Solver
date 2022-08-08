#include <iostream>
#include "game.h"
#include "utils.h"

using namespace std;

int main() {

    Game g1("BWBW");
    cout << g1 << endl;

    g1 = g1 + 1;
    cout << g1 << endl;



    


    return 0;
}
