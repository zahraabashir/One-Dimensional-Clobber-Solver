#pragma once
#include <cstddef>
#include <cstdint>

#define UNDO_BUFFER_SIZE (sizeof(int) + 2 * sizeof(uint8_t))

int *getMoves(const uint8_t *board, size_t len, int player, size_t *moveCount);

size_t getMoveCount(const uint8_t *board, size_t len, int player);


uint64_t getCode(uint8_t *board, size_t len, int player);
//uint32_t getHash2(uint8_t *board, size_t len, int player);
void play(uint8_t *board, uint8_t *undoBuffer, int from, int to);
void undo(uint8_t *board, uint8_t *undoBuffer);


