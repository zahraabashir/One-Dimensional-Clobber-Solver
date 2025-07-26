#include <algorithm>
#include <cstring>

#include "miscTypes.h"
#include "state.h"
#include "utils.h"
#include "database3.h"
#include "zobrist.h"

using namespace std;

namespace {

inline bool shouldMirror(const vector<uint8_t> &board) {
    const size_t N = board.size();

    for (size_t i = 0; i < N; i++) {
        const char &c1 = board[i];
        const char &c2 = board[N - 1 - i];

        if (c1 != c2)
            return c2 > c1;
    }

    return false;
}

} // namespace

//////////////////////////////////////////////////
// TODO double check this is correct...
Subgame *makeGameSingleNew(int shape, uint32_t number) {
    assert(shape > 0);

    Subgame *sg = new Subgame;
    vector<uint8_t> &boardVec = sg->boardVec();

    boardVec.reserve(shape);

    for (int i = 0; i < shape; i++) {
        const uint8_t tile = ((number >> i) & 0x1) + 1;
        boardVec.push_back(tile);
    }

    return sg;
}

std::vector<Subgame*> makeGameNew(uint64_t shapeNumber, uint32_t gameNumber) {
    vector<Subgame*> subgames;

    vector<int> shape = numberToShape(shapeNumber);

    for (int s : shape) {
        assert(0 < s && s < 32);

        uint32_t num = gameNumber;
        gameNumber >>= s;

        num &= (uint32_t(-1) >> (32 - s));

        subgames.push_back(makeGameSingleNew(s, num));
    }

    return subgames;
}

Subgame *Subgame::concatSubgames(const std::vector<Subgame*> &subgames) {
    Subgame *result = new Subgame();

    const size_t nSubgames = subgames.size();
    const size_t nEmpties = nSubgames > 1 ? nSubgames - 1 : 0;

    size_t totalSize = nEmpties;
    for (const Subgame *sg: subgames) {
        assert(sg != nullptr);
        totalSize += sg->size();
    }

    std::vector<uint8_t> &board = result->_data;
    board.reserve(totalSize);

    for (size_t i = 0; i < nSubgames; i++) {
        const Subgame *sg = subgames[i];

        for (const uint8_t &tile : sg->_data)
            board.push_back(tile);

        if (i + 1 < nSubgames)
            board.push_back(EMPTY);
    }

    assert(result->size() == totalSize);

    return result;
}

void Subgame::tryMirror() {
    if (shouldMirror(_data)) {
        const size_t N = _data.size();

        vector<uint8_t> reversed;
        reversed.reserve(N);

        for (size_t i = 0; i < N; i++)
            reversed.push_back(_data[N - 1 - i]);

        _data = std::move(reversed);
    }
}

void Subgame::negate() {
    size_t N = size();

    for (size_t i = 0; i < N; i++)
        (*this)[i] = opponentNumber((*this)[i]);
}

Subgame *Subgame::getNormalizedGame() const {
    vector<Subgame*> subgames = generateSubgamesNew(board(), size());

    for (Subgame *sg : subgames)
        sg->tryMirror();

    std::sort(subgames.begin(), subgames.end(), Subgame::normalizeSortOrder);

    Subgame *normalized = Subgame::concatSubgames(subgames);

    for (Subgame *sg : subgames)
        delete sg;

    return normalized;
}


uint64_t Subgame::getHash() const {
    return getZobristHash(BLACK, board(), size());
}

vector<optional<Subgame*>> Subgame::getChildren(int player, uint64_t dominance) const {
    assert(player == BLACK || player == WHITE);

    vector<optional<Subgame*>> children;

    const size_t boardSize = size();
    uint8_t boardCopy[boardSize];

    for (size_t i = 0; i < boardSize; i++)
        boardCopy[i] = (*this)[i];

    auto assertUndo = [&]() -> void {
        assert(memcmp(boardCopy, board(), boardSize) == 0);
    };

    assertUndo();

    size_t moveCount;
    int *moves = getMoves(boardCopy, boardSize, player, &moveCount);
    children.reserve(moveCount);

    uint8_t undoBuffer[UNDO_BUFFER_SIZE];

    for (size_t i = 0; i < moveCount; i++) {
        if (getDominated(dominance, i)) {
            children.push_back({});
            continue;
        }

        assertUndo();
        const int from = moves[2 * i];
        const int to = moves[2 * i + 1];

        play(boardCopy, undoBuffer, from, to);

        Subgame *child = new Subgame(boardCopy, boardSize);
        children.push_back(child);

        undo(boardCopy, undoBuffer);
    }

    assertUndo();
    delete[] moves;
    return children;
}

vector<Subgame*> Subgame::getNormalizedChildren(int player, uint64_t dominance) const {
    vector<optional<Subgame*>> childrenNonNormal = getChildren(player, dominance);

    vector<Subgame*> childrenNormal;
    childrenNormal.reserve(childrenNonNormal.size());

    for (optional<Subgame*> &sg_opt : childrenNonNormal) {
        if (!sg_opt.has_value())
            continue;

        Subgame *sg = sg_opt.value();

        Subgame *sgNormal = sg->getNormalizedGame();

        if (sgNormal->size() == 0)
            delete sgNormal;
        else
            childrenNormal.push_back(sgNormal);

        delete sg;
    }

    return childrenNormal;
}

bool Subgame::normalizeSortOrder(const Subgame *sg1, const Subgame *sg2) {
    if (sg1 == nullptr)
        return false;
    if (sg2 == nullptr)
        return true;

    const size_t N1 = sg1->size();
    const size_t N2 = sg2->size();

    if (N1 != N2)
        return N1 > N2;

    for (size_t i = 0; i < N1; i++) {
        const uint8_t &tile1 = (*sg1)[i];
        const uint8_t &tile2 = (*sg2)[i];

        if (tile1 != tile2)
            return tile1 > tile2;
    }

    return false;
}

bool Subgame::isVisuallyInversePair(const Subgame *sg1, const Subgame *sg2) {
    assert(sg1 != nullptr && sg2 != nullptr);

    const size_t N1 = sg1->size();
    const size_t N2 = sg2->size();

    if (N1 != N2)
        return false;

    bool failForward = false;
    bool failBackward = false;

    for (size_t i = 0; i < N1; i++) {
        const uint8_t &tile1 = (*sg1)[i];

        const uint8_t &tile2 = (*sg2)[i];
        const uint8_t &tile2Reversed = (*sg2)[N1 - 1 - i];

        const uint8_t sumForward = tile1 + tile2;
        const uint8_t sumBackward = tile1 + tile2Reversed;

        failForward |= (sumForward != 3);
        failBackward |= (sumBackward != 3);
    }

    if (failForward && failBackward)
        return false;

    return true;
}

vector<Subgame*> generateSubgamesNew(const uint8_t *board, size_t len) {
    vector<Subgame*> subgames;

    int start = -1;
    int end = -1;

    int foundMask = 0;

    for (int i = 0; i < len; i++) {
        if (start == -1 && board[i] != 0) {
            start = i;
            foundMask = 0;
        }

        if (board[i] != 0) {
            foundMask |= board[i];
        }

        if (start != -1 && board[i] == 0) {
            if (foundMask == 3) {
                //subgames.push_back(pair<int, int>(start, i));

                const size_t startIdx = start;
                const size_t endIdx = i;
                assert(endIdx >= startIdx);
                const size_t subgameLen = endIdx - startIdx;

                subgames.push_back(new Subgame(board + startIdx, subgameLen));
            }
            start = -1;
        }
    }

    if (start != -1 && foundMask == 3) {
        //subgames.push_back(pair<int, int>(start, len));

        const size_t startIdx = start;
        const size_t endIdx = len;
        assert(endIdx >= startIdx);
        const size_t subgameLen = endIdx - startIdx;

        subgames.push_back(new Subgame(board + startIdx, subgameLen));
    }

    return subgames;
}


//////////////////////////////////////// GameGenerator implementation
vector<vector<int>> initShapeList() {
    vector<vector<int>> shapeListAll = makeShapes();

    sort(shapeListAll.begin(), shapeListAll.end(),
        [](const vector<int> &s1, const vector<int> &s2) {
            int bits1 = s1.size() - 1;
            int bits2 = s2.size() - 1;

            for (int chunk : s1) {
                bits1 += chunk;
            }

            for (int chunk : s2) {
                bits2 += chunk;
            }

            if (bits1 == bits2) {
                return s1.size() > s2.size();
            }

            return bits1 < bits2;
        }
    );


    vector<vector<int>> shapeList;
    for (const vector<int> &shape : shapeListAll) {
        int totalSize = shape.size() - 1;

        for (const int &chunkSize : shape)
            totalSize += chunkSize;

        //if (totalSize <= 12)
        //    shapeList.emplace_back(shape);
        shapeList.emplace_back(shape);
    }

    return shapeList;
}

const vector<vector<int>> GameGenerator::_shapeList = initShapeList();

GeneratedGame GameGenerator::generate() const {
    assert(*this);

    GeneratedGame genGame;

    const vector<int> &shape = _getCurrentShape();

    const uint64_t shapeNumber = shapeToNumber(shape);

    vector<Subgame*> subgames = makeGameNew(shapeNumber, _currentGameNumber);
    Subgame *g = Subgame::concatSubgames(subgames);
    for (Subgame *sg : subgames)
        delete sg;

    genGame.game = g;
    genGame.shapeNumber = shapeNumber;
    genGame.gameNumber = _currentGameNumber;
    genGame.gameSize = _currentGameSize;
    genGame.shape = shape;

    return genGame;
}


bool GameGenerator::_incrementBoard() {
    assert(_currentGameNumber <= _maxGameNumber);

    if (_currentGameNumber == _maxGameNumber)
        return false;

    _currentGameNumber++;
    return true;
}

bool GameGenerator::_incrementShape() {
    _shapeIdx++;
    const bool hasNext = _shapeIdx < _shapeList.size();

    if (hasNext)
        _reshapeGame(_getCurrentShape());

    return hasNext;
}

void GameGenerator::_reshapeGame(const vector<int> &newShape) {
    const vector<int> &shape = _getCurrentShape();

    _currentGameNumber = 0;

    _maxGameNumber = 1;
    _currentGameSize = 0;
    for (int chunkSize : shape) {
        _maxGameNumber <<= chunkSize;
        _currentGameSize += chunkSize;
    }

    _maxGameNumber -= 1;
}

