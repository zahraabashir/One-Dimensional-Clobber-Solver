#pragma once

// allow replacement of games with simpler games (games having smaller move counts)
#define SOLVER_SUBSTITUTE

// ignore dominated moves
#define SOLVER_DELETE_DOMINATED_MOVES

// try searching on a board after deleting a subgame whose outcome agrees with the current player
#define SOLVER_DELETE_SUBGAMES

// Max bits of index into the database (database will generate all games from sizes 1 to this size, inclusive)
#define DB_MAX_BITS 16

// games of sizes from 1 to this number will have dominance information
#define DB_MAX_DOMINANCE_BITS 16

// connected games from 1 to this number will have bound information
// CAN CHANGE TO 16 WITH db3-r12-16s.bin
#define DB_MAX_BOUND_BITS 12


// games are only substituted if they are in the database, and have both dominance and bound information (so the minimum of these will determine what games can be substituted)

// games can be substituted with other games that are up to/including this length
#define RMAP_SIZE 12

#define DB_MAX_SUB_BITS std::min({DB_MAX_BITS, DB_MAX_DOMINANCE_BITS, DB_MAX_BOUND_BITS})



/////////////              Weird/unused stuff below (probably shouldn't change)
#define STATIC_MC_DELTA 1
#define STATIC_EXTRA
#define SIMPLIFY_ALTERNATE_SORT
#define ALTERNATE_ID_SCALING
// database generation setting -- should be disabled
//#define STRICT_BOUNDS

