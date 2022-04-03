#include <iostream>
#include <cstring>
#include "utils.h"
#include "state.h"
#include "solver.h"
#include "database.h"
#include <chrono>
#include <set>
#include <string>
using namespace std;

bool all_black(string s){
    set <char> s1;
   
    for ( int i=0 ; i < s.length() ; i++){
        s1.insert(s[i]);}
     
    if ( s1.size() == 1 && s[0]=='B'){
        // cout << "YES";
        return true;
    }
    else{
        // cout << "NO";
        return false;
    }
}
bool all_white(string s){
    set <char> s1;
   
    for ( int i=0 ; i < s.length() ; i++){
        s1.insert(s[i]);}
     
    if ( s1.size() == 1 && s[0]=='W'){
        // cout << "YES";
        return true;
    }
    else{
        // cout << "NO";
        return false;
    }
}

int get_pattern_value(char* board, int length){
    // all patterns
    string s = "";
    int up_count = 0;
    int down_count = 0;
    int star_count = 0;

    for (int i = 0; i < length; i++) {
        s += board[i];

    }

    if (s[0] =='W' && all_black(s.substr(1, length))){
        //length-1 * up + lenth * star
        up_count += length-2;
        star_count += length-1;
        star_count = star_count%2;
        return (star_count + 10*down_count + 100*up_count);
    }
    else if(s[0] =='B' && all_white(s.substr(1, length))){
        //length-1 * down + lenth * star
        down_count += length-2;
        star_count += length-1;
        star_count = star_count%2;

        return (star_count + 10*down_count + 100*up_count);

    }
    else if(length%3==0){
        int count =0;
        for (int i = 0; i < length/3 ; i++) {
            if (s.substr(0+i*3, 3+i*3) =="BBW"){
                count +=1;}
        
        }
        if (count == length/3){
            return ((length/3 +1)/2) *100;
        }
         else{
            return VAL_UNK;
        }

        count = 0;
        for (int i = 0; i < length/3 ; i++) {
            if (s.substr(0+i*3, 3+i*3) =="WWB"){
                count +=1;
            }
    
        }
        if (count == length/3){
            return ((length/3 +1)/2) *100;
        }
        else{
            return VAL_UNK;
        }

    }

    else{

        return VAL_UNK;
    }
}



int gameResult(Database &db, char *board, int boardSize, int player) {
    int result;

    auto start = std::chrono::steady_clock::now();
    BasicSolver *solver = new BasicSolver(player, boardSize, &db);
    solver->timeLimit = 1000000000.0;
    solver->startTime = start;


    char boardText[boardSize + 1];
    memcpy(boardText, board, boardSize);
    boardText[boardSize] = 0;
    for (int i = 0; i < boardSize; i++) {
        boardText[i] = playerNumberToChar(boardText[i]);
    }

    State *root = new State(boardText, player);

    result = solver->solveID(root, player, opponentNumber(player));

    delete solver;
    delete root;

    return result;
}



using namespace std;

void printBits(int x, int length) {
    for (int i = 0; i < length; i++) {
        cout << ((x >> i) & 1);
    }
    cout << endl;
}

