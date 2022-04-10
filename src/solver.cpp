/*

        TODO: Solver needs to accomodate varying length boards...
            the transposition table will need to change, and copying/restoring boards
*/


#include "solver.h"
#include "utils.h"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>


using namespace std;

int node_count = 0; //nodes visited
int best_from = -1; //root player's move
int best_to = -1;

//int collisions = 0; //transposition table collisions


void BasicSolver::generateBounds(State *state, Bound &alpha, Bound &beta) {
    vector<pair<int, int>> sg = generateSubgames(state);

    int lowSum = 0;
    bool lowStar = false;

    int highSum = 0;
    bool highStar = false;

    for (auto it = sg.begin(); it != sg.end(); it++) {
        int size = it->second - it->first;
        if (size > DB_MAX_BOUND_BITS) {
            alpha = Bound::min();
            beta = Bound::max();
            return;
        }

        unsigned char *dbEntry = db->get(size, state->board + it->first);

        if (DB_GET_OUTCOME(dbEntry) == OC_UNKNOWN) {
            alpha = Bound::min();
            beta = Bound::max();
            return;
        }

        int low = DB_GET_BOUND(dbEntry, 0);
        int high = DB_GET_BOUND(dbEntry, 1);

        int lowVal = sign(low) * max(abs(low) - 1, 0);
        lowSum += lowVal;
        lowStar = abs(low) % 2 != 0 ? !lowStar : lowStar;

        int highVal = sign(high) * max(abs(high) - 1, 0);
        highSum += highVal;
        highStar = abs(high) % 2 != 0 ? !highStar : highStar;

    }

    alpha.ups = lowSum;
    alpha.star = lowStar;

    beta.ups = highSum;
    beta.star = highStar;
}



BasicSolver::BasicSolver(int rootPlayer, int boardSize, Database *db) {
    this->rootPlayer = rootPlayer;
    this->boardSize = boardSize;

    this->db = db;

    this->rng = new default_random_engine(3.141);

    outOfTime = false;

    int bits = 24;
    if (boardSize > 38) {
        bits = 23;
    }

    codeLength = bits;

    //look at macros in header file
    tableEntrySize = 1 + sizeof(char *) + 3 + sizeof(unsigned int) + 1;

    bitMask = 0;
    for (int i = 0; i < bits; i++) {
        bitMask <<= 1;
        bitMask |= 1;
    }

    size_t tableSize = 1;
    tableSize <<= (size_t) bits;
    tableSize *= (size_t) tableEntrySize;

    entryCount = ((size_t) 1) << bits;

    //cout << "Entry size: " << tableEntrySize << endl;
    //cout << "Size: " << (double) tableSize / (1000.0 * 1000.0) << endl;
    //cout << "Board size: " << boardSize << endl;   

    table = (char *) calloc(tableSize, 1);
}

BasicSolver::~BasicSolver() {
//    char *ptr = table;
//
//    for (size_t i = 0; i < entryCount; i++) {
//        if (BOARDLEN(ptr) != 0) {
//            delete[] BOARDPTR(ptr);
//        }
//
//        ptr += tableEntrySize;
//    }


    free(table);
    delete rng;
}

