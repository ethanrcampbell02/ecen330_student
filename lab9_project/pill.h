#ifndef PILL_H
#define PILL_H

#include "board.h"

// Orientations of the pill 
typedef enum {
    ORIENTATION_HORIZONTAL,
    ORIENTATION_VERTICAL,
    ORIENTATION_SINGLE,
    ORIENTATION_EMPTY
} orientation_t;

// Defining pill movement types (shifts and rotations)
typedef enum {
    MOVE_DOWN,
    MOVE_RIGHT,
    MOVE_LEFT,
    ROTATE_CLOCKWISE,
    ROTATE_COUNTERCLOCKWISE
} move_type_t;

// Pills are made up of two tile entities 
typedef struct {
    entity_t *pill_half_1;
    entity_t *pill_half_2;
    orientation_t orientation;
} pill_t;

// Initializes the given pill to the hidden state
void pill_initHidden(pill_t *pill);

// Initializes the given pill at the top of the screen
void pill_initTop(pill_t *pill, board_t *board);

// Move the pill right, left, or down or rotate CW or CCW
// * Moves: down, left, right
// * Rotations: clockwise, counterclockwise
void pill_move(pill_t *pill, board_t *board, move_type_t direction);

void pill_test(pill_t *pill);

#endif /* PILL_H */