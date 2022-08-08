#pragma once

#include <cstdlib>
#include <string>


struct __GameCharView;
struct __GameCharViewBracket;

struct Game {
    char *data;
    int size;


    Game();
    Game(int size);
    Game(const std::string &boardText);
    Game(int size, char *board);
    Game(const Game &game);

    ~Game();

    void resize(int newSize);
    void operator=(const Game &g);

    __GameCharView chars();

};

struct __GameCharView {
  public:
    Game *g;

    __GameCharView(Game *g);
    __GameCharViewBracket operator[](int i);
};

struct __GameCharViewBracket {
  public:
    Game *g;
    int i;

    __GameCharViewBracket(Game *g, int i);
    operator char();
    void operator=(char c);

};




Game operator+(const Game &g1, const Game &g2);
Game operator-(const Game &g);

std::ostream &operator<<(std::ostream &os, const Game &g);
std::ostream &operator<<(std::ostream &os, const __GameCharView &view);
std::ostream &operator<<(std::ostream &os, const __GameCharViewBracket &view);
