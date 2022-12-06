#include "pill.h"
#include "board.h"
#include "config.h"
#include "graphics.h"

#include <stdbool.h>
#include <stdio.h>

// Initializes the given pill to the hidden state
void pill_initHidden(pill_t *pill) {
    // First pill half
    pill->pill_half_1->color = ENTITY_COLOR_EMPTY;
    pill->pill_half_1->type = ENTITY_EMPTY;
    pill->pill_half_1->location.x = 255;
    pill->pill_half_1->location.y = 255;

    // Second pill half
    pill->pill_half_2->color = ENTITY_COLOR_EMPTY;
    pill->pill_half_2->type = ENTITY_EMPTY;
    pill->pill_half_2->location.x = 255;
    pill->pill_half_2->location.y = 255;

    pill->orientation = ORIENTATION_EMPTY;
}

// Initializes the given pill at the top of the screen
void pill_initTop(pill_t *pill, board_t *board) {
    // Generate and place first pill half
    entity_t pill_half_1;
    pill->pill_half_1 = &pill_half_1;
    entity_randomizeColor(pill->pill_half_1);
    pill->pill_half_1->type = ENTITY_PILL_HALF;
    pill->pill_half_1->location.x = CONFIG_PILL_START_TILE_X;
    pill->pill_half_1->location.y = CONFIG_PILL_START_TILE_Y;
    board->tiles[CONFIG_PILL_START_TILE_X][CONFIG_PILL_START_TILE_Y] = pill->pill_half_1;

    // Generate and place second pill half
    entity_t pill_half_2;
    pill->pill_half_2 = &pill_half_2;
    entity_randomizeColor(pill->pill_half_2);
    pill->pill_half_2->type = ENTITY_PILL_HALF;
    pill->pill_half_2->location.x = CONFIG_PILL_START_TILE_X + 1;
    pill->pill_half_2->location.y = CONFIG_PILL_START_TILE_Y;
    board->tiles[CONFIG_PILL_START_TILE_X+1][CONFIG_PILL_START_TILE_Y] = pill->pill_half_2;

    pill->orientation = ORIENTATION_HORIZONTAL;
    drMarioDisplay_drawPill(pill);
}

// Checks if there is an unoccupied position below the pill
bool pill_canMoveDown(pill_t *pill, board_t *board) {
    uint8_t ph_1_x = 255;
    uint8_t ph_2_x = 255;
    uint8_t below_ph_1 = 255;
    uint8_t below_ph_2 = 255;

    switch (pill->orientation) {
        case ORIENTATION_HORIZONTAL:
            ph_1_x = pill->pill_half_1->location.x;
            ph_2_x = pill->pill_half_2->location.x;
            below_ph_1 = pill->pill_half_1->location.y + 1;
            below_ph_2 = pill->pill_half_2->location.y + 1;

            // Check if the space under the pill is OOB
            if (below_ph_1 >= CONFIG_BOARD_HEIGHT_TILES) {
                // display_drawCircle(150, 100, 10, DISPLAY_RED);
                return false;
            }

            // Check for occupied spaces under first half
            if (board->tiles[ph_1_x][below_ph_1] != NULL) {
                return false;
            }

            // Check for occupied spaces under second half
            if (board->tiles[ph_2_x][below_ph_2] != NULL) {
                return false;
            }

            // Nothing bad was found
            return true;

        case ORIENTATION_VERTICAL:
            ph_1_x = pill->pill_half_1->location.x;
            ph_2_x = pill->pill_half_2->location.x;
            below_ph_1 = pill->pill_half_1->location.y + 1;
            below_ph_2 = pill->pill_half_2->location.y + 1;

            // Check if the space under the pill is OOB
            if (below_ph_2 >= CONFIG_BOARD_HEIGHT_TILES) {
                return false;
            }

            // Check for occupied spaces under second half
            if (board->tiles[ph_2_x][below_ph_2] != NULL) {
                return false;
            }

            // Nothing bad was found
            return true;

        case ORIENTATION_SINGLE:
            ph_1_x = pill->pill_half_1->location.x;
            below_ph_1 = pill->pill_half_1->location.y + 1;

            // Check if the space under the pill is OOB
            if (below_ph_1 >= CONFIG_BOARD_HEIGHT_TILES) {
                return false;
            }

            // Check for occupied spaces under first half
            if (board->tiles[ph_1_x][below_ph_1] != NULL) {
                return false;
            }

            // Nothing bad was found
            return true;
    }
}

// TODO: Actually write code for each movement
// Move or rotate the pill in the given direction
// * Moves: down, left, right
// * Rotations: clockwise, counterclockwise
// Will not move the pill if the space is occupied
void pill_move(pill_t *pill, board_t *board, move_type_t direction) {
    // // TODO: actually super broken for some reason
    // location_t ph1_loc;
    // location_t ph2_loc;

    // // Copy location structures over
    // if (pill->pill_half_1 != NULL)
    //     ph1_loc = pill->pill_half_1->location;
    // if (pill->pill_half_2 != NULL)
    //     ph1_loc = pill->pill_half_2->location;
    
    // const test[20];

    printf("Pill location = (%d, %d)\n", pill->pill_half_1->location.x, pill->pill_half_1->location.y);

    // // Adjust pill positions differently for each move
    // switch(direction) {
    //     // Shifts
    //     case MOVE_DOWN:
    //         // Check if pill can move down
    //         // if (pill_canMoveDown(pill, board)) {
    //         if (true) {
    //             // Move both pill halves down a tile
    //             ph1_loc.y++;
    //             ph2_loc.y++;
    //             entity_move(board, pill->pill_half_1, ph1_loc);
    //             entity_move(board, pill->pill_half_2, ph2_loc);
    //             drMarioDisplay_drawPill(pill);
    //         }

    //         break;

    //     case MOVE_RIGHT:
    //         break;
    //     case MOVE_LEFT:
    //         break;

    //     // Rotations
    //     case ROTATE_CLOCKWISE:
    //         break;
    //     case ROTATE_COUNTERCLOCKWISE:
    //         break;
    // }
}

void pill_test(pill_t *pill) {
    printf("Pill location = (%d, %d)\n", pill->pill_half_1->location.x, pill->pill_half_1->location.y);
}