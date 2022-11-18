#include "gameControl.h"
#include "config.h"
#include "missile.h"
#include "plane.h"

#include "touchscreen.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Index of the first missile in the second half of the missiles array
#define MISSILES_HALFWAY_INDEX (CONFIG_MAX_TOTAL_MISSILES / 2)

// Define array for all missiles and sub-arrays for different types
static missile_t missiles[CONFIG_MAX_TOTAL_MISSILES];
static missile_t *enemy_missiles = &(missiles[0]);
static missile_t *player_missiles = &(missiles[CONFIG_MAX_ENEMY_MISSILES]);
static missile_t *plane_missile = &(missiles[CONFIG_MAX_TOTAL_MISSILES - 1]);

// Keep track of statistics
static uint8_t player_shots;
static uint8_t enemy_impacts;
static uint8_t prev_player_shots;
static uint8_t prev_enemy_impacts;

// Other global variables
static display_point_t touchedLocation; // Most recent touched location
static bool tickFirstHalf; // Flag telling to tick even or odd missiles
static bool firstTick;

// Checks if the given point is within the exploding radius of the
// given exploding missile
static bool point_isInExplosion(uint16_t pX, uint16_t pY,
                                missile_t *expMissile) {
  // Calculate distance (squared) from the exploding missile
  int16_t delta_x = pX - expMissile->x_current;
  int16_t delta_y = pY - expMissile->y_current;
  float distFromExplosionSqr = (float)(delta_x * delta_x + delta_y * delta_y);

  // Get square of the explosion radius
  float radiusSqr = expMissile->radius * expMissile->radius;

  // If missile is within explosion radius and is flying, trigger
  // explosion
  if (distFromExplosionSqr <= radiusSqr)
    return true;
  else
    return false;
}

#define SHOTS_CURSOR_X 10
#define SHOTS_CURSOR_Y 10
#define IMPACTS_CURSOR_X (DISPLAY_WIDTH / 2)
#define IMPACTS_CURSOR_Y 10
#define STATISTICS_TEXT_SIZE 1
#define MAX_STR_LEN 15

// Erases and re-prints the statistics at the top of the screen
void printStatistics() {
  char shotsStr[MAX_STR_LEN];
  char impactsStr[MAX_STR_LEN];

  // ERASE TEXT FROM PREVIOUS TICK //

  // Populate strings with the text to be printed
  sprintf(shotsStr, "Shots: %d", prev_player_shots);
  sprintf(impactsStr, "Impacts: %d", prev_enemy_impacts);

  // Print the number of shots the player has fired
  display_setTextColor(DISPLAY_BLACK);
  display_setCursor(SHOTS_CURSOR_X, SHOTS_CURSOR_Y);
  display_print(shotsStr);

  // Print the number of impacted enemy/plane missiles
  display_setCursor(IMPACTS_CURSOR_X, IMPACTS_CURSOR_Y);
  display_print(impactsStr);

  // PRINT NEW TEXT WITH UPDATED STATISTICS //

  // Populate strings with the text to be printed
  sprintf(shotsStr, "Shots: %d", player_shots);
  sprintf(impactsStr, "Impacts: %d", enemy_impacts);

  // Print the number of shots the player has fired
  display_setTextColor(DISPLAY_WHITE);
  display_setCursor(SHOTS_CURSOR_X, SHOTS_CURSOR_Y);
  display_print(shotsStr);

  // Print the number of impacted enemy/plane missiles
  display_setCursor(IMPACTS_CURSOR_X, IMPACTS_CURSOR_Y);
  display_print(impactsStr);
}

// Initialize the game control logic
// This function will initialize all missiles, stats, plane, etc.
void gameControl_init() {
  // Initialize black background
  display_fillScreen(DISPLAY_BLACK);

  // Initialize all missiles as dead
  for (uint16_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++)
    missile_init_dead(&missiles[i]);

  // Initialize the plane
  plane_init(plane_missile);

  // Initialize other global variables
  touchedLocation.x = touchedLocation.y = 0;
  tickFirstHalf = true;
  firstTick = true;
  player_shots = 0;
  enemy_impacts = 0;
  prev_player_shots = 0;
  prev_enemy_impacts = 0;
}

