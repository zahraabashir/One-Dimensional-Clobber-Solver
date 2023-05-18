#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include "utils.h"

extern std::map<triple<int, int, int>, std::vector<std::vector<uint64_t>>> sortedMap;

void makeSortedMap();
void checkSorting();
uint64_t simplifyIdx(const triple<int, int, int> &mapTriple, const uint64_t &idx);
