#include <memory>
#include <string>

class State {
  private:

    //State **children;
    std::shared_ptr<State> *children; //array of shared pointers

    int *moves;
    size_t moveCount;


    State();
    void generateMoves(int idx = 0, int moveDepth = 0);

  public:
    std::string board;


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