// Tick the game control logic
//
// This function should tick the missiles, handle screen touches, collisions,
// and updating statistics.
void gameControl_tick() {
  // Keep track of statistics from previous tick
  prev_player_shots = player_shots;
  prev_enemy_impacts = enemy_impacts;

  // Iterate through all enemy missiles
  for (uint16_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++) {
    // Re-initialize any dead enemy missiles
    if (missile_is_dead(&enemy_missiles[i])) {
      missile_init_enemy(&enemy_missiles[i]);
    }
  }

  // // Update touched location if the touchscreen is touched
  // if (touchscreen_get_status() == TOUCHSCREEN_PRESSED) {
  //   touchedLocation = touchscreen_get_location();
  // }

  // Re-initialize one dead player missile if the screen is released
  if (touchscreen_get_status() == TOUCHSCREEN_RELEASED) {
    touchedLocation = touchscreen_get_location();
    touchscreen_ack_touch();

    // Iterate through player missiles
    for (uint16_t i = 0; i < CONFIG_MAX_PLAYER_MISSILES; i++) {
      // Re-initialize the first dead player missile found
      if (missile_is_dead(&player_missiles[i])) {
        missile_init_player(&player_missiles[i], touchedLocation.x,
                            touchedLocation.y);
        player_shots++;
        break;
      }
    }
  }

  missile_t *explodingMissile = NULL;
  missile_t *currMissile = NULL;
  bool missile_isInExplosion;

  // Iterate through all exploding missiles to check if other missiles
  // explosions' must be triggered
  for (uint16_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++) {
    // Check if the missile is exploding
    if (missile_is_exploding(&missiles[i])) {
      explodingMissile = &missiles[i];

      // Iterate through all enemy missiles
      for (uint16_t j = 0; j < CONFIG_MAX_ENEMY_MISSILES; j++) {
        currMissile = &enemy_missiles[j];
        missile_isInExplosion = point_isInExplosion(
            currMissile->x_current, currMissile->y_current, explodingMissile);

        // Check if the missile is within the explosion radius of the exploding
        // missile
        if (missile_isInExplosion && missile_is_flying(currMissile)) {
          missile_trigger_explosion(currMissile);
        }
      }

      missile_isInExplosion = point_isInExplosion(
          plane_missile->x_current, plane_missile->y_current, explodingMissile);

      // Check the plane missile
      if (missile_isInExplosion && missile_is_flying(plane_missile)) {
        missile_trigger_explosion(plane_missile);
      }

      display_point_t plane_loc = plane_getXY();
      bool plane_isInExplosion =
          point_isInExplosion(plane_loc.x, plane_loc.y, explodingMissile);

      // Also check the plane
      if (plane_isInExplosion) {
        plane_explode();
      }
    }
  }

  // Tick half of the missiles (either first or last half)
  if (tickFirstHalf) {
    // Tick the first half of the missiles array
    for (uint16_t i = 0; i < MISSILES_HALFWAY_INDEX; i++) {
      missile_tick(&missiles[i]);
    }
  } else {
    // Tick the last half of the missiles array
    for (uint16_t i = MISSILES_HALFWAY_INDEX; i < CONFIG_MAX_TOTAL_MISSILES;
         i++) {
      missile_tick(&missiles[i]);
    }
  }

  // Tick other half of the missiles next cycle
  tickFirstHalf = !tickFirstHalf;

  // Tick the plane
  plane_tick();

  // Check for any impacted enemy missiles
  for (uint16_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++) {
    currMissile = &enemy_missiles[i];
    if (currMissile->impacted) {
      enemy_impacts++;
    }
  }

  // Check for an impacted plane missile
  if (plane_missile->impacted) {
    enemy_impacts++;
    // Don't want to read that it is impacted next tick
    plane_missile->impacted = false;
  }

  // If statistics have changed, update the printed statistics
  if (prev_enemy_impacts != enemy_impacts ||
      prev_player_shots != player_shots || firstTick) {
    printStatistics();
  }

  firstTick = false;
}
