#include "graphics.h"
#include "pill.h"
#include "board.h"
#include "buttons.h"

// Game control states
typedef enum {
    instructions_st,
    newPill_st,
    pillControl_st,
    gravity_st  // TODO: this is temporary
} drMario_st_t;
static drMario_st_t currState;

// Game variables
static board_t board;
static pill_t pill;    // TODO: make this an array after testing

// Initialize the game control logic
void gameControl_init() {
    buttons_init();
    drMarioDisplay_init();
    board_initViruses(&board, CONFIG_NUM_VIRUSES);
    pill_initTop(&pill, &board);
    // printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    // pill_move(&pill, &board, MOVE_DOWN);
    // printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    // pill_move(&pill, &board, MOVE_DOWN);
    // printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    // pill_move(&pill, &board, MOVE_DOWN);

    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    printf("Pill location = (%d, %d)\n\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);

    pill_test(&pill);
    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    pill_test(&pill);
    printf("Pill location = (%d, %d)\n", pill.pill_half_1->location.x, pill.pill_half_1->location.y);
    pill_test(&pill);
}

// Tick the game control logic
void gameControl_tick() {
    // // Game state machine
    // switch (currState) {

    // }
}