int main() {

    Database db;
    //db.load(); //might be wrong to load here

    int maxLength = DB_MAX_BITS;
    int maxGame = 0;
    string s1 = "";
    string s2 = "";

                
//BLACK = 0
vector<string> string_list;

    for (int m = 2; m < DB_MAX_BITS/2; m++) {
            
        for (int n = 2; n < DB_MAX_BITS/2; n++) {
                s1 = string(m, 'B') + string(n, 'W');
                s2 = string(m, 'W') + string(n, 'B');
                string_list.push_back(s1);
                string_list.push_back(s2);
                
            }
    }
    
    bool found = false;
    string s ="";
    for (int length = 1; length <= maxLength; length++) {
        maxGame *= 2;
        maxGame += 1;

        char boardText[length + 1];
        char board[length + 1];
        char mirrorBoard[length + 1];

        boardText[length] = 0;
        board[length] = 0;
        mirrorBoard[length] = 0;

        unsigned char *entry;

        for (int game = 0; game <= maxGame; game++) {
            s = "";
            bool mirror = false;

            if (game > (maxGame + 2) / 2) {
                mirror = true;
            }

            for (int i = 0; i < length; i++) {
                if ((game >> i) & 1) {
                    boardText[i] = 'W';
                    
                    board[i] = WHITE;
                    s += 'W';
                } else {
                    boardText[i] = 'B';
                    board[i] = BLACK;
                    s += 'B';
                }
                
            }

            entry = db.get(length, board);
            found = (std::find(string_list.begin(), string_list.end(), s) != string_list.end());
            
            if (found == true){
                DB_SET_VALUE(entry, 0);
                continue;

            }
            
            if (mirror) {
                for (int i = 0; i < length; i++) {
                    if ((game >> i) & 1) {
                        mirrorBoard[i] = BLACK;
                        
                    } else {
                        mirrorBoard[i] = WHITE;
                    }
                }
                entry = db.get(length, mirrorBoard);
            }
            

            uint64_t gValue = VAL_UNK;
            gValue = get_pattern_value(boardText, length);
            DB_SET_VALUE(entry, gValue);

        
    }

    }

    // cout<<"END \n\n\n\n";
    maxLength = DB_MAX_BITS;
    maxGame = 0;
    for (int length = 1; length <= maxLength; length++) {

        maxGame *= 2;
        maxGame += 1;

        char boardText[length + 1];
        char board[length + 1];
        char mirrorBoard[length + 1];

        boardText[length] = 0;
        board[length] = 0;
        mirrorBoard[length] = 0;

        unsigned char *entry;

        for (int game = 0; game <= maxGame; game++) {
            // cout << "Length - Game: " << length << " " << game << endl;
            // printBits(game, length);


            bool mirror = false;

            if (game > (maxGame + 2) / 2) {
                mirror = true;
            }

            //mirror = false;

            for (int i = 0; i < length; i++) {
                if ((game >> i) & 1) {
                    boardText[i] = 'W';
                    board[i] = WHITE;
                } else {
                    boardText[i] = 'B';
                    board[i] = BLACK;
                }
            }

            if (mirror) {
                for (int i = 0; i < length; i++) {
                    if ((game >> i) & 1) {
                        mirrorBoard[i] = BLACK;
                    } else {
                        mirrorBoard[i] = WHITE;
                    }
                }
            }


            int outcome = 0;
            uint64_t domBlack = 0;
            uint64_t domWhite = 0;

            if (mirror) {
                entry = db.get(length, mirrorBoard);
                outcome = DB_GET_OUTCOME(entry);
                domBlack = DB_GET_DOMINATED(entry, 1);
                domWhite = DB_GET_DOMINATED(entry, 2);

                switch (outcome) {
                    case OC_B:
                        outcome = OC_W;
                        break;

                    case OC_W:
                        outcome = OC_B;
                        break;

                    case OC_N:
                        outcome = OC_N;
                        break;

                    case OC_P:
                        outcome = OC_P;
                        break;
                }


                entry = db.get(length, board);
                DB_SET_OUTCOME(entry, outcome);
                DB_SET_DOMINATED(entry, 1, domWhite); //invert dominance
                DB_SET_DOMINATED(entry, 2, domBlack);

                continue;
            }


            // cout << "solving" << endl;
            int result1, result2;

            {
                auto start = std::chrono::steady_clock::now();
                BasicSolver *solver = new BasicSolver(1, length, &db);
                solver->timeLimit = 1000000000.0;
                solver->startTime = start;

                State *root = new State(boardText, 1);
                
                result1 = solver->solveID(root, 1, opponentNumber(1));

                delete solver;
                delete root;
            }

            {
                auto start = std::chrono::steady_clock::now();
                BasicSolver *solver = new BasicSolver(2, length, &db);
                solver->timeLimit = 1000000000.0;
                solver->startTime = start;

                State *root = new State(boardText, 2);
                
                result2 = solver->solveID(root, 2, opponentNumber(2));

                delete solver;
                delete root;
            }

            outcome = 0;

            if (result1 == result2) {
                outcome = result1;
            } else if (result1 == 1) {
                outcome = OC_N;
            } else {
                outcome = OC_P;
            }

            // std::cout << "Lookup" << std::endl;
            entry = db.get(length, board);
            if (DB_GET_OUTCOME(entry) != 0) {
                // cout << "Overwriting outcome: " << DB_GET_OUTCOME(entry) << endl;
                while(1){}
            }
            uint64_t a = 0;
            DB_SET_OUTCOME(entry, outcome);            
            a = DB_GET_VALUE(entry);
            if (a!=VAL_UNK){
                for (int i = 0; i < length; i++) {
                cout << boardText[i];
                 }
                cout << endl;
                cout<<a<<"\tGAME VALUE\n";
            }


            if (length <= DB_MAX_DOMINANCE_BITS ) {
                //find dominated moves for B
                char sumBoard[length + 1 + length + 1];
                sumBoard[length + 1 + length] = 0;
                sumBoard[length] = E;

                {
                    State s1(boardText, 1);
                    State s2(boardText, 1);

                    char boardCopy[length];
                    memcpy(boardCopy, s2.board, length);

                    size_t moveCount = 0;
                    int *moves = s1.getMoves(1, 2, &moveCount);

                    char undo1[sizeof(int) + 2 * sizeof(char)];
                    char undo2[sizeof(int) + 2 * sizeof(char)];

                    for (int i = 0; i < moveCount; i++) {
                        s1.play(moves[2 * i], moves[2 * i + 1], undo1);

                        for (int j = i + 1; j < moveCount; j++) {
                            s2.play(moves[2 * j], moves[2 * j + 1], undo2);

                            memcpy(sumBoard, s1.board, length);
                            memcpy(&sumBoard[length + 1], s2.board, length);


                            for (int k = length + 1; k < 2 * length + 1; k++) {
                                sumBoard[k] = opponentNumber(sumBoard[k]);
                            }


                            int bFirst = gameResult(db, sumBoard, 2 * length + 1, 1);
                            int wFirst = gameResult(db, sumBoard, 2 * length + 1, 2);

                            memcpy(s2.board, boardCopy, length);

                            if (bFirst == wFirst) {
                                if (bFirst == 1) { // I - J is positive for black --> I > J
                                    domBlack |= (((uint64_t) 1) << j);
                                } else { // I - J is negative for black --> I < J
                                    domBlack |= (((uint64_t) 1) << i);
                                }
                                //while (1) {}
                            }



                            //s2.undo(undo2);
                        }

                        s1.undo(undo1);
                    }

                    if (moveCount > 0) {
                        delete[] moves;
                    }
                }

                //find dominated moves for W
                {
                    State s1(boardText, 2);
                    State s2(boardText, 2);

                    char boardCopy[length];
                    memcpy(boardCopy, s2.board, length);

                    size_t moveCount = 0;
                    int *moves = s1.getMoves(2, 1, &moveCount);

                    char undo1[sizeof(int) + 2 * sizeof(char)];
                    char undo2[sizeof(int) + 2 * sizeof(char)];

                    for (int i = 0; i < moveCount; i++) {
                        s1.play(moves[2 * i], moves[2 * i + 1], undo1);

                        for (int j = i + 1; j < moveCount; j++) {
                            s2.play(moves[2 * j], moves[2 * j + 1], undo2);

                            memcpy(sumBoard, s1.board, length);
                            memcpy(&sumBoard[length + 1], s2.board, length);


                            for (int k = length + 1; k < 2 * length + 1; k++) {
                                sumBoard[k] = opponentNumber(sumBoard[k]);
                            }


                            int bFirst = gameResult(db, sumBoard, 2 * length + 1, 1);
                            int wFirst = gameResult(db, sumBoard, 2 * length + 1, 2);

                            memcpy(s2.board, boardCopy, length);


                            if (bFirst == wFirst) {
                                if (bFirst == 2) { // I - J is positive for white --> I > J
                                    domWhite |= (((uint64_t) 1) << j);
                                } else { // I - J is negative for white --> I < J
                                    domWhite |= (((uint64_t) 1) << i);
                                }
                                //while (1) {}
                            }



                            //s2.undo(undo2);
                        }

                        s1.undo(undo1);
                    }

                    if (moveCount > 0) {
                        delete[] moves;
                    }
                }


                DB_SET_DOMINATED(entry, 1, domBlack);
                DB_SET_DOMINATED(entry, 2, domWhite);
                // std::cout << domBlack << " " << domWhite << std::endl;
            }
            //end of dominated


            // cout << endl;
        }

    }


    db.save();
    return 0;
}
