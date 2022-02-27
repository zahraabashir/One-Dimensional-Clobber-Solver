#include "solver.h"
#include "utils.h"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>
#include <algorithm>
#include <random>


int node_count = 0; //nodes visited
int best_from = 0; //root player's move
int best_to = 0;

//int collisions = 0; //transposition table collisions

BasicSolver::BasicSolver(int rootPlayer, int boardSize, Database *db) {
    this->rootPlayer = rootPlayer;
    this->rootOpponent = opponentNumber(rootPlayer);
    this->boardSize = boardSize;

    this->useDatabase = true;
    this->db = db;

    outOfTime = false;

    int bits = 24;
    if (boardSize > 38) {
        bits = 23;
    }

    codeLength = bits;

    //look at macros in header file
    tableEntrySize = boardSize + 5 + 0 * sizeof(int);

    bitMask = 0;
    for (int i = 0; i < bits; i++) {
        bitMask <<= 1;
        bitMask |= 1;
    }

    size_t tableSize = 1;
    tableSize <<= (size_t) bits;
    tableSize *= (size_t) tableEntrySize;

    //std::cout << "Entry size: " << tableEntrySize << std::endl;
    //std::cout << "Size: " << (double) tableSize / (1000.0 * 1000.0) << std::endl;
    //std::cout << "Board size: " << boardSize << std::endl;   

    table = (char *) calloc(tableSize, 1);
}

BasicSolver::~BasicSolver() {
    free(table);
}

bool BasicSolver::validateTableEntry(State *state, int p, char *entry) {
    bool found = false;
    if (PLAYER(entry) == p) {
        found = true;
        for (int i = 0; i < boardSize; i++) {
            if (entry[i] != state->board[i]) {
                found = false;
                break;
            }
        }
    }
    return found;
}

int BasicSolver::solveID(State *state, int p, int n) {
    maxCompleted = 1;

    int depth = 0;

    limitCompletions = true;

    while (true) {
        maxDepth = depth;
        //collisions = 0;

        completed = 0;

        maxCompleted += 100;

        if (depth > 150) {
            limitCompletions = false;
        }


        //std::cout << depth << std::endl;


        std::pair<int, bool> result = rootSearchID(state, p, n, 0);

        if (outOfTime) {
            return EMPTY;
            best_from = -1;
        }

        //std::cout << depth << " " << collisions << std::endl;

        if (result.second) {
            return result.first;
        }

        depth += 1;
    }
}