bool BasicSolver::validateTableEntry(State *state, int p, char *entry) {
    uint8_t len = BOARDLEN(entry);
    char *entryBoard = BOARDPTR(entry);

    //cout << "--------------------------" << endl;
    //for (int i = 0; i < state->boardSize; i++) {
    //    cout << playerNumberToChar(state->board[i]);
    //}
    //cout << endl;

    //for (int i = 0; i < len; i++) {
    //    cout << playerNumberToChar(entryBoard[i]);
    //}
    //cout << endl;
    //cout << "==========================" << endl;
    //cout << endl;


    if (len != state->boardSize) {
        return false;
    }



    bool found = false;
    if (PLAYER(entry) == p) {
        found = true;
        for (int i = 0; i < state->boardSize; i++) {
            if (entryBoard[i] != state->board[i]) {
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

        //used to be 100
        maxCompleted += 50;

        //used to be 150
        if (depth > 300) {
            limitCompletions = false;
        }


        //cout << depth << endl;


        Bound alpha = Bound::min();
        Bound beta = Bound::max();

        Bound cb1, cb2;

        pair<int, bool> result = rootSearchID(state, p, n, 0, alpha, beta, cb1, cb2);

        if (outOfTime) {
            return EMPTY;
            best_from = -1;
        }

        //cout << depth << " " << collisions << endl;

        if (result.second) {
            return result.first;
        }

        depth += 1;
    }
}


pair<int, bool> BasicSolver::rootSearchID(State *state, int p, int n, int depth, Bound alpha, Bound beta, Bound &rb1, Bound &rb2) {
    node_count += 1;
    updateTime();
    if (outOfTime) {
        return pair<int, bool>(0, false);
    }

    //int boundWin = checkBounds(state);

    //if (boundWin != 0) {
    //    return pair<int, bool>(boundWin, true);
    //}

    Bound a, b;
    if (!limitCompletions) {
        generateBounds(state, a, b);
    }

//    if (a > Bound(0, 0)) {
//        return pair<int, bool>(1, true);
//    }
//
//    if (b < Bound(0, 0)) {
//        return pair<int, bool>(2, true);
//    }


//    Bound b1, b2;
//    generateBounds(state, b1, b2);
//
//    if (b1 > Bound()) {
//        return pair<int, bool>(1, true);
//    }
//
//    if (b2 < Bound()) {
//        return pair<int, bool>(2, true);
//    }


    //lookup entry
    //if solved, return
    int code = state->code(p);
    char *entry = getTablePtr(code);

    bool validEntry = validateTableEntry(state, p, entry);
    if (validEntry && OUTCOME(entry) != EMPTY) {
        return pair<int, bool>(OUTCOME(entry), true);
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
            //memcpy(entry, state->board, boardSize);
            RESIZETTBOARD(entry, state->boardSize);
            memcpy(BOARDPTR(entry), state->board, state->boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = n;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = -127;
        }
        return pair<int, bool>(n, true);
    }

    //Delete dominated moves
    vector<pair<int, int>> sg = generateSubgames(state);

    for (int i = 0; i < sg.size(); i++) {
        int start = sg[i].first;
        int end = sg[i].second; //index after end
        int len = end - start;

        unsigned char *dbEntry = db->get(len, &state->board[start]);

        uint64_t dominated = DB_GET_DOMINATED(dbEntry, p);

        int moveIndex = 0;
        for (int j = 0; j < moveCount; j++) {
            int from = moves[2 * j];
            int to = moves[2 * j + 1];

            if (from >= start && from < end) { //found move
                if ((dominated >> moveIndex) & ((uint64_t) 1)) {
                    //cout << "FOUND" << endl;
                    moves[2 * j] = -1;
                    moves[2 * j + 1] = -1;
                }
                moveIndex++;
            }
        }

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
            //memcpy(entry, state->board, boardSize);
            RESIZETTBOARD(entry, state->boardSize);
            memcpy(BOARDPTR(entry), state->board, state->boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = EMPTY;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = h;
        }        

        delete[] moves;
        return pair<int, bool>(h, false);
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
        //    cout << i << " ";
        //}
        

        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        if (from == -1) {
            if (!checkedBestMove) {
                checkedBestMove = true;

                i = -1;
            }
            continue;
        }

        state->play(from, to, undoBuffer);

        Bound cb1, cb2;
        pair<int, bool> result = searchID(state, n, p, depth + 1, alpha, beta, cb1, cb2);

        if (outOfTime) {
            delete[] moves;
            return result;
        }

        state->undo(undoBuffer);

        allProven &= result.second;

        if (result.second && result.first == p) {
            if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
                //memcpy(entry, state->board, boardSize);
                RESIZETTBOARD(entry, state->boardSize);
                memcpy(BOARDPTR(entry), state->board, state->boardSize);
                PLAYER(entry) = p;
                OUTCOME(entry) = p;
                BESTMOVE(entry) = i;
                DEPTH(entry) = depth;
                HEURISTIC(entry) = 127;
            }
            best_from = from;
            best_to = to;
            delete[] moves;
            return pair<int, bool>(p, true);
        }

        //update ab
        if (!limitCompletions) {
            bool abCut = false;

            if (p == 1) {
                if (cb1 > rb1) {
                    rb1 = cb1;
                }
                if (cb1 >= beta) {
                    abCut = true;
                } else {
                    if (cb1 > alpha) {
                        alpha = cb1;
                    }
                }
            } else {
                if (cb2 < rb2) {
                    rb2 = cb2;
                }
                if (cb2 <= alpha) {
                    abCut = true;
                } else {
                    if (cb2 < beta) {
                        beta = cb2;
                    }
                }
            }

            if (abCut && depth > 0 && !limitCompletions) {
                delete[] moves;
                return pair<int, bool>(0, false);
            }
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
    //    cout << endl;
    //}


    delete[] moves;

    if (allProven) {
        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            //memcpy(entry, state->board, boardSize);
            RESIZETTBOARD(entry, state->boardSize);
            memcpy(BOARDPTR(entry), state->board, state->boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = n;
            BESTMOVE(entry) = newBestMove; //these two values don't matter -- node result is known
            DEPTH(entry) = depth;
            HEURISTIC(entry) = -127;

        }
        best_from = -1;
        best_to = -1;
        return pair<int, bool>(n, true);
    }


    if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        //memcpy(entry, state->board, boardSize);
        RESIZETTBOARD(entry, state->boardSize);
        memcpy(BOARDPTR(entry), state->board, state->boardSize);
        PLAYER(entry) = p;
        OUTCOME(entry) = EMPTY;
        BESTMOVE(entry) = newBestMove;
        DEPTH(entry) = depth;
        HEURISTIC(entry) = bestVal;
    }
    return pair<int, bool>(bestVal, false);
}

vector<pair<int, int>> generateSubgames(State *state) {
    char *board = state->board;
    vector<pair<int, int>> subgames;

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
                subgames.push_back(pair<int, int>(start, i));
            }
            start = -1;
        }
    }

    if (start != -1) {
        subgames.push_back(pair<int, int>(start, state->boardSize));
    }

          
    return subgames;
}



