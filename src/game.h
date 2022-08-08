#pragma once

#include <cstdlib>
#include <string>


struct __GameCharView;

struct Game {
    char *data;
    int size;


    Game();
    Game(int size);
    Game(const std::string &boardText);
    Game(int size, char *board);
    Game(const Game &game);

    ~Game();

    char &operator[](int i);

    void resize(int newSize);
    void operator=(const Game &g);

    __GameCharView chr(int i);

};

struct __GameCharView {
    Game *g;
    int i;

    __GameCharView(Game *g, int i);
    operator char();
    void operator=(char c);
};



Game operator+(const Game &g1, const Game &g2);
Game operator-(const Game &g);

std::ostream &operator<<(std::ostream &os, const Game &g);
