#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include "utils.h"
#include "database3.h"
#include "solver.h"

extern Database *db;
extern Solver *solver;


extern std::map<triple<int, int, int>, std::vector<uint64_t> *> replacementMap;
