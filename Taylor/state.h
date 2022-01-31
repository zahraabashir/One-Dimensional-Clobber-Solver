#pragma once

#include <memory>
#include <string>

class State {
  private:
    int *generateMoves(const int &player, const int &opponent, size_t *moveCount, int idx = 0, int moveDepth = 0);
    State();

  public:
    int *board;
    int boardSize;
    //int player;

    State(std::string board, int player);
    ~State();
    int code();
    bool operator==(const State &s);
    void play(int from, int to, char *undoBuffer);
    void undo(char *undoBuffer);
    int *getMoves(const int &player, const int &opponent, size_t *moveCount);
};

std::ostream &operator<<(std::ostream &os, const State &s);

