#ifndef PILL_H
#define PILL_H

// Pills are made up of two tile entities
typedef struct {
    int pill_half_1;
    int pill_half_2;
} pill_t;

// Defining pill movement directions
typedef enum {
    MOVE_DOWN,
    MOVE_RIGHT,
    MOVE_LEFT
} move_direction_t;

// Defining pill rotation directions
typedef enum {
    ROTATE_CLOCKWISE,
    ROTATE_COUNTERCLOCKWISE
} rotate_direction_t;

// Move the pill right, left, or down
void pill_move(pill_t *pill, move_direction_t direction);

// Rotate the pill clockwise or counterclockwise
void pill_rotate(pill_t *pill, rotate_direction_t direction);

#endif /* PILL_H */