#pragma once
#include <cstddef>
#include <cstdint>

#define UNDO_BUFFER_SIZE (sizeof(int) + 2 * sizeof(uint8_t))

int *getMoves(uint8_t *board, size_t len, int player, size_t *moveCount);


int code(uint8_t *board, size_t len, int player);
void play(uint8_t *board, uint8_t *undoBuffer, int from, int to);
void undo(uint8_t *board, uint8_t *undoBuffer);
