#ifndef BOARD_H
#define BOARD_H

#include "display.h"
#include "config.h"
#include <stdint.h>

// Defining the colors of entities
#define ENTITY_COLOR_EMPTY DISPLAY_BLACK
#define ENTITY_COLOR_RED DISPLAY_RED
#define ENTITY_COLOR_BLUE DISPLAY_CYAN
#define ENTITY_COLOR_YELLOW DISPLAY_YELLOW
#define ENTITY_NUM_COLORS 3

// Defining three types of objects that will occupy tiles
typedef enum {
    ENTITY_EMPTY,
    ENTITY_PILL_HALF,
    ENTITY_VIRUS
} entity_type_t;

// Struct for keeping track of board positions
typedef struct location_t {
    uint8_t x;
    uint8_t y;
} location_t;

// Defining entity parameters
typedef struct entity_t {
    entity_type_t type;
    location_t location;
    uint16_t color;
} entity_t;

// Struct containing the current board state
typedef struct {
    entity_t *tiles[CONFIG_BOARD_WIDTH_TILES][CONFIG_BOARD_HEIGHT_TILES];
} board_t;

// Initializes the board with empty tiles
void board_initEmpty(board_t *board);

// Initializes the board with the specified number of viruses
void board_initViruses(board_t *board, uint8_t numViruses);

// Creates an empty tile at this location
void entity_initEmpty(board_t *board, location_t location);

// Creates a virus at this location with the given color
void entity_initVirus(board_t *board, location_t location, uint16_t color);

// Randomizes the given entity's color
void entity_randomizeColor(entity_t *entity);

// Moves the entity to the given location
void entity_move(board_t *board, entity_t *entity, location_t location);

#endif /* BOARD_H */