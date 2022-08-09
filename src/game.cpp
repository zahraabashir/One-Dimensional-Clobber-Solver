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

Game::Game(int size, const char *board) {
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

Game::Game(const vector<int> &shape) {
    size = 0;
    data = NULL;

    int newSize = shape.size() - 1;

    for (int chunkSize : shape) {
        newSize += chunkSize;
    }

    if (newSize <= 0) {
        return;
    }

    resize(newSize);
}

Game::Game(const vector<int> &shape, int number) {
    size = 0;
    data = NULL;

    int newSize = shape.size() - 1;

    for (int chunkSize : shape) {
        newSize += chunkSize;
    }

    if (newSize <= 0) {
        return;
    }

    resize(newSize);

    {
        int shapeIdx = 0;
        int currentChunk = 0;
        int shift = 0;

        for (int i = 0; i < size; i++) {
            if (currentChunk >= shape[shapeIdx]) {
                currentChunk = 0;
                shapeIdx += 1;
                data[i] = 0;

                continue;
            }

            currentChunk += 1;

            data[i] = ((number >> shift) & 1) == 0 ? 1 : 2;
            shift += 1;
        }
    }

}

Game::~Game() {
    if (size > 0) {
        free(data);
    }
}

char &Game::operator[](int i) {
    return data[i];
}

const char &Game::operator[](int i) const {
    return data[i];
}

void Game::resize(int newSize) {
    if (size == newSize) {
        return;
    }

    char *newData = NULL;

    if (newSize > 0) {
        newData = (char *) calloc(newSize, 1);
    }

    memcpy(newData, data, min(size, newSize));

    if (size > 0) {
        free(data);
    }

    size = newSize;
    data = newData;
}

void Game::operator=(const Game &g) {
    resize(g.size);
    memcpy(data, g.data, size);
}

__GameCharView Game::chr(int i) {
    return __GameCharView(data, i);
}

__GameCharViewConst Game::chr(int i) const {
    return __GameCharViewConst(data, i);
}

void Game::play(int from, int to, char *undoBuffer) {
    if (undoBuffer != NULL) {
        int minIdx = std::min<int>(from, to);
        ((int *) undoBuffer)[0] = minIdx;
        ((char *) undoBuffer)[0 + sizeof(int)] = data[minIdx];
        ((char *) undoBuffer)[0 + sizeof(int) + 1] = data[minIdx + 1];
    }
    
    data[to] = data[from];
    data[from] = EMPTY;
}

void Game::undo(const char *undoBuffer) {
    int start = ((int *) undoBuffer)[0];
    data[start] = (char) undoBuffer[0 + sizeof(int)];
    data[start + 1] = (char) undoBuffer[0 + sizeof(int) + 1];
}

int Game::hash(int player) const {
    int result = 1 * (player - 1);
    int cumulativePower = 2;

    for (int i = 0; i < size; i++) {
        result += cumulativePower * data[i];
        cumulativePower *= 3;
    }
    return result;
}

vector<pair<int, int>> Game::moves(int player) const {
    int opponent = opponentNumber(player);
    vector<pair<int, int>> moves(0);
    __generateMoves(player, opponent, 0, 0, moves);
    return moves;
}

vector<int> Game::shape() const {
    vector<int> shape;

    int chunkSize = 0;

    for (int i = 0; i < size; i++) {
        if (data[i] != 0) {
            chunkSize += 1;
        } else {
            if (chunkSize > 0) {
                shape.push_back(chunkSize);
                chunkSize = 0;
            }
        }
    }

    if (chunkSize > 0) {
        shape.push_back(chunkSize);
    }

    return shape;
}

int Game::number() const {
    int val = 0;

    int power = 1;
    for (int i = 0; i < size; i++) {
        if (data[i] == 0) {
            continue;
        }

        val += data[i] == 2 ? 1 : 0;
        power *= 2;
    }

    return val;
}

void Game::operator+=(const Game &g) {
    int space = (size != 0 && g.size != 0) ? 1 : 0;
    int newSize = size + g.size + space;
    int oldSize = size;

    resize(newSize);

    memcpy(data + oldSize + space, g.data, g.size);

}

void Game::operator-=(const Game &g) {
    int space = (size != 0 && g.size != 0) ? 1 : 0;
    int newSize = size + g.size + space;
    int oldSize = size;

    resize(newSize);

    for (int i = 0; i < g.size; i++) {
        data[oldSize + space + i] = opponentNumber(g.data[i]);
    }

}

void Game::operator+=(const std::string &str) {
    int newSize = size + str.size();
    int oldSize = size;
    resize(newSize);

    for (int i = 0; i < str.size(); i++) {
        data[oldSize + i] = charToPlayerNumber(str[i]);
    }
}

void Game::operator-=(const std::string &str) {
    int newSize = size + str.size();
    int oldSize = size;
    resize(newSize);

    for (int i = 0; i < str.size(); i++) {
        data[oldSize + i] = opponentNumber(charToPlayerNumber(str[i]));
    }
}

void Game::operator+=(uint8_t c) {
    resize(size + 1);
    data[size - 1] = c;
}

void Game::operator-=(uint8_t c) {
    resize(size + 1);
    data[size - 1] = opponentNumber(c);
}

void Game::negate() {
    for (int i = 0; i < size; i++) {
        data[i] = opponentNumber(data[i]);
    }
}

void Game::__generateMoves(const int &player, const int &opponent,
    int idx, int moveDepth, vector<pair<int, int>> &moves) const {

    if (idx >= size) {
        moves.resize(moveDepth);
        return;
    }

    char *c2 = &data[idx];
    bool move1 = false;
    bool move2 = false;

    if (*c2 == player) {
        char *c1 = &data[idx - 1];
        char *c3 = &data[idx + 1];
        move1 = idx > 0 && *c1 == opponent;
        move2 = (idx + 1 < size) && *c3 == opponent;
    }

    __generateMoves(player, opponent, idx + 1, moveDepth + move1 + move2, moves);

    if (move1) {
        moves[moveDepth] = pair<int, int>(idx, idx - 1);
    }
    if (move2) {
        moves[moveDepth + move1] = pair<int, int>(idx, idx + 1);
    }
}

Game operator+(const Game &g1, const Game &g2) {
    int space = (g1.size != 0 && g2.size != 0) ? 1 : 0;
    int newSize = g1.size + g2.size + space;

    Game g(newSize);

    memcpy(g.data, g1.data, g1.size);
    memcpy(g.data + g1.size + space, g2.data, g2.size);

    return g;
}

Game operator-(const Game &g1, const Game &g2) {
    Game g = g1 + g2;

    int space = (g1.size != 0 && g2.size != 0);

    for (int i = 0; i < g2.size; i++) {
        g[g1.size + space + i] = opponentNumber(g2[i]);
    } 

    return g;
}


Game operator+(const Game &g1, const string &str) {
    Game g2(g1.size + str.size());

    memcpy(g2.data, g1.data, g1.size);
    for (int i = 0; i < str.size(); i++) {
        g2.data[g1.size + i] = charToPlayerNumber(str[i]);
    }

    return g2;
}

Game operator-(const Game &g1, const string &str) {
    Game g2(g1.size + str.size());

    memcpy(g2.data, g1.data, g1.size);
    for (int i = 0; i < str.size(); i++) {
        g2.data[g1.size + i] = opponentNumber(charToPlayerNumber(str[i]));
    }

    return g2;
}

Game operator+(const Game &g1, uint8_t c) {
    Game g2(g1.size + 1);

    memcpy(g2.data, g1.data, g1.size);
    g2.data[g1.size] = c;

    return g2;
}

Game operator-(const Game &g1, uint8_t c) {
    Game g2(g1.size + 1);

    memcpy(g2.data, g1.data, g1.size);
    g2.data[g1.size] = c;

    return g2;
}

Game operator-(const Game &g) {
    Game g2(g.size);

    for (int i = 0; i < g.size; i++) {
        g2.data[i] = opponentNumber(g.data[i]);
    }

    return g2;
}

////////////////////////////// __GameCharView
__GameCharView::__GameCharView(char *data, int i) {
    this->data = data;
    this->i = i;
}

__GameCharView::operator char() const {
    return playerNumberToChar(data[i]);
}

void __GameCharView::operator=(char c) {
    data[i] = charToPlayerNumber(c);
}

////////////////////////////// __GameCharViewConst
__GameCharViewConst::__GameCharViewConst(const char *data, int i) {
    this->data = data;
    this->i = i;
}

__GameCharViewConst::operator char() const {
    return playerNumberToChar(data[i]);
}

////////////////////////////// stream operators
ostream &operator<<(ostream &os, const Game &g) {
    for (int i = 0; i < g.size; i++) {
        os << playerNumberToChar(g.data[i]);
    }

    return os;
}

istream &operator>>(istream &is, Game &g) {
    string str;
    is >> str;

    g.resize(str.size());

    for (int i = 0; i < str.size(); i++) {
        g.data[i] = charToPlayerNumber(str[i]);
    }

    return is;
}
