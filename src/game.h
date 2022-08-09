#pragma once

#include <cstdlib>
#include <string>
#include <vector>


struct __GameCharView;
struct __GameCharViewConst;

struct Game {
    char *data;
    int size;

    Game();
    Game(int size);
    Game(const std::string &boardText);
    Game(int size, const char *board);
    Game(const Game &game);
    Game(const std::vector<int> &shape);
    Game(const std::vector<int> &shape, int number);

    ~Game();

    char &operator[](int i);
    const char &operator[](int i) const;

    void resize(int newSize);
    void operator=(const Game &g);

    __GameCharView chr(int i);
    __GameCharViewConst chr(int i) const;


    void play(int from, int to, char *undoBuffer);
    void undo(const char *undoBuffer);
    int hash(int player) const;
    std::vector<std::pair<int, int>> moves(int player) const;

    std::vector<int> shape() const;
    int number() const;

    void operator+=(const Game &g);
    void operator-=(const Game &g);

    void operator+=(const std::string &str);
    void operator-=(const std::string &str);

    void operator+=(uint8_t c);
    void operator-=(uint8_t c);

    void negate();

  private:
    void __generateMoves(const int &player, const int &opponent,
        int idx, int moveDepth, std::vector<std::pair<int, int>> &moves) const;
};

Game operator+(const Game &g1, const Game &g2);
Game operator-(const Game &g1, const Game &g2);

Game operator+(const Game &g1, const std::string &str);
Game operator-(const Game &g1, const std::string &str);

Game operator+(const Game &g1, uint8_t c);
Game operator-(const Game &g1, uint8_t c);

Game operator-(const Game &g);


struct __GameCharView {
    char *data;
    int i;

    __GameCharView(char *data, int i);
    operator char() const;
    void operator=(char c);
};

struct __GameCharViewConst {
    const char *data;
    int i;

    __GameCharViewConst(const char *data, int i);
    operator char() const;
};



std::ostream &operator<<(std::ostream &os, const Game &g);
std::istream &operator>>(std::istream &is, Game &g);

