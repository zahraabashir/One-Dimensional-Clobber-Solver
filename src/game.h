#pragma once

#include <cstdlib>
#include <string>
#include <vector>


struct __GameCharView;

struct Game {
    char *data;
    int size;


    Game();
    Game(int size);
    Game(const std::string &boardText);
    Game(int size, char *board);
    Game(const Game &game);
    Game(const std::vector<int> &shape);
    Game(const std::vector<int> &shape, int number);

    ~Game();

    char &operator[](int i);

    void resize(int newSize);
    void operator=(const Game &g);



    __GameCharView chr(int i);


    void play(int from, int to, char *undoBuffer);
    void undo(char *undoBuffer);
    int hash(int player);
    std::vector<std::pair<int, int>> moves(int player);

  private:
    void __generateMoves(const int &player, const int &opponent,
        int idx, int moveDepth, std::vector<std::pair<int, int>> &moves);


};

struct __GameCharView {
    Game *g;
    int i;

    __GameCharView(Game *g, int i);
    operator char();
    void operator=(char c);
};



Game operator+(const Game &g1, const Game &g2);
Game operator+(const Game &g1, const std::string &str);
Game operator+(const Game &g1, char c);
Game operator-(const Game &g1, const Game &g2);
Game operator-(const Game &g);

std::ostream &operator<<(std::ostream &os, const Game &g);
std::istream &operator>>(std::istream &is, Game &g);
