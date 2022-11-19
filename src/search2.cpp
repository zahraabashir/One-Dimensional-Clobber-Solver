#error Don't use SEARCH_VERSION 2

pair<int, bool> Solver::searchID(uint8_t *board, size_t boardLen, int n, int p, int depth) {
    _validEntry = false;

    DBOUT(
        cout << endl;
        printBoard(board, boardLen, false);
        cout << " " << playerNumberToChar(n) << " " << depth << endl;

    );


    node_count += 1;

    size_t sboardLen = boardLen;
    uint8_t *sboard = new uint8_t[boardLen];
    memcpy(sboard, board, boardLen);

    simplify(&sboard, &sboardLen);

    DBOUT(
        cout << "SIMPLIFY:" << endl;
        printBoard(sboard, sboardLen, false);
        cout << " " << playerNumberToChar(n) << " " << depth << endl;

    );


    {
        uint8_t *dbEntry = db->get(sboard, sboardLen);
        if (dbEntry) {
            uint8_t outcome = *db_get_outcome(dbEntry);

            if (outcome == OC_B || outcome == OC_W) {
                return pair<int, bool>(outcome, true);
            }
            if (outcome == OC_N) {
                return pair<int, bool>(n, true);
            }
            if (outcome == OC_P) {
                return pair<int, bool>(p, true);
            }

        }
    }




    //lookup entry
    //if solved, return
    int code = getCode(sboard, sboardLen, n);
    uint32_t hash2 = getHash2(sboard, sboardLen, n);
    uint8_t *blockPtr = getBlockPtr(code);
    uint8_t *entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 0);

    bool validEntry = entryPtr != 0;

    if (validEntry && *tt_get_outcome(entryPtr) != 0) {
        delete[] sboard;
        return pair<int, bool>(*tt_get_outcome(entryPtr), true);
    }

    //generate subgames, look them up in the database
    vector<pair<int, int>> subgames = generateSubgames(sboard, sboardLen);

    //sort subgames by length
    sort(subgames.begin(), subgames.end(),
        [](const pair<int, int> &a, const pair<int, int> &b) {
            return (a.second - a.first) > (b.second - b.first);
        }
    );

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
    uint64_t nPositionMask = 0;

    for (auto it = subgames.begin(); it != subgames.end(); it++) {
        int length = it->second - it->first;
        lengths.push_back(length);

        uint8_t *dbEntry = db->get(&sboard[it->first], length);
        int outcome = dbEntry ? *db_get_outcome(dbEntry) : OC_UNKNOWN;
        outcomes.push_back(outcome);

        counts[outcome] += 1;
        outcomeMask |= (1 << outcome);

        if ((outcome == OC_B && n == WHITE) || (outcome == OC_W && n == BLACK)) {
            uint64_t mask = 1;

            for (int i = 1; i < length; i++) {
                mask <<= 1;
                mask |= 1;
            }

            mask <<= it->first;
            opposingPositionMask |= mask;
        }


        if (outcome == OC_N) {
            uint64_t mask = 1;

            for (int i = 1; i < length; i++) {
                mask <<= 1;
                mask |= 1;
            }

            mask <<= it->first;
            nPositionMask |= mask;
        }


    }


    //Only Bs
    if ((outcomeMask & ~(1 << OC_B)) == 0 && counts[OC_B] > 0) {
        //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        if (true) {
            entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = BLACK;
            tt_get_best_moves(entryPtr)[0] = 0;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = 127 * (n == BLACK ? 1 : -1);
        }        
        delete[] sboard;
        return pair<int, bool>(OC_B, true);
    }

    //Only Ws
    if ((outcomeMask & ~(1 << OC_W)) == 0 && counts[OC_W] > 0) {
        //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        if (true) {
            entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = WHITE;
            tt_get_best_moves(entryPtr)[0] = 0;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = 127 * (n == WHITE ? 1 : -1);
        }        

        delete[] sboard;
        return pair<int, bool>(OC_W, true);
    }

    //Only one N
    if ((outcomeMask & ~(1 << OC_N)) == 0 && counts[OC_N] == 1) {
        //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        if (true) {
            entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = n;
            tt_get_best_moves(entryPtr)[0] = 0;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = 127;
        }        
        delete[] sboard;
        return pair<int, bool>(n, true); //current player wins
    }


    //Use differences
    #if defined(SOLVER_DELETE_SUBGAMES)

    for (int i = 0; i < subgames.size(); i++) {
        uint8_t sboardCopy[sboardLen];
        size_t sboardCopyLen = sboardLen;
        memcpy(sboardCopy, sboard, sboardLen);

        pair<int, int> &sg = subgames[i];

        if (outcomes[i] == n) {
            for (int j = 0; j < lengths[i]; j++) {
                sboardCopy[sg.first + j] = 0;
            }

            //Don't swap players or increase depth -- we haven't played a move
            pair<int, bool> result = searchID(sboardCopy, sboardCopyLen, n, p, depth);

            if (result.second && result.first == n) {

                //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
                entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
                if (true) {
                    *tt_get_valid(entryPtr) = true;
                    *tt_get_outcome(entryPtr) = n;
                    tt_get_best_moves(entryPtr)[0] = 0;
                    *tt_get_depth(entryPtr) = depth;
                    *tt_get_heuristic(entryPtr) = 127;
                }        
                delete[] sboard;
                return result;
            }


        }
    }
    #endif


    //generate moves
    //check for terminal
    size_t moveCount;
    int *moves = getMoves(sboard, sboardLen, n, &moveCount);

    if (moveCount == 0) { //is terminal
        completed += 1;
        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
        if (true) {
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = p;
            tt_get_best_moves(entryPtr)[0] = 0;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = -127;
        }

        delete[] sboard;
        return pair<int, bool>(p, true);
    }


    //Delete dominated moves
    #if defined(SOLVER_DELETE_DOMINATED_MOVES)
    vector<pair<int, int>> sg = generateSubgames(sboard, sboardLen);

    for (int i = 0; i < sg.size(); i++) {
        int start = sg[i].first;
        int end = sg[i].second; //index after end
        int len = end - start;

        uint8_t *dbEntry = db->get(&sboard[start], len);

        uint64_t dominated = dbEntry ? db_get_dominance(dbEntry)[n - 1] : 0;

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
    #endif




    //if deep, generate heuristic and return
    if (depth == maxDepth || (limitCompletions && (completed >= maxCompleted))) {
        completed += 1;

        size_t pMoveCount;
        int *pMoves = getMoves(sboard, sboardLen, p, &pMoveCount);

        if (pMoveCount > 0) {
            delete[] pMoves;
        }

        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
        int h = *tt_get_heuristic(entryPtr);
        if (!validEntry) {
            h = -pMoveCount;
        }

        //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
        if (true) {
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = EMPTY;
            tt_get_best_moves(entryPtr)[0] = 0;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = h;
        }        

        delete[] moves;
        delete[] sboard;
        return pair<int, bool>(h, false);
    }

    //visit children, starting with best; find new best and update heuristic
    //if solved, save value and return

    int bestMove = -1;
    bool checkedBestMove = false;

    entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 0);
    validEntry = entryPtr != 0;
    if (validEntry) {
        bestMove = tt_get_best_moves(entryPtr)[0];
    }



    //not opposing, not N position
    vector<int> moves1;
    for (int i = 0; i < moveCount; i++) {
        if (moves[2 * i] == -1) {
            continue;
        }

        bool isOpposing = ((((uint64_t) 1) << moves[2 * i]) & opposingPositionMask) != 0;
        bool isNPosition = ((((uint64_t) 1) << moves[2 * i]) & nPositionMask) != 0;

        if (i == bestMove || isOpposing || isNPosition) {
            continue;
        }
        moves1.push_back(i);
    }
    shuffle(moves1.begin(), moves1.end(), *rng);


    //Opposing moves
    vector<int> moves2;
    for (int i = 0; i < moveCount; i++) {
        if (moves[2 * i] == -1) {
            continue;
        }

        bool isOpposing = ((((uint64_t) 1) << moves[2 * i]) & opposingPositionMask) != 0;
        bool isNPosition = ((((uint64_t) 1) << moves[2 * i]) & nPositionMask) != 0;
        if (i == bestMove) {
            continue;
        }

        if (isOpposing) {
            moves2.push_back(i);
        }
    }
    shuffle(moves2.begin(), moves2.end(), *rng);

    //N positions
    vector<int> moves3;
    for (int i = 0; i < moveCount; i++) {
        if (moves[2 * i] == -1) {
            continue;
        }

        bool isOpposing = ((((uint64_t) 1) << moves[2 * i]) & opposingPositionMask) != 0;
        bool isNPosition = ((((uint64_t) 1) << moves[2 * i]) & nPositionMask) != 0;
        if (i == bestMove) {
            continue;
        }

        if (isNPosition) {
            moves3.push_back(i);
        }
    }
    shuffle(moves3.begin(), moves3.end(), *rng);

    vector<int> moveOrder;

    vector<vector<int> *> vecs = {&moves1, &moves2, &moves3};
    for (const vector<int> *v : vecs) {
        for (const auto m : *v) {
            moveOrder.push_back(m);
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

    uint8_t undoBuffer[UNDO_BUFFER_SIZE];

    int newBestMove = 0;

    bool allProven = true;


    for (auto it = moveOrder.rbegin(); it != moveOrder.rend(); it++) {
        int i = *it;

        if (checkedBestMove && i == bestMove) {
            continue;
        }

        int from = moves[2 * i];
        int to = moves[2 * i + 1];


        if (from == -1) { //this move was pruned
            continue;
        }

        play(sboard, undoBuffer, from, to);
        pair<int, bool> result = searchID(sboard, sboardLen, p, n, depth + 1);
        undo(sboard, undoBuffer);



        allProven &= result.second;

        if (result.second && result.first == n) {
            //if (true || depth >= DEPTH(entry) || PLAYER(entry) == 0) {
            if (true) {
                entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
                *tt_get_valid(entryPtr) = true;
                *tt_get_outcome(entryPtr) = n;
                tt_get_best_moves(entryPtr)[0] = i;
                *tt_get_depth(entryPtr) = depth;
                *tt_get_heuristic(entryPtr) = 127;
            }

            delete[] moves;
            delete[] sboard;
            return pair<int, bool>(n, true);
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

    delete[] moves;



    if (allProven) {
        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
        if (true) {
            *tt_get_valid(entryPtr) = true;
            *tt_get_outcome(entryPtr) = p;
            tt_get_best_moves(entryPtr)[0] = newBestMove;
            *tt_get_depth(entryPtr) = depth;
            *tt_get_heuristic(entryPtr) = -127;
        }

        delete[] sboard;
        return pair<int, bool>(p, true);
    }


    if (true) {
        entryPtr = getEntryPtr(blockPtr, sboard, sboardLen, n, hash2, 1);
        *tt_get_valid(entryPtr) = true;
        *tt_get_outcome(entryPtr) = EMPTY;
        tt_get_best_moves(entryPtr)[0] = newBestMove;
        *tt_get_depth(entryPtr) = depth;
        *tt_get_heuristic(entryPtr) = bestVal;
    }
    delete[] sboard;
    return pair<int, bool>(bestVal, false);
}