char *boardComparePtr;
bool subgameCompare(const pair<int, int> &a, const pair<int, int> &b) {
    char *board = boardComparePtr;

    int s1 = a.first;
    int e1 = a.second;
    int l1 = e1 - s1;

    int s2 = b.first;
    int e2 = b.second;
    int l2 = e2 - s2;

    for (int i = 0; i < min(l1, l2); i++) {
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



void BasicSolver::simplify(State *state, int depth) {
    char *board = state->board; //MUST reassign this after deleting state->board


    vector<pair<int, int>> subgames = generateSubgames(state);
    int subgameCount = subgames.size();


    //{ //TODO this MUST work somehow...
    //    char arr[state->boardSize];
    //    memcpy(arr, state->board, state->boardSize);
    //    
    //    memset(state->board, 7, state->boardSize);
    //    delete[] state->board;
    //    state->board = new char[state->boardSize];
    //    board = state->board;

    //    memcpy(state->board, arr, state->boardSize);
    //}

    //#define SOLVER_SIMPLIFY
    #if defined(SOLVER_SIMPLIFY)

    if (depth <= 1 || true) {
        bool type2 = false;
        bool printSub = false;


        //replace games with simpler games
        subgames = generateSubgames(state);
        subgameCount = subgames.size();

        size_t newSize = max(subgameCount - 1, 0);

        //Figure out new size based on database links
        for (int i = 0; i < subgameCount; i++) {
            int start = subgames[i].first;
            int end = subgames[i].second;
            int len = end - start;

            unsigned char *entry = db->get(len, state->board + start);
            if (entry == 0 || DB_GET_OUTCOME(entry) == 0) {
                entry = 0;
            }
            //entry = 0;

            if (entry == 0) {
                newSize += len;
            } else {
                unsigned char *linkedEntry = db->getFromIdx(DB_GET_LINK(entry));
                newSize += DB_GET_LENGTH(linkedEntry);


                if (linkedEntry != entry) {
                    type2 = true;
                }
            }

        }


        if (type2 && printSub) {
            cout << "Board before substitution (" << state->boardSize << ")" << endl;
            for (int i = 0; i < state->boardSize; i++) {
                cout << playerNumberToChar(state->board[i]);
            }
            cout << endl;
        }



        if (newSize != state->boardSize) {
            //cout << "Board size changed: " << state->boardSize << " --> " << newSize << endl;
            //while (1) {
            //}
        }

        if (newSize > 0) {
            char *newBuffer = new char[newSize + 1];
            memset(newBuffer, 0, newSize + 1);


            size_t idx = 0;
            //copy simpler games from database links
            for (int i = 0; i < subgameCount; i++) {
                int start = subgames[i].first;
                int end = subgames[i].second;
                int len = end - start;

                unsigned char *entry = db->get(len, state->board + start);
                if (entry == 0 || DB_GET_OUTCOME(entry) == 0) {
                    entry = 0;
                }
                //entry = 0;
                unsigned char *linkedEntry = entry == 0 ? 0 : db->getFromIdx(DB_GET_LINK(entry));


                if (entry == 0 || entry == linkedEntry) {
                    memcpy(newBuffer + idx, state->board + start, len);
                    idx += len + 1;

                    //cout << "Substitution 1:" << endl;
                    //for (int j = 0; j < len; j++) {
                    //    cout << playerNumberToChar(state->board[start + j]);
                    //}
                    //cout << endl;

                } else {
                    int linkedLength = DB_GET_LENGTH(linkedEntry);
                    int linkedNumber = DB_GET_NUMBER(linkedEntry);

                    char *newGame = generateGame(linkedLength, linkedNumber);

                    memcpy(newBuffer + idx, newGame, linkedLength);
                    idx += linkedLength + 1;

                    //cout << "Substitution 2:" << endl;
                    //for (int j = 0; j < linkedLength; j++) {
                    //    cout << playerNumberToChar(newGame[j]);
                    //}
                    //cout << endl;


                    delete[] newGame;
                }

            }


            delete[] state->board;
            state->board = newBuffer;
            board = state->board;
            state->boardSize = newSize;
        }

        if (type2 && printSub) {
            cout << "Board after substitution (" << state->boardSize << ")" << endl;
            for (int i = 0; i < state->boardSize; i++) {
                cout << playerNumberToChar(state->board[i]);
            }
            cout << endl << endl;
        }

    }

    #endif



    //find subgames
    subgames = generateSubgames(state);
    subgameCount = subgames.size();

    //print
    for (int i = 0; i < state->boardSize; i++) {
        //cout << playerNumberToChar(board[i]);
    }
    //cout << endl;
    //cout << "Subgames: " << subgames.size() << endl;


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
                //cout << "Match" << endl;

                //forward pass
                bool diff = true;

                for (int k = 0; k < l1; k++) {
                    if (board[s1 + k] != 3 - board[s2 + k]) {
                        diff = false;
                        break;
                    }
                }

                if (diff) {
                    //cout << "Simplify on forward pass" << endl;
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
                    //cout << "Simplify on backward pass" << endl;
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
        string subBoard(&board[s], length);
        //int outcome = db->get(length, subBoard.data());
        unsigned char *entry = db->get(length, subBoard.data());
        int outcome = DB_GET_OUTCOME(entry);

        //cout << "Deleting" << endl;
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
    sort(subgames.begin(), subgames.end(), subgameCompare);


    int position = 0;
    char newBoard[state->boardSize];
    memset(newBoard, 0, state->boardSize);

    for (int i = 0; i < subgameCount; i++) {
        int s = subgames[i].first;
        int e = subgames[i].second;

        memcpy(&newBoard[position], &board[s], e - s);
        position += (e - s) + 1;
    }

    memcpy(state->board, newBoard, state->boardSize);

    for (int i = 0; i < state->boardSize; i++) {
    //    cout << playerNumberToChar(board[i]);
    }
    //cout << endl;


    //cout << "Board: ";
    //for (int i = 0; i < state->boardSize; i++) {
    //    cout << playerNumberToChar(state->board[i]);
    //}
    //cout << endl;



    size_t minSize = gameLength(state->boardSize, state->board);
    char *minBuffer = new char[minSize];
    memset(minBuffer, 0, minSize);

    if (state->boardSize > 0) {
        memcpy(minBuffer, state->board, minSize);
    }

    //cout << "board: ";
    //for (int i = 0; i < minSize; i++) {
    //    cout << playerNumberToChar(minBuffer[i]);
    //}
    //cout << endl;


    delete[] state->board;
    state->board = minBuffer;
    state->boardSize = minSize;

    
    
    //delete[] minBuffer;




    //cout << endl;
}

bool subgameLengthCompare(const pair<int, int> &a, const pair<int, int> &b) {
    return (a.second - a.first) > (b.second - b.first);
}


int BasicSolver::checkBounds(State *state) {
    return 0;
    //int roll = (*rng)() % 10000;
    //if (roll < 7000) {
    //    return 0;
    //}


    vector<pair<int, int>> sg = generateSubgames(state);


    int lowSum = 0;
    bool lowStar = false;

    int highSum = 0;
    bool highStar = false;

    for (auto it = sg.begin(); it != sg.end(); it++) {
        int size = it->second - it->first;
        if (size > DB_MAX_BOUND_BITS) {
            return 0;
        }

        unsigned char *dbEntry = db->get(size, state->board + it->first);

        if (DB_GET_OUTCOME(dbEntry) == OC_UNKNOWN) {
            return 0;
        }

        int low = DB_GET_BOUND(dbEntry, 0);
        int high = DB_GET_BOUND(dbEntry, 1);


        //cout << "[" << low << " " << high << "]" << endl;

        int lowVal = sign(low) * max(abs(low) - 1, 0);
        lowSum += lowVal;
        lowStar = abs(low) % 2 != 0 ? !lowStar : lowStar;

        int highVal = sign(high) * max(abs(high) - 1, 0);
        highSum += highVal;
        highStar = abs(high) % 2 != 0 ? !highStar : highStar;

    }

    //cout << "{" << lowSum << " " << highSum << "}" << endl;

    if (lowSum - (lowStar ? 1 : 0) > 0) {
        //cout << "Black cut" << endl;
        //for (int i = 0; i < boardSize; i++) {
        //    cout << playerNumberToChar(state->board[i]);
        //}
        //cout << endl;
        //while(1) {}

        return 1;
    }

    if (highSum + (highStar ? 1 : 0) < 0) {
        //cout << "White cut" << endl;
        //for (int i = 0; i < boardSize; i++) {
        //    cout << playerNumberToChar(state->board[i]);
        //}
        //cout << endl;
        //while(1) {}


        return 2;
    }




    return 0;
}

pair<int, bool> BasicSolver::searchID(State *state, int p, int n, int depth, Bound alpha, Bound beta, Bound &rb1, Bound &rb2) {
    node_count += 1;
    updateTime();
    if (outOfTime) {
        return pair<int, bool>(0, false);
    }

    char oldBoard[state->boardSize];
    uint8_t oldBoardSize = state->boardSize;
    memcpy(oldBoard, state->board, state->boardSize);
    simplify(state, depth);

//    int boundWin = checkBounds(state);
//
//    if (boundWin != 0) {
//        RESIZESTATEBOARD(state, oldBoardSize);
//        memcpy(state->board, oldBoard, state->boardSize);
//        return pair<int, bool>(boundWin, true);
//    }

    Bound a, b;
    if (!limitCompletions) {
        generateBounds(state, a, b);
    }

    //lookup entry
    //if solved, return
    int code = state->code(p);
    char *entry = getTablePtr(code);

    bool validEntry = validateTableEntry(state, p, entry);
    if (validEntry && OUTCOME(entry) != EMPTY) {
        RESIZESTATEBOARD(state, oldBoardSize);
        memcpy(state->board, oldBoard, state->boardSize);
        return pair<int, bool>(OUTCOME(entry), true);
    }

    if (!validEntry && PLAYER(entry) != 0) {
        //collisions++;
    }



    //generate subgames, look them up in the database
    vector<pair<int, int>> subgames = generateSubgames(state);

    //sort subgames by length
    sort(subgames.begin(), subgames.end(), subgameLengthCompare);

    vector<int> lengths;
    vector<int> outcomes;

    //count outcomes
    int counts[5];
    for (int i = 0; i < 5; i++) {
        counts[i] = 0;
    }

    int outcomeMask = 0;


    //this is OK because 64 is the maximum board size
    uint64_t opposingPositionMask = 0;

    for (auto it = subgames.begin(); it != subgames.end(); it++) {
        int length = it->second - it->first;
        lengths.push_back(length);

        unsigned char *entry = db->get(length, &state->board[it->first]);
        int outcome = DB_GET_OUTCOME(entry);
        //int outcome = db->get(length, &state->board[it->first]);
        outcomes.push_back(outcome);

        counts[outcome] += 1;
        outcomeMask |= (1 << outcome);

        if ((outcome == OC_B && p == WHITE) || (outcome == OC_W && p == BLACK)) {
            uint64_t mask = 1;

            for (int i = 1; i < length; i++) {
                mask <<= 1;
                mask |= 1;
            }

            mask <<= it->first;
            opposingPositionMask |= mask;
        }
    }

    //Only Bs
    if ((outcomeMask & ~(1 << OC_B)) == 0 && counts[OC_B] > 0) {

        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            //memcpy(entry, state->board, boardSize);
            RESIZETTBOARD(entry, state->boardSize);
            memcpy(BOARDPTR(entry), state->board, state->boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = BLACK;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = 127 * (p == BLACK ? 1 : -1);
        }        

        RESIZESTATEBOARD(state, oldBoardSize);
        memcpy(state->board, oldBoard, state->boardSize);

        return pair<int, bool>(OC_B, true);
    }

    //Only Ws
    if ((outcomeMask & ~(1 << OC_W)) == 0 && counts[OC_W] > 0) {

        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            //memcpy(entry, state->board, boardSize);
            RESIZETTBOARD(entry, state->boardSize);
            memcpy(BOARDPTR(entry), state->board, state->boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = WHITE;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = 127 * (p == WHITE ? 1 : -1);
        }        

        RESIZESTATEBOARD(state, oldBoardSize);
        memcpy(state->board, oldBoard, state->boardSize);

        return pair<int, bool>(OC_W, true);
    }

    //Only one N
    if ((outcomeMask & ~(1 << OC_N)) == 0 && counts[OC_N] == 1) {
        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            //memcpy(entry, state->board, boardSize);
            RESIZETTBOARD(entry, state->boardSize);
            memcpy(BOARDPTR(entry), state->board, state->boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = p;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = 127;
        }        

        RESIZESTATEBOARD(state, oldBoardSize);
        memcpy(state->board, oldBoard, state->boardSize);

        return pair<int, bool>(p, true); //current player wins
    }


    //Use differences
    char boardCopy[state->boardSize];
    int boardCopySize = state->boardSize;
    memcpy(boardCopy, state->board, state->boardSize);

    for (int i = 0; i < subgames.size(); i++) {
        pair<int, int> &sg = subgames[i];

        if (outcomes[i] == p) {
            for (int j = 0; j < lengths[i]; j++) {
                state->board[sg.first + j] = 0;
            }
           
            //Don't swap players or increase depth -- we haven't played a move
            Bound cb1, cb2;
            pair<int, bool> result = searchID(state, p, n, depth, alpha, beta, cb1, cb2);
            if (outOfTime) {
                //memcpy(state->board, oldBoard, state->boardSize);
                return result;
            }
            RESIZESTATEBOARD(state, boardCopySize);
            memcpy(state->board, boardCopy, state->boardSize);

            if (result.second && result.first == p) {

                if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
                    //memcpy(entry, state->board, boardSize);
                    RESIZETTBOARD(entry, state->boardSize);
                    memcpy(BOARDPTR(entry), state->board, state->boardSize);
                    PLAYER(entry) = p;
                    OUTCOME(entry) = p;
                    BESTMOVE(entry) = 0;
                    DEPTH(entry) = depth;
                    HEURISTIC(entry) = 127;
                }        
                RESIZESTATEBOARD(state, oldBoardSize);
                memcpy(state->board, oldBoard, state->boardSize);

                return result;
            }

            if (!limitCompletions) {
                bool abCut = false;

                if (p == 1) {
                    if (cb1 > rb1) {
                        rb1 = cb1;
                    }
                    if (cb1 >= beta) {
                        abCut = true;
                    } else {
                        if (cb1 > alpha) {
                            alpha = cb1;
                        }
                    }
                } else {
                    if (cb2 < rb2) {
                        rb2 = cb2;
                    }
                    if (cb2 <= alpha) {
                        abCut = true;
                    } else {
                        if (cb2 < beta) {
                            beta = cb2;
                        }
                    }
                }

                if (abCut && depth > 0 && !limitCompletions) {
                    RESIZESTATEBOARD(state, oldBoardSize);
                    memcpy(state->board, oldBoard, state->boardSize);
                    return pair<int, bool>(0, false);
                }
            }


        }
    }

    //generate moves
    //check for terminal
    size_t moveCount;
    int *moves = state->getMoves(p, n, &moveCount);

    if (moveCount == 0) { //is terminal
        completed += 1;
        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            //memcpy(entry, state->board, boardSize);
            RESIZETTBOARD(entry, state->boardSize);
            memcpy(BOARDPTR(entry), state->board, state->boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = n;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = -127;
        }

        RESIZESTATEBOARD(state, oldBoardSize);
        memcpy(state->board, oldBoard, state->boardSize);
        return pair<int, bool>(n, true);
    }

    //Delete dominated moves
    vector<pair<int, int>> sg = generateSubgames(state);

    for (int i = 0; i < sg.size(); i++) {
        int start = sg[i].first;
        int end = sg[i].second; //index after end
        int len = end - start;

        unsigned char *dbEntry = db->get(len, &state->board[start]);

        uint64_t dominated = DB_GET_DOMINATED(dbEntry, p);

        int moveIndex = 0;
        for (int j = 0; j < moveCount; j++) {
            int from = moves[2 * j];
            int to = moves[2 * j + 1];

            if (from >= start && from < end) { //found move
                if ((dominated >> moveIndex) & ((uint64_t) 1)) {
                    //cout << "FOUND" << endl;
                    moves[2 * j] = -1;
                    moves[2 * j + 1] = -1;
                }
                moveIndex++;
            }
        }

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
            //memcpy(entry, state->board, boardSize);
            RESIZETTBOARD(entry, state->boardSize);
            memcpy(BOARDPTR(entry), state->board, state->boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = EMPTY;
            BESTMOVE(entry) = 0;
            DEPTH(entry) = depth;
            HEURISTIC(entry) = h;
        }        

        RESIZESTATEBOARD(state, oldBoardSize);
        memcpy(state->board, oldBoard, state->boardSize);
        delete[] moves;
        return pair<int, bool>(h, false);
    }

    //visit children, starting with best; find new best and update heuristic
    //if solved, save value and return

    vector<int> moveOrder;


    int bestMove = -1;
    bool checkedBestMove = false;
    if (validEntry) {
        bestMove = BESTMOVE(entry);
    }

    for (int i = 0; i < moveCount; i++) {
        if (moves[2 * i] == -1) {
            continue;
        }

        if (i == bestMove || ((((uint64_t) 1) << moves[2 * i]) & opposingPositionMask) != 0) {
            continue;
        }
        moveOrder.push_back(i);
    }

    shuffle(moveOrder.begin(), moveOrder.end(), *rng);

    for (int i = 0; i < moveCount; i++) {
        if (moves[2 * i] == -1) {
            continue;
        }

        if ((((uint64_t) 1) << moves[2 * i]) & opposingPositionMask) {
            moveOrder.push_back(i);
        }
    }

    if (bestMove != -1) {
        if (moves[2 * bestMove] != -1) {
            moveOrder.push_back(bestMove);
        } else {
            bestMove = -1;
        }
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
        //    cout << i << " ";
        //}
        
        int from = moves[2 * i];
        int to = moves[2 * i + 1];

        if (from == -1) { //this move was pruned
            continue;
        }

        state->play(from, to, undoBuffer);

        Bound cb1, cb2;

        int beforeSize = state->boardSize;
        pair<int, bool> result = searchID(state, n, p, depth + 1, alpha, beta, cb1, cb2);
        if (beforeSize != state->boardSize) {
            cout << "Size change" << endl;
            while (1) { }
        }
        if (outOfTime) {
            //memcpy(state->board, oldBoard, state->boardSize);
            delete[] moves;
            return result;
        }
    
        state->undo(undoBuffer);

        allProven &= result.second;

        if (result.second && result.first == p) {
            if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
                //memcpy(entry, state->board, boardSize);
                RESIZETTBOARD(entry, state->boardSize);
                memcpy(BOARDPTR(entry), state->board, state->boardSize);
                PLAYER(entry) = p;
                OUTCOME(entry) = p;
                BESTMOVE(entry) = i;
                DEPTH(entry) = depth;
                HEURISTIC(entry) = 127;
            }

            RESIZESTATEBOARD(state, oldBoardSize);
            memcpy(state->board, oldBoard, state->boardSize);
            delete[] moves;
            return pair<int, bool>(p, true);
        }

        //update ab
        if (!limitCompletions) {
            bool abCut = false;

            if (p == 1) {
                if (cb1 > rb1) {
                    rb1 = cb1;
                }
                if (cb1 >= beta) {
                    abCut = true;
                } else {
                    if (cb1 > alpha) {
                        alpha = cb1;
                    }
                }
            } else {
                if (cb2 < rb2) {
                    rb2 = cb2;
                }
                if (cb2 <= alpha) {
                    abCut = true;
                } else {
                    if (cb2 < beta) {
                        beta = cb2;
                    }
                }
            }

            if (abCut && depth > 0 && !limitCompletions) {
                RESIZESTATEBOARD(state, oldBoardSize);
                memcpy(state->board, oldBoard, state->boardSize);
                delete[] moves;
                return pair<int, bool>(0, false);
            }
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
    //    cout << endl;
    //}

    delete[] moves;

    if (allProven) {
        if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            //memcpy(entry, state->board, boardSize);
            RESIZETTBOARD(entry, state->boardSize);
            memcpy(BOARDPTR(entry), state->board, state->boardSize);
            PLAYER(entry) = p;
            OUTCOME(entry) = n;
            BESTMOVE(entry) = newBestMove; //these two values don't matter -- node result is known
            DEPTH(entry) = depth;
            HEURISTIC(entry) = -127;

        }

        RESIZESTATEBOARD(state, oldBoardSize);
        memcpy(state->board, oldBoard, state->boardSize);
        return pair<int, bool>(n, true);
    }


    if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        //memcpy(entry, state->board, boardSize);
        RESIZETTBOARD(entry, state->boardSize);
        memcpy(BOARDPTR(entry), state->board, state->boardSize);
        PLAYER(entry) = p;
        OUTCOME(entry) = EMPTY;
        BESTMOVE(entry) = newBestMove;
        DEPTH(entry) = depth;
        HEURISTIC(entry) = bestVal;
    }
    RESIZESTATEBOARD(state, oldBoardSize);
    memcpy(state->board, oldBoard, state->boardSize);
    return pair<int, bool>(bestVal, false);
}


/*
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
*/

/*
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
*/

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
    auto now = chrono::steady_clock::now();
    double elapsed = chrono::duration<double>(now - startTime).count();

    if (elapsed >= timeLimit) {
        outOfTime = true;
    }
}
