#pragma once

#include <memory>
#include <string>

class State {
  private:

    void zeroPointers();

  public:

    int *moves;


    State();
    void generateMoves(int idx = 0, int moveDepth = 0);

    std::string board;
    size_t moveCount;


    //State **children;
    std::shared_ptr<State> *children; //array of shared pointers

    int playerNumber;
    char playerChar;

    int outcome;

    State(std::string board, int playerNumber);
    ~State();
    int code();
    bool operator==(const State &s);
    State *play(int from, int to);

    const int *getMoves();


    void expand();
    bool isTerminal();


};

std::ostream &operator<<(std::ostream &os, const State &s);

