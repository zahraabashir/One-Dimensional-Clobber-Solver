#pragma once

#define FIXED_BOARD_SIZE 70


// allow replacement of games with simpler games (games having smaller move counts)
#define SOLVER_SUBSTITUTE

// allow alpha beta pruning
#define SOLVER_ALPHA_BETA



// don't use variable board sizes -- fix board sizes to FIXED_BOARD_SIZE
#define SOLVER_FIX_MEMORY_LEAK
