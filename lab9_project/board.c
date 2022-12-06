#include "board.h"
#include "config.h"
#include "graphics.h"

#include <stdint.h>

// TODO: Figure out if I can get rid of this one
// // Initializes the board with empty tiles
// void board_initEmpty(board_t *board) {
//     location_t currLocation;

//     // Iterate through each row of the board
//     for (int col = 0; col < CONFIG_BOARD_WIDTH_TILES; col++) {
//         currLocation.x = col;

//         // Iterate through each column of the board
//         for (int row = 0; row < CONFIG_BOARD_WIDTH_TILES; row++) {
//             currLocation.y = row;
//             entity_initEmpty(board, currLocation);

//         }
//     }
// }

// Initializes the board with empty tiles
void board_initEmpty(board_t *board) {
    // Iterate through each row of the board
    for (int col = 0; col < CONFIG_BOARD_WIDTH_TILES; col++) {
        // Iterate through each column in this row
        for (int row = 0; row < CONFIG_BOARD_HEIGHT_TILES; row++) {
            board->tiles[col][row] = NULL;
        }
    }
}

// Initializes the board with the specified number of viruses
void board_initViruses(board_t *board, uint8_t numViruses) {
    uint16_t colors[ENTITY_NUM_COLORS] = {ENTITY_COLOR_RED, ENTITY_COLOR_BLUE, ENTITY_COLOR_YELLOW};
    uint16_t color;
    location_t location;

    // Start with an empty board
    board_initEmpty(board);

    // Iterate through each color
    for (int colorIndex = 0; colorIndex < ENTITY_NUM_COLORS; colorIndex++) {
        color = colors[colorIndex];

        // Generate viruses of this color in positions below the cutoff line
        for (int i = 0; i < CONFIG_NUM_VIRUSES / 3; i++) {
            // Randomize virus position
            location.x = rand() % CONFIG_BOARD_WIDTH_TILES;
            location.y = rand() % (CONFIG_BOARD_HEIGHT_TILES - CONFIG_VIRUS_TOP_CUTOFF) + CONFIG_VIRUS_TOP_CUTOFF;

            // Only place in this location if it is unoccupied
            if (board->tiles[location.x][location.y] == NULL)
                entity_initVirus(board, location, color);

            // Otherwise decrement the counter and place somewhere else
            else i--;
        }
    }
}

// TODO: might not use this function after all
// Creates an empty tile at this location
void entity_initEmpty(board_t *board, location_t location) {
    entity_t newEntity;
    newEntity.location = location;
    newEntity.type = ENTITY_EMPTY;
    newEntity.color = ENTITY_COLOR_EMPTY;
    board->tiles[location.x][location.y] = &newEntity;
}

// Creates a virus at this location with the given color
void entity_initVirus(board_t *board, location_t location, uint16_t color) {
    // Generate the virus
    entity_t newVirus;
    newVirus.location = location;
    newVirus.type = ENTITY_VIRUS;
    newVirus.color = color;

    // Place and draw the virus
    board->tiles[location.x][location.y] = &newVirus;
    drMarioDisplay_drawVirus(&newVirus);
}

// Randomizes the given entity's color
void entity_randomizeColor(entity_t *entity) {
    uint8_t colorCode = rand() % 3;

    // Choose the color from the randomized code
    switch (colorCode) {
        case 0: entity->color = ENTITY_COLOR_RED;
        case 1: entity->color = ENTITY_COLOR_BLUE;
        case 2: entity->color = ENTITY_COLOR_YELLOW;
    }
}

// Moves the entity to the given location
void entity_move(board_t *board, entity_t *entity, location_t location) {
    // Clear the entity's current position if within bounds
    if (entity->location.x < CONFIG_BOARD_WIDTH_TILES && entity->location.y < CONFIG_BOARD_HEIGHT_TILES) {
        board->tiles[entity->location.x][entity->location.y] = NULL;
        drMarioDisplay_clearTile(entity->location);  
    }

    // Move the entity
    entity->location = location;
    board->tiles[location.x][location.y] = entity;
    
    // Draw the entity if it is a virus (pills are handled elsewhere)
    if (entity->type == ENTITY_VIRUS)
        drMarioDisplay_drawVirus(entity);
}