std::pair<int, bool> BasicSolver::rootSearchID(State *state, int p, int n, int depth) {
    node_count += 1;
    updateTime();
    if (outOfTime) {
        return std::pair<int, bool>(0, false);
    }

    //lookup entry
    //if solved, return
    int code = state->code(p);
    char *entry = getTablePtr(code);

    bool validEntry = validateTableEntry(state, p, entry);
    if (validEntry && OUTCOME(entry) != EMPTY) {
        return std::pair<int, bool>(OUTCOME(entry), true);
    }

    if (!validEntry && PLAYER(entry) != 0) {
        //collisions++;
    }

    //generate moves
    //check for terminal
    size_t moveCount;
    int *moves = state->getMoves(p, n, &moveCount);

    if (moveCount == 0) { //is terminal
        completed += 1;
        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = n;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = -127;
        }
        return std::pair<int, bool>(n, true);
    }


    //if deep, generate heuristic and return
    if (depth == maxDepth || (limitCompletions && (completed >= maxCompleted))) {
        completed += 1;

        size_t pMoveCount;
        int *pMoves = state->getMoves(n, p, &pMoveCount);

        if (pMoveCount > 0) {
            delete[] pMoves;
        }

        int h = HEURISTIC(entry);
        if (!validEntry) {
            //h = (int) moveCount - (int) pMoveCount;
            h = -pMoveCount;
        }

        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = EMPTY;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = h;
        }        

        return std::pair<int, bool>(h, false);
    }

    //visit children, starting with best; find new best and update heuristic
    //if solved, save value and return
    int bestMove = 0;
    bool checkedBestMove = false;
    if (validEntry) {
        bestMove = BESTMOVE(entry);
    }

    int bestVal = -127;

    char undoBuffer[sizeof(int) + 2 * sizeof(char)];

    int newBestMove = 0;

    bool allProven = true;

    for (int i = bestMove; i < moveCount; i++) {
        if (checkedBestMove && i == bestMove) {
            continue;
        }

        
        //if (depth == 0) {
        //    std::cout << i << " ";
        //}
        

        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        state->play(from, to, undoBuffer);
        std::pair<int, bool> result = searchID(state, n, p, depth + 1);

        if (outOfTime) {
            delete[] moves;
            return result;
        }

        state->undo(undoBuffer);

        allProven &= result.second;

        if (result.second && result.first == p) {
            if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
                memcpy(entry, state->board, boardSize);
                PLAYER(entry) = p;
                OUTCOME(entry) = p;
                BESTMOVE(entry) = i;
                DEPTH(entry) = depth;
                HEURISTIC(entry) = 127;
            }
            best_from = from;
            best_to = to;
            delete[] moves;
            return std::pair<int, bool>(p, true);
        }

        if (!result.second) {
            result.first *= -1;
            if (result.first > bestVal) {
                newBestMove = i;
                bestVal = result.first;

            }
        }

        if (!checkedBestMove) {
            checkedBestMove = true;

            i = -1;
        }
    }

    //if (depth == 0) {
    //    std::cout << std::endl;
    //}


    delete[] moves;

    if (allProven) {
        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = n;
            BESTMOVE(entry) = newBestMove; //these two values don't matter -- node result is known
            DEPTH(entry) = depth;
            HEURISTIC(entry) = -127;

        }
        best_from = -1;
        best_to = -1;
        return std::pair<int, bool>(n, true);
    }


    if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        
        memcpy(entry, state->board, boardSize);
        PLAYER(entry) = p;
        OUTCOME(entry) = EMPTY;
        BESTMOVE(entry) = newBestMove;
        DEPTH(entry) = depth;
        HEURISTIC(entry) = bestVal;
    }
    return std::pair<int, bool>(bestVal, false);
}

std::vector<std::pair<int, int>> generateSubgames(State *state) {
    char *board = state->board;
    std::vector<std::pair<int, int>> subgames;

    int start = -1;
    int end = -1;

    int foundMask = 0;

    for (int i = 0; i < state->boardSize; i++) {
        if (start == -1 && board[i] != 0) {
            start = i;
            foundMask = 0;
        }
        
        if (board[i] != 0) {
            foundMask |= board[i];
        }

        if (start != -1 && board[i] == 0) {
            if (foundMask == 3) {
                subgames.push_back(std::pair<int, int>(start, i));
            }
            start = -1;
        }
    }

    if (start != -1) {
        subgames.push_back(std::pair<int, int>(start, state->boardSize));
    }

          
    return subgames;
}



char *boardComparePtr;
bool subgameCompare(const std::pair<int, int> &a, const std::pair<int, int> &b) {
    char *board = boardComparePtr;

    int s1 = a.first;
    int e1 = a.second;
    int l1 = e1 - s1;

    int s2 = b.first;
    int e2 = b.second;
    int l2 = e2 - s2;

    for (int i = 0; i < std::min(l1, l2); i++) {
        if (board[s1 + i] == board[s2 + i]) {
            continue;
        }

        if (board[s1 + i] < board[s2 + i]) {
            return true;
        } else {
            return false;
        }
    }


    return l1 < l2;
}



