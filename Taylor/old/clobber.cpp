#include <iostream>
#include <memory>

using namespace std;

#define EMPTY 0
#define E 0
#define BLACK 1
#define B 1
#define WHITE 2
#define W 2


class State;
std::ostream &operator<<(std::ostream &, const std::pair<int, int> &p);
std::ostream &operator<<(std::ostream &, const State &);
int charToPlayerNumber(char);
char playerNumberToChar(int);
char opponentChar(char);
int opponentNumber(int);

bool solveOr(State *);
bool solveAnd(State *);

//TODO should use shared pointers
class State {
  public:
    string board;
    int toPlay;
    char toPlayChar;
    int result;


    size_t numberOfChildren;
    State **childList;

  private:
    std::pair<int, int> *moveList;
    size_t numberOfMoves;




    void generateMoves(int idx = 0, size_t moves = 0) {
        if ((size_t) idx > board.length()) {
            moveList = new std::pair<int, int>[moves];
            numberOfMoves = moves;
            return;
        }

        char *c1 = &board[idx - 1];
        char *c2 = &board[idx];
        char *c3 = &board[idx + 1];

        bool move1 = false;
        bool move2 = false;

        if (*c2 == toPlayChar) {
            move1 = idx > 0 && *c1 == opponentChar(toPlayChar);
            move2 = *c3 == opponentChar(toPlayChar);
        }

        generateMoves(idx + 1, moves + move1 + move2);

        if (move1) {
            moveList[moves] = std::pair<int, int>(idx, idx - 1);
        }
        if (move2) {
            moveList[moves + move1] = std::pair<int, int>(idx, idx + 1);
        }
    }

  public:
    State(string board, int toPlay) {
        this->board = board;
        this->toPlay = toPlay;
        this->toPlayChar = playerNumberToChar(toPlay);
        this->numberOfMoves = -1;
        this->numberOfChildren = -1;
        this->result = E;
    }

    State(const State &s) {
        this->board = s.board;
        this->toPlay = s.toPlay;
        this->toPlayChar = s.toPlayChar;
        this->result = s.result;

        this->numberOfMoves = s.numberOfMoves;
        if (s.numberOfMoves != (size_t) -1) {
            this->moveList = new std::pair<int, int>[s.numberOfMoves];

            for (size_t i = 0; i < this->numberOfMoves; i++) {
                this->moveList[i] = s.moveList[i];
            }
        }


        this->numberOfChildren = s.numberOfChildren;
        if (s.numberOfChildren != (size_t) -1) {
            this->childList = new State*[s.numberOfChildren];
            for (size_t i = 0; i < this->numberOfChildren; i++) {
                this->childList[i] = s.childList[i];
            }
        }

    }

    ~State() {
        if (numberOfMoves != (size_t) -1) {
            delete[] moveList;
        }

        if (numberOfChildren != (size_t) -1) {
            delete[] childList;
        }
    }

    const std::pair<int, int> *moves() {
        if (numberOfMoves != (size_t) -1) {
            return moveList;
        }

        generateMoves();
        return moveList;
    }

    int moveCount() {
        if (numberOfMoves != (size_t) -1) {
            return numberOfMoves;
        }

        generateMoves();
        return numberOfMoves;
    }

    void playMove(int from, int to) {
        if (numberOfMoves != (size_t) -1) {
            delete[] moveList;
            numberOfMoves = (size_t) -1;
        }

        if (numberOfChildren != (size_t) -1) {
            delete[] childList;
            numberOfChildren = (size_t) -1;
        }

        board[to] = board[from];
        board[from] = '.';

        toPlay = opponentNumber(toPlay);
        toPlayChar = playerNumberToChar(toPlay);

    }

    bool isTerminal() {
        return moveCount() == 0;
    }

    int getResult() {
        return result;
    }

    void expand() {
        if (numberOfChildren != (size_t) -1) {
            return;
        }

        size_t movec = moveCount();
        const std::pair<int, int> *moveArray = moves();

        childList = new State*[movec];

        for (size_t i = 0; i < movec; i++) {
            childList[i] = new State(*this);
            childList[i]->playMove(moveArray[i].first, moveArray[i].second);
        }

        numberOfChildren = movec;

    }


};

std::ostream &operator<<(std::ostream &os, const std::pair<int, int> &p) {
    os << "(" << p.first << " " << p.second << ")";
    return os;
}


std::ostream &operator<<(std::ostream &os, const State &b) {
    os << "[" << b.board << " " << playerNumberToChar(b.toPlay) << "]";
    return os;
}

char opponentChar(char c) {
    if (c == 'B') {
        return 'W';
    }
    if (c == 'W') {
        return 'B';
    }
    return '.';
}

int opponentNumber(int n) {
    if (n == B) {
        return W;
    }
    if (n == W) {
        return B;
    }
    return 0;
}


char playerNumberToChar(int p) {
    switch (p) {
        case E:
            return '.';
            break;
        case B:
            return 'B';
            break;
        case W:
            return 'W';
            break;
    }
    cerr << "Bad input for playerNumberToChar(): " << p << endl;
    return 'X';
}

int charToPlayerNumber(char c) {
    switch (c) {
        case '.':
            return E;
            break;
        case 'B':
            return B;
            break;
        case 'W':
            return W;
            break;
    }
    cerr << "Bad input for charToPlayerNumber(): " << (int) c << endl;
    return -1;
}

int rootPlayer;

bool solveOr(State *s) {
    if (s->result == rootPlayer) {
        return true;
    }

    s->expand();
    State **children = s->childList;

    for (size_t i = 0; i < s->numberOfChildren; i++) {
        bool win = solveAnd(children[i]);
        if (win) {
            return true;
        }
    }
    return false;
}

bool solveAnd(State *s) {
    if (s->result == rootPlayer) {
        return true;
    }

    s->expand();
    State **children = s->childList;

    for (size_t i = 0; i < s->numberOfChildren; i++) {
        bool win = solveOr(children[i]);
        if (!win) {
            return false;
        }
    }
    return true;
}



// ? None 0.9999 12345 
int main(int argc, char **argv) {
    if (argc < 4) {
        cout << "Usage:\n./clobber <position> <toPlay> <seconds> <debug>" << endl;
        return 0;
    }

    if (argc < 5 || *argv[4] != '1') {
        State *root = new State(argv[1], charToPlayerNumber(*argv[2]));
        rootPlayer = charToPlayerNumber(*argv[2]);

        bool win = solveOr(root);

        if (win) {
            cout << playerNumberToChar(rootPlayer);
        } else {
            cout << playerNumberToChar(opponentNumber(rootPlayer));
        }
        cout << endl;

        return 0;
    }

    State *root = new State(argv[1], charToPlayerNumber(*argv[2]));
    cout << *root << endl;
    rootPlayer = charToPlayerNumber(*argv[2]);
    cout << solveOr(root) << endl;



    return 0;

}
