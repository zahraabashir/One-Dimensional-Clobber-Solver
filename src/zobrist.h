#pragma once
#include <cstdint>
#include <cstddef>

uint64_t getZobristHash(int player, const uint8_t* board, size_t len);
