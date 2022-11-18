#include "plane.h"
#include "config.h"
#include "display.h"
#include "missile.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define PLANE_HALF_WIDTH 10
#define PLANE_HALF_LENGTH 20

#define NUM_DIRECTIONS 2
#define DISPLAY_HALF_WIDTH (DISPLAY_WIDTH / 2)
#define DISPLAY_QUARTER_WIDTH (DISPLAY_WIDTH / 4)
#define DISPLAY_TOP_QUARTER (DISPLAY_HEIGHT / 3)
#define STARTING_OFFSET_X -20
#define ENDING_OFFSET_X 20

#define MAX_COOLDOWN_SECS 10
#define MIN_COOLDOWN_SECS 5

////////// State Machine Variables //////////

// States defining the planes behavior
static enum plane_st_t { flying_st, dead_st } currentState;

static int16_t x_pos;     // Current X position of the plane
static int16_t y_pos;     // Current Y position of the plane
static bool facing_right; // Direction the plane is facing

static missile_t *missile;  // Pointer to the plane's missile
static uint16_t fire_x_pos; // X position that the plane should fire the missile
static bool missile_fired;  // Indicates whether the missile has been fired

static bool explode_me;           // Indicates when the plane should be exploded
static uint16_t dead_ticks_count; // Number of ticks the plane has been dead
static uint16_t dead_ticks_total; // Total number of ticks before respawn

////////// Helper Functions //////////

// Determins whether the plane has gone its full course and should
// be killed
static bool plane_isOffscreen() {
  // Boundary is determined by the direction of the plane
  if (facing_right && x_pos >= (DISPLAY_WIDTH + PLANE_HALF_LENGTH))
    return true;
  else if (!facing_right && x_pos <= 0)
    return true;
  else
    return false;
}

// Checks if the plane has met the conditions to fire a missile:
//  1. The plane is past the boundary point set at initialization.
//  2. The missile has not been fired yet.
static bool plane_canFireMissile() {
  // Check if the missile has been fired
  if (missile_fired) {
    return false;
  }

  // Boundary state is determined by the direction of the plane
  if (facing_right && x_pos >= fire_x_pos)
    return true;
  else if (!facing_right && x_pos <= fire_x_pos)
    return true;
  else
    return false;
}

// Draws (or erases) the plane at the current position
static void plane_drawPlane(bool erase) {
  int16_t vertex_x1, vertex_y1;
  int16_t vertex_x2, vertex_y2;
  int16_t vertex_x3, vertex_y3;

  // Plane is normally white, color in black if erasing
  uint16_t color = erase ? DISPLAY_BLACK : DISPLAY_WHITE;

  // Plane's position is the center of the triangle
  // * Mirror the plane depending on its movement direction
  if (facing_right) {
    // First point
    vertex_x1 = x_pos + PLANE_HALF_LENGTH;
    vertex_y1 = y_pos;

    // Second Point
    vertex_x2 = x_pos - PLANE_HALF_LENGTH;
    vertex_y2 = y_pos + PLANE_HALF_WIDTH;

    // Third Point
    vertex_x3 = x_pos - PLANE_HALF_LENGTH;
    vertex_y3 = y_pos - PLANE_HALF_WIDTH;
  } else {
    // First point
    vertex_x1 = x_pos - PLANE_HALF_LENGTH;
    vertex_y1 = y_pos;

    // Second Point
    vertex_x2 = x_pos + PLANE_HALF_LENGTH;
    vertex_y2 = y_pos - PLANE_HALF_WIDTH;

    // Third Point
    vertex_x3 = x_pos + PLANE_HALF_LENGTH;
    vertex_y3 = y_pos + PLANE_HALF_WIDTH;
  }

  // Draw the plane sprite as a triangle
  display_fillTriangle(vertex_x1, vertex_y1, vertex_x2, vertex_y2, vertex_x3,
                       vertex_y3, color);
}

// All printed messages for states are given here:
#define FLYING_ST_MSG "flying_st\n"
#define DEAD_ST_MSG "dead_st\n"

// This is a debug state print routine. It will print the names of the states
// each time tick() is called. It only prints states if they are different than
// the previous state.
static void debugStatePrint() {
  static enum plane_st_t previousState;
  static bool firstPass = true;

  // Only print the message if:
  // 1. This is the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state
  // name over and over
  if (previousState != currentState || firstPass) {
    firstPass = false; // previousState will be defined, firstPass is false
    previousState = currentState;

    // Print the current state if it changed since the last pass
    switch (currentState) {
    case flying_st:
      printf(FLYING_ST_MSG);
      break;
    case dead_st:
      printf(DEAD_ST_MSG);
      break;
    }
  }
}

////////// Header Functions //////////

// Initialize the plane state machine
// Pass in a pointer to the missile struct (the plane will only have one
// missile)
void plane_init(missile_t *plane_missile) {
  // Randomize the cooldown timer before spawn
  uint16_t dead_time_total_secs =
      rand() % (MAX_COOLDOWN_SECS - MIN_COOLDOWN_SECS) + MIN_COOLDOWN_SECS;
  dead_ticks_total = dead_time_total_secs / CONFIG_GAME_TIMER_PERIOD;

  // Randomize the direction the plane is facing
  facing_right = rand() % NUM_DIRECTIONS;

  // Randomize the firing location in the middle half of the screen
  fire_x_pos = rand() % DISPLAY_HALF_WIDTH + DISPLAY_QUARTER_WIDTH;

  // Initialize starting position based on direction
  x_pos =
      facing_right ? STARTING_OFFSET_X : (DISPLAY_WIDTH - STARTING_OFFSET_X);
  y_pos = DISPLAY_TOP_QUARTER;

  // Initialize the rest of the variables
  missile_fired = false;
  explode_me = false;
  missile = plane_missile;
  dead_ticks_count = 0;

  currentState = dead_st;
}

// State machine tick function
void plane_tick() {
  // debugStatePrint();

  // State transition control
  switch (currentState) {
  case flying_st:
    // If the plane is either past the edge or the explosion flag
    // has been raised, erase the missile and re-initialze the plane
    // to the dead state
    if (plane_isOffscreen() || explode_me) {
      plane_drawPlane(true);
      plane_init(missile);
      currentState = dead_st;
    }

    // If the plane is past the firing point and has not fired the
    // missile, fire the missile and continue flying
    else if (plane_canFireMissile()) {
      missile_init_plane(missile, x_pos, y_pos);
      missile_fired = true;
      currentState = flying_st;
    }

    // Otherwise, continue flying
    else {
      currentState = flying_st;
    }

    break;

  case dead_st:
    // If the cooldown has passed, move to flying state
    if (dead_ticks_count >= dead_ticks_total) {
      currentState = flying_st;
    }

    // Otherwise, continue waiting for the cooldown to end
    else {
      currentState = dead_st;
    }
    break;
  }

  // State action control
  switch (currentState) {
  // Erase the plane, increment the position, and draw the plane
  case flying_st:
    plane_drawPlane(true);

    // Move left or right depending on the faced direction
    if (facing_right)
      x_pos += CONFIG_PLANE_DISTANCE_PER_TICK;
    else
      x_pos -= CONFIG_PLANE_DISTANCE_PER_TICK;

    plane_drawPlane(false);
    break;

  // Increment the cooldown counter
  case dead_st:
    dead_ticks_count++;
    break;
  }
}

// Trigger the plane to expode
void plane_explode() { explode_me = true; }

// Get the XY location of the plane
display_point_t plane_getXY() {
  display_point_t plane_pos = {x_pos, y_pos};
  return plane_pos;
}
