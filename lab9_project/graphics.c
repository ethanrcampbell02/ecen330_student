#include "display.h"
#include "config.h"
#include "board.h"
#include "pill.h"
#include <stdbool.h>
#include <stdio.h>

// Initialize display and draw the board
void drMarioDisplay_init() {
    int16_t lineStartX = CONFIG_BOARD_TOP_LEFT_X;
    int16_t lineStartY = CONFIG_BOARD_TOP_LEFT_Y;
    int16_t board_width = CONFIG_BOARD_WIDTH_TILES * CONFIG_TILE_WIDTH;
    int16_t board_height = CONFIG_BOARD_HEIGHT_TILES * CONFIG_TILE_HEIGHT;

    // Initialize screen and fill in the background
    display_init();
    display_fillScreen(CONFIG_BACKGROUND_COLOR);
    
    // Draw horizontal lines to define the tiles
    for (int i = 0; i <= CONFIG_BOARD_HEIGHT_TILES; i++) {
        display_drawFastHLine(lineStartX, lineStartY, board_width, CONFIG_BOARD_LINE_COLOR);
        lineStartY += CONFIG_TILE_HEIGHT;
    }

    lineStartX = CONFIG_BOARD_TOP_LEFT_X;
    lineStartY = CONFIG_BOARD_TOP_LEFT_Y;

    // Draw vertical lines to define the tiles
    for (int i = 0; i <= CONFIG_BOARD_WIDTH_TILES; i++) {
        display_drawFastVLine(lineStartX, lineStartY, board_height+1, CONFIG_BOARD_LINE_COLOR);
        lineStartX += CONFIG_TILE_WIDTH;
    }
}

// Fills the tile at a location with the given color
void fillTile(location_t location, uint16_t color) {
    int16_t cornerX = CONFIG_BOARD_TOP_LEFT_X + location.x * CONFIG_TILE_WIDTH + 1;
    int16_t cornerY = CONFIG_BOARD_TOP_LEFT_Y + location.y * CONFIG_TILE_HEIGHT + 1;
    int16_t width = CONFIG_TILE_WIDTH - 1;
    int16_t height = CONFIG_TILE_HEIGHT - 1;

    display_fillRect(cornerX, cornerY, width, height, color);
}

// Erases the current tile
void drMarioDisplay_clearTile(location_t location) {
    fillTile(location, CONFIG_BACKGROUND_COLOR);
}

// Draws the given virus
void drMarioDisplay_drawVirus(entity_t *virus) {
    int16_t cornerX = CONFIG_BOARD_TOP_LEFT_X + virus->location.x * CONFIG_TILE_WIDTH + 1;
    int16_t cornerY = CONFIG_BOARD_TOP_LEFT_Y + virus->location.y * CONFIG_TILE_HEIGHT + 1;
    uint16_t width = CONFIG_TILE_WIDTH - 1;
    uint16_t height = CONFIG_TILE_HEIGHT - 1;
    uint16_t color = CONFIG_BACKGROUND_COLOR;

    // Virus sprite bitmap
    const uint8_t virus_bitmap[] = {
        0xc0, 0x30, 0x80, 0x10, 0x00, 0x00, 0x09, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 
        0x10, 0x80, 0x00, 0x00, 0x80, 0x10, 0xc0, 0x30,
    };

    fillTile(virus->location, virus->color);
    display_drawBitmap(cornerX, cornerY, virus_bitmap, width, height, color);
}

// Draws the given pill
void drMarioDisplay_drawPill(pill_t *pill) {
    uint16_t cornerX = CONFIG_BOARD_TOP_LEFT_X + pill->pill_half_1->location.x * CONFIG_TILE_WIDTH + 1;
    uint16_t cornerY = CONFIG_BOARD_TOP_LEFT_Y + pill->pill_half_1->location.y * CONFIG_TILE_HEIGHT + 1;
    uint16_t width;
    uint16_t height;
    uint16_t color = CONFIG_BACKGROUND_COLOR;

    // Horizontal pill bitmap
    const uint8_t pillHorizontal_bitmap[] = {
        0xc0, 0x00, 0x01, 0x80, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0xc0, 0x00, 0x01, 0x80
    };

    // Vertical pill bitmap
    const uint8_t pillVertical_bitmap[] = {
        0xc0, 0x30, 0x80, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x10, 
        0xc0, 0x30
    };

    // Half pill bitmap
    const uint8_t pillSingle_bitmap[] = {
        0xc0, 0x30, 0x80, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x80, 0x10, 0xc0, 0x30, 
    };

    // Draw differently based on the orientation
    switch (pill->orientation) {
        case ORIENTATION_EMPTY:
            printf("WARNING: Trying to print an empty pill");
            break;
        case ORIENTATION_HORIZONTAL:
            // Fill in the tiles
            fillTile(pill->pill_half_1->location, pill->pill_half_1->color);
            fillTile(pill->pill_half_2->location, pill->pill_half_2->color);

            width = CONFIG_TILE_WIDTH * 2 - 1;
            height = CONFIG_TILE_HEIGHT - 1;

            // Draw in the overlay to round out the corners
            display_drawBitmap(cornerX, cornerY, pillHorizontal_bitmap, width, height, color);

            break;
        case ORIENTATION_VERTICAL:
            fillTile(pill->pill_half_1->location, pill->pill_half_1->color);
            fillTile(pill->pill_half_2->location, pill->pill_half_2->color);

            width = CONFIG_TILE_WIDTH - 1;
            height = CONFIG_TILE_HEIGHT * 2 - 1;

            // Draw in the overlay to round out the corners
            display_drawBitmap(cornerX, cornerY, pillVertical_bitmap, width, height, color);

            break;
        case ORIENTATION_SINGLE:
            fillTile(pill->pill_half_1->location, pill->pill_half_1->color);

            width = CONFIG_TILE_WIDTH - 1;
            height = CONFIG_TILE_HEIGHT - 1;

            // Draw in the overlay to round out the corners
            display_drawBitmap(cornerX, cornerY, pillSingle_bitmap, width, height, color);

            break;
    }
}