void BasicSolver::simplify(State *state) {
    char *board = state->board;

    //find subgames
    std::vector<std::pair<int, int>> subgames = generateSubgames(state);

    //print
    for (int i = 0; i < state->boardSize; i++) {
        //std::cout << playerNumberToChar(board[i]);
    }
    //std::cout << std::endl;
    //std::cout << "Subgames: " << subgames.size() << std::endl;

    int subgameCount = subgames.size();

    //find subgames that have the same length
    for (int i = 0; i < subgameCount; i++) {
        int s1 = subgames[i].first;
        int e1 = subgames[i].second;
        int l1 = e1 - s1;

        for (int j = i + 1; j < subgameCount; j++) {
            int s2 = subgames[j].first;
            int e2 = subgames[j].second;
            int l2 = e2 - s2;
            if (l1 == l2) {
                //std::cout << "Match" << std::endl;

                //forward pass
                bool diff = true;

                for (int k = 0; k < l1; k++) {
                    if (board[s1 + k] != 3 - board[s2 + k]) {
                        diff = false;
                        break;
                    }
                }

                if (diff) {
                    //std::cout << "Simplify on forward pass" << std::endl;
                    for (int k = 0; k < l1; k++) {
                        board[s1 + k] = 0;
                        board[s2 + k] = 0;
                    }
                    continue;
                }

                //backward pass
                diff = true;

                for (int k = 0; k < l1; k++) {
                    if (board[s1 + k] != 3 - board[e2 - 1 - k]) {
                        diff = false;
                        break;
                    }
                }

                if (diff) {
                    //std::cout << "Simplify on backward pass" << std::endl;
                    for (int k = 0; k < l1; k++) {
                        board[s1 + k] = 0;
                        board[s2 + k] = 0;
                    }
                    continue;
                }


            }
        }
    }

    //delete other P positions
    for (int i = 0; i < subgameCount; i++) {
        int s = subgames[i].first;
        int e = subgames[i].second;

        if (board[s] == 0) {
            continue;
        }

        int length = e - s;
        std::string subBoard(&board[s], length);
        int outcome = db->get(length, subBoard.data());

        //std::cout << "Deleting" << std::endl;
        if (outcome == OC_P) {
            for (int j = 0; j < length; j++) {
                board[s + j] = 0;
            }
        }
    }

    //now canonicalize the board
    subgames = generateSubgames(state);
    subgameCount = subgames.size();

    //sort subgames
    boardComparePtr = board;
    std::sort(subgames.begin(), subgames.end(), subgameCompare);


    int position = 0;
    char newBoard[state->boardSize] = {};
    for (int i = 0; i < subgameCount; i++) {
        int s = subgames[i].first;
        int e = subgames[i].second;

        memcpy(&newBoard[position], &board[s], e - s);
        position += (e - s) + 1;
    }

    memcpy(state->board, newBoard, state->boardSize);

    for (int i = 0; i < state->boardSize; i++) {
    //    std::cout << playerNumberToChar(board[i]);
    }
    //std::cout << std::endl;



    //std::cout << std::endl;
}

bool subgameLengthCompare(const std::pair<int, int> &a, const std::pair<int, int> &b) {
    return (a.second - a.first) > (b.second - b.first);
}

std::pair<int, bool> BasicSolver::searchID(State *state, int p, int n, int depth) {
    node_count += 1;
    updateTime();
    if (outOfTime) {
        return std::pair<int, bool>(0, false);
    }

    char *oldBoard = new char[state->boardSize];
    memcpy(oldBoard, state->board, state->boardSize);
    simplify(state);


    //lookup entry
    //if solved, return
    int code = state->code(p);
    char *entry = getTablePtr(code);

    bool validEntry = validateTableEntry(state, p, entry);
    if (validEntry && OUTCOME(entry) != EMPTY) {
        memcpy(state->board, oldBoard, state->boardSize);
        delete[] oldBoard;
        return std::pair<int, bool>(OUTCOME(entry), true);
    }

    if (!validEntry && PLAYER(entry) != 0) {
        //collisions++;
    }



    //generate subgames, look them up in the database
    if (useDatabase) {
        std::vector<std::pair<int, int>> subgames = generateSubgames(state);

        //sort subgames by length
        std::sort(subgames.begin(), subgames.end(), subgameLengthCompare);

        std::vector<int> lengths;
        std::vector<int> outcomes;

        //count outcomes
        int counts[5];
        for (int i = 0; i < 5; i++) {
            counts[i] = 0;
        }

        int outcomeMask = 0;


        for (auto it = subgames.begin(); it != subgames.end(); it++) {
            int length = it->second - it->first;
            lengths.push_back(length);

            int outcome = db->get(length, &state->board[it->first]);
            outcomes.push_back(outcome);

            counts[outcome] += 1;
            outcomeMask |= (1 << outcome);
        }

        //Only Bs
        if ((outcomeMask & ~(1 << OC_B)) == 0 && counts[OC_B] > 0) {
            memcpy(state->board, oldBoard, state->boardSize);
            delete[] oldBoard;
            return std::pair<int, bool>(OC_B, true);
        }

        //Only Ws
        if ((outcomeMask & ~(1 << OC_W)) == 0 && counts[OC_W] > 0) {
            memcpy(state->board, oldBoard, state->boardSize);
            delete[] oldBoard;
            return std::pair<int, bool>(OC_W, true);
        }

        //Only one N
        if ((outcomeMask & ~(1 << OC_N)) == 0 && counts[OC_N] == 1) {
            memcpy(state->board, oldBoard, state->boardSize);
            delete[] oldBoard;
            return std::pair<int, bool>(p, true); //current player wins
        }


        //Use differences




        /*
        int commonOutcome = 0;
        if (outcomes.size() > 0) {
            commonOutcome = outcomes[0];
        }

        for (int i = 1; i < outcomes.size(); i++) {
            if (outcomes[i] != commonOutcome && outcomes[i] != OC_P) {
                commonOutcome = 0;
                break;
            }
        }

        if (commonOutcome == 1 || commonOutcome == 2) {
            memcpy(state->board, oldBoard, state->boardSize);
            delete[] oldBoard;
            //std::cout << "DB CUT" << std::endl;
            return std::pair<int, bool>(commonOutcome, true);
        }
        */
    }


    //generate moves
    //check for terminal
    size_t moveCount;
    int *moves = state->getMoves(p, n, &moveCount);

    if (moveCount == 0) { //is terminal
        completed += 1;
        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = n;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = -127;
        }

        memcpy(state->board, oldBoard, state->boardSize);
        delete[] oldBoard;
        return std::pair<int, bool>(n, true);
    }


    //if deep, generate heuristic and return
    if (depth == maxDepth || (limitCompletions && (completed >= maxCompleted))) {
        completed += 1;

        size_t pMoveCount;
        int *pMoves = state->getMoves(n, p, &pMoveCount);

        if (pMoveCount > 0) {
            delete[] pMoves;
        }

        int h = HEURISTIC(entry);
        if (!validEntry) {
            //h = (int) moveCount - (int) pMoveCount;
            h = -pMoveCount;
        }

        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = EMPTY;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = h;
        }        

        memcpy(state->board, oldBoard, state->boardSize);
        delete[] oldBoard;
        return std::pair<int, bool>(h, false);
    }

    //visit children, starting with best; find new best and update heuristic
    //if solved, save value and return

    std::vector<int> moveOrder;


    int bestMove = -1;
    bool checkedBestMove = false;
    if (validEntry) {
        bestMove = BESTMOVE(entry);
    }

    for (int i = 0; i < moveCount; i++) {
        if (i == bestMove) {
            continue;
        }
        moveOrder.push_back(i);
    }

    std::shuffle(moveOrder.begin(), moveOrder.end(), std::default_random_engine(3.141));
    if (bestMove != -1) {
        moveOrder.push_back(bestMove);
    }

    int bestVal = -127;

    char undoBuffer[sizeof(int) + 2 * sizeof(char)];

    int newBestMove = 0;

    bool allProven = true;

    for (auto it = moveOrder.rbegin(); it != moveOrder.rend(); it++) {
        int i = *it;

        if (checkedBestMove && i == bestMove) {
            continue;
        }
        
        //if (depth == 0) {
        //    std::cout << i << " ";
        //}
        
        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        state->play(from, to, undoBuffer);
        std::pair<int, bool> result = searchID(state, n, p, depth + 1);
        if (outOfTime) {
            memcpy(state->board, oldBoard, state->boardSize);
            delete[] oldBoard;
            delete[] moves;
            return result;
        }
    
        state->undo(undoBuffer);

        allProven &= result.second;

        if (result.second && result.first == p) {
            if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
                memcpy(entry, state->board, boardSize);
                PLAYER(entry) = p;
                OUTCOME(entry) = p;
                BESTMOVE(entry) = i;
                DEPTH(entry) = depth;
                HEURISTIC(entry) = 127;
            }

            memcpy(state->board, oldBoard, state->boardSize);
            delete[] oldBoard;
            delete[] moves;
            return std::pair<int, bool>(p, true);
        }

        if (!result.second) {
            result.first *= -1;
            if (result.first > bestVal) {
                newBestMove = i;
                bestVal = result.first;
            }
        }

        if (!checkedBestMove) {
            checkedBestMove = true;
            i = -1;
        }
    }

    //if (depth == 0) {
    //    std::cout << std::endl;
    //}

    delete[] moves;

    if (allProven) {
        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = n;
            BESTMOVE(entry) = newBestMove; //these two values don't matter -- node result is known
            DEPTH(entry) = depth;
            HEURISTIC(entry) = -127;

        }

        memcpy(state->board, oldBoard, state->boardSize);
        delete[] oldBoard;
        return std::pair<int, bool>(n, true);
    }


    if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        memcpy(entry, state->board, boardSize);
        PLAYER(entry) = p;
        OUTCOME(entry) = EMPTY;
        BESTMOVE(entry) = newBestMove;
        DEPTH(entry) = depth;
        HEURISTIC(entry) = bestVal;
    }
    memcpy(state->board, oldBoard, state->boardSize);
    delete[] oldBoard;
    return std::pair<int, bool>(bestVal, false);
}

// to print the first best move
int BasicSolver::solveRoot(State *state, int p, int n) {

    //Look up state in table
    int code = state->code(p);
    char *entry = getTablePtr(code);

    bool found = false;
    if (PLAYER(entry) == p) {
        found = true;
        for (int i = 0; i < boardSize; i++) {
            if (entry[i] != state->board[i]) {
                found = false;
                break;
            }
        }
    }

    if (found) {
        return OUTCOME(entry);
    }

    size_t moveCount;
    int *moves = state->getMoves(p, n, &moveCount);

    if (moveCount == 0) {
        memcpy(entry, state->board, boardSize);
        PLAYER(entry) = p;
        OUTCOME(entry) = n;
        node_count += 1;
        return n;
    }

    char undoBuffer[sizeof(int) + 2 * sizeof(char)];

    for (size_t i = 0; i < moveCount; i++) {
        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        state->play(from, to, undoBuffer);
        node_count += 1;
        int result = solve(state, n, p);
        state->undo(undoBuffer);

        if (result == p) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = p;
            best_from = from;
            best_to=to;
            delete[] moves;
            return p;
        }
    }

    memcpy(entry, state->board, boardSize);
    PLAYER(entry) = p;
    OUTCOME(entry) = n;
    best_from=-1;
    best_to=-1;

    delete[] moves;
    return n;
}

int BasicSolver::solve(State *state, int p, int n) {

    //Look up state in table
    int code = state->code(p);
    char *entry = getTablePtr(code);

    bool found = false;
    if (PLAYER(entry) == p) {
        found = true;
        for (int i = 0; i < boardSize; i++) {
            if (entry[i] != state->board[i]) {
                found = false;
                break;
            }
        }
    }

    if (found) {
        return OUTCOME(entry);
    }

    size_t moveCount;
    int *moves = state->getMoves(p, n, &moveCount);

    if (moveCount == 0) {
        memcpy(entry, state->board, boardSize);
        PLAYER(entry) = p;
        OUTCOME(entry) = n;
        node_count += 1;
        return n;
    }

    char undoBuffer[sizeof(int) + 2 * sizeof(char)];

    for (size_t i = 0; i < moveCount; i++) {
        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        state->play(from, to, undoBuffer);
        node_count += 1;
        int result = solve(state, n, p);
        state->undo(undoBuffer);

        if (result == p) {
            memcpy(entry, state->board, boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = p;

            delete[] moves;
            return p;
        }
    }

    memcpy(entry, state->board, boardSize);
    PLAYER(entry) = p;
    OUTCOME(entry) = n;

    delete[] moves;
    return n;
}

char *BasicSolver::getTablePtr(int code) {
    int idx = code & bitMask;
    for (int i = 1; ; i++) {
        int shift = i * codeLength;

        if (shift >= sizeof(int) * 8) {
            break;
        }

        int add = code >> shift;
        add &= bitMask;
        code += add;
    }
    code &= bitMask;

    return table + (idx * tableEntrySize);
}

void BasicSolver::updateTime() {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - startTime).count();

    if (elapsed >= timeLimit) {
        outOfTime = true;
    }
}
