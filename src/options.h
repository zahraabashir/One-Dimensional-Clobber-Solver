#pragma once

#define FIXED_BOARD_SIZE 70


// allow replacement of games with simpler games (games having smaller move counts)
#define SOLVER_SUBSTITUTE

// allow alpha beta pruning
//#define SOLVER_ALPHA_BETA

// check bounds at the start of a node to see if it's a win
//#define SOLVER_CHECK_BOUNDS

// don't use variable board sizes -- fix board sizes to FIXED_BOARD_SIZE, likely incompatible with SOLVER_SUBSTITUTE
//#define SOLVER_FIX_MEMORY_LEAK

// ignore dominated moves
#define SOLVER_DELETE_DOMINATED_MOVES

// try searching on a board after deleting a subgame whose outcome agrees with the current player
#define SOLVER_DELETE_SUBGAMES

// 0: sum of black and white moves, 1: sum of black and white undominated moves, 2: length (in tiles)
// changing this requires rebuilding the database
#define SUBGAME_COMPLEXITY_METRIC 0

//#define STRICT_BOUNDS

//Failed experiment using N positions for move ordering
//#define SEARCH_VERSION 2


#define ALTERNATE_ID_SCALING


// Max bits of index into the database (database will generate all connected games from sizes 1 to this size, inclusive)
#define DB_MAX_BITS 16

// connected games from 1 to this number will have dominance information
#define DB_MAX_DOMINANCE_BITS 16

// connected games from 1 to this number will have bound information
// CAN CHANGE TO 16 WITH db3-r12-16s.bin
#define DB_MAX_BOUND_BITS 12


// games are only substituted if they are in the database, and have both dominance and bound information (so the minimum of these will determine what games can be substituted)


#define RMAP_SIZE 12

// CAN CHANGE TO 16 WITH db3-r12-16s.bin
#define DB_MAX_SUB_BITS 12


#define BIT_VECTOR_SIZE 2

#define STATIC_MC_DELTA 1
#define STATIC_EXTRA
#define SIMPLIFY_ALTERNATE_SORT
