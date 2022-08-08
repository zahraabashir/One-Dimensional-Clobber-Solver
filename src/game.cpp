#include "game.h"

#include <iostream>
#include <cstring>
#include "utils.h"

using namespace std;

////////////////////////////// Game
Game::Game() {
    size = 0;
    data = NULL;
}

Game::Game(int size) {
    this->size = 0;
    data = NULL;

    resize(size);
}

Game::Game(const string &boardText) {
    size = 0;
    data = NULL;

    resize(boardText.size());

    for (int i = 0; i < size; i++) {
        data[i] = charToPlayerNumber(boardText[i]);
    }
}

Game::Game(int size, char *board) {
    this->size = 0;
    data = NULL;

    resize(size);
    memcpy(data, board, size);
}

Game::Game(const Game &game) {
    size = 0;
    data = NULL;

    resize(game.size);
    memcpy(data, game.data, size);
}

Game::~Game() {
    if (size > 0) {
        free(data);
    }
}

char &Game::operator[](int i) {
    return data[i];
}

void Game::resize(int newSize) {
    if (size == newSize) {
        return;
    }

    if (size > 0) {
        free(data);
    }

    size = newSize;

    if (size == 0) {
        data = NULL;
        return;
    }

    data = (char *) calloc(size, 1);
}

void Game::operator=(const Game &g) {
    resize(g.size);
    memcpy(data, g.data, size);
}

Game operator+(const Game &g1, const Game &g2) {
    int space = (g1.size != 0 && g2.size != 0) ? 1 : 0;
    int newSize = g1.size + g2.size + space;

    Game g(newSize);

    memcpy(g.data, g1.data, g1.size);
    memcpy(g.data + g1.size + space, g2.data, g2.size);

    return g;
}

Game operator-(const Game &g) {
    Game g2(g.size);

    for (int i = 0; i < g.size; i++) {
        g2.data[i] = opponentNumber(g.data[i]);
    }

    return g2;
}

__GameCharView Game::chr(int i) {
    return __GameCharView(this, i);
}

////////////////////////////// __GameCharView
__GameCharView::__GameCharView(Game *g, int i) {
    this->g = g;
    this->i = i;
}

__GameCharView::operator char() {
    return playerNumberToChar(g->data[i]);
}

void __GameCharView::operator=(char c) {
    g->data[i] = charToPlayerNumber(c);
}

////////////////////////////// ostream operators
ostream &operator<<(ostream &os, const Game &g) {
    for (int i = 0; i < g.size; i++) {
        os << playerNumberToChar(g.data[i]);
    }

    return os;
}

