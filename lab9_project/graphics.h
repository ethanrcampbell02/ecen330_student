#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "board.h"
#include "pill.h"
#include <stdbool.h>

// Initialize display and draw the board
void drMarioDisplay_init();

// Erases the current tile
void drMarioDisplay_clearTile(location_t location);

// Draws the given virus
void drMarioDisplay_drawVirus(entity_t *virus);

// Draws the given pill
void drMarioDisplay_drawPill(pill_t *pill);

#endif /* GRAPHICS_H */