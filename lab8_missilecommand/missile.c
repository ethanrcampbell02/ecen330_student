#include "missile.h"
#include "config.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "display.h"

// States defining the missile's behavior
enum missile_st_t {
  flying_st,
  explosion_grow_st,
  explosion_shrink_st,
  dead_st
};

//////////////// Helper Functions ////////////////

// General missile member initialization that is independent
// of the missile's type. Origin and destination of missile
// must already be defined for calculations to be correct.
void missile_init_general(missile_t *missile) {
  // Calculate the total length of the missile's trajectory
  double x_len = (float)(missile->x_dest - missile->x_origin);
  double y_len = (float)(missile->y_dest - missile->y_origin);
  missile->total_length = (uint16_t)sqrt(pow(x_len, 2) + pow(y_len, 2));

  // Initialize other variables
  missile->length = 0.0;
  missile->explode_me = false;
  missile->x_current = missile->x_origin;
  missile->y_current = missile->y_origin;
  missile->impacted = false;
}

// Unlike most state machines that have a single `init` function, our missile
// will have different initializers depending on the missile type.

// Initialize the missile as a dead missile.  This is useful at the start of the
// game to ensure that player and plane missiles aren't moving before they
// should.
void missile_init_dead(missile_t *missile) {
  // Only set the current state, member variables are set later
  missile->currentState = dead_st;
}

// Initialize the missile as an enemy missile.  This will randomly choose the
// origin and destination of the missile.  The origin should be somewhere near
// the top of the screen, and the destination should be the very bottom of the
// screen.

// Lowest possible location an enemy missile can spawn in
#define ENEMY_ORIGIN_BOTTOM_CUTOFF (DISPLAY_HEIGHT / 4)

void missile_init_enemy(missile_t *missile) {
  missile->type = MISSILE_TYPE_ENEMY;
  missile->currentState = flying_st;

  // Randomly choose origin point in the top quarter of the display
  missile->x_origin = rand() % DISPLAY_WIDTH;
  missile->y_origin = rand() % ENEMY_ORIGIN_BOTTOM_CUTOFF;

  // Randomly choose destination point on the bottom edge of the display
  missile->x_dest = rand() % DISPLAY_WIDTH;
  missile->y_dest = DISPLAY_HEIGHT;

  // Initialize other struct members
  missile_init_general(missile);
}

// Initialize the missile as a player missile.  This function takes an (x, y)
// destination of the missile (where the user touched on the touchscreen).  The
// origin should be the closest "firing location" to the destination (there are
// three firing locations evenly spaced along the bottom of the screen).

#define NUM_START_POINTS 3
#define PLAYER_START_1_X (DISPLAY_WIDTH / 4)
#define PLAYER_START_2_X (2 * (DISPLAY_WIDTH / 4))
#define PLAYER_START_3_X (3 * (DISPLAY_WIDTH / 4))

void missile_init_player(missile_t *missile, uint16_t x_dest, uint16_t y_dest) {
  missile->type = MISSILE_TYPE_PLAYER;
  missile->currentState = flying_st;

  // Determine missile origin point as the closest to the touched location
  // of three possible player missile sources on the bottom edge of the screen
  missile->y_origin = DISPLAY_HEIGHT;

  uint16_t lowest_dist = UINT16_MAX;
  uint16_t start_points[] = {PLAYER_START_1_X, PLAYER_START_2_X,
                             PLAYER_START_3_X};

  uint16_t curr_dist = 0;
  // Iterate through each possible starting location and keep track of closest
  for (uint8_t i = 0; i < NUM_START_POINTS; i++) {
    curr_dist = abs(x_dest - start_points[i]);

    // If current distance is less than the previously calculated lowest
    // distance, update the lowest distance and x_origin
    if (curr_dist < lowest_dist) {
      lowest_dist = curr_dist;
      missile->x_origin = start_points[i];
    }
  }

  // Destination is given based on the touched location
  missile->x_dest = x_dest;
  missile->y_dest = y_dest;

  // Initialize other struct members
  missile_init_general(missile);
}

// Initialize the missile as a plane missile.  This function takes an (x, y)
// location of the plane which will be used as the origin.  The destination can
// be randomly chosed along the bottom of the screen.
void missile_init_plane(missile_t *missile, int16_t plane_x, int16_t plane_y) {
  missile->type = MISSILE_TYPE_PLANE;
  missile->currentState = flying_st;

  // Origin point is given as the position of the plane
  missile->x_origin = plane_x;
  missile->y_origin = plane_y;

  // Randomly choose destination point on the bottom edge of the display
  missile->x_dest = rand() % DISPLAY_WIDTH;
  missile->y_dest = DISPLAY_HEIGHT;

  // Initialize other struct members
  missile_init_general(missile);
}

////////// State Machine TICK Function //////////

// Plane missile speeds were not defined in config
#define PLANE_MISSILE_DISTANCE_PER_SECOND 50
#define PLANE_MISSILE_DISTANCE_PER_TICK                                        \
  (PLANE_MISSILE_DISTANCE_PER_SECOND * CONFIG_GAME_TIMER_PERIOD)

// Max radius of explosions
#define MAX_EXPLOSION_RADIUS 25

// Background color of game screen
#define BACKGROUND_COLOR DISPLAY_BLACK

// Missile speed is doubled because tick rate is halved
#define TICK_RATE_FACTOR 2

// Define state machine behavior of the missile
void missile_tick(missile_t *missile) {
  uint16_t missile_color;
  double missile_distance_per_tick;
  double percentage_traveled;

  // Determine missile color and speed from its type
  if (missile->type == MISSILE_TYPE_PLAYER) {
    missile_color = CONFIG_COLOR_PLAYER;
    missile_distance_per_tick = CONFIG_PLAYER_MISSILE_DISTANCE_PER_TICK;
  } else if (missile->type == MISSILE_TYPE_ENEMY) {
    missile_color = CONFIG_COLOR_ENEMY;
    missile_distance_per_tick = CONFIG_ENEMY_MISSILE_DISTANCE_PER_TICK;
  } else if (missile->type == MISSILE_TYPE_PLANE) {
    missile_color = CONFIG_COLOR_PLANE;
    missile_distance_per_tick = PLANE_MISSILE_DISTANCE_PER_TICK;
  }

  // State transition control
  switch (missile->currentState) {
  case flying_st:
    // Erase current missile path
    display_drawLine(missile->x_origin, missile->y_origin, missile->x_current,
                     missile->y_current, BACKGROUND_COLOR);

    // Update missile's position
    missile->length += (missile_distance_per_tick * TICK_RATE_FACTOR);
    percentage_traveled = missile->length / missile->total_length;
    missile->x_current =
        missile->x_origin +
        percentage_traveled * (missile->x_dest - missile->x_origin);
    missile->y_current =
        missile->y_origin +
        percentage_traveled * (missile->y_dest - missile->y_origin);

    // If missile's explosion flag is set, transition to the growing explosion
    // state
    if (missile->explode_me) {
      missile->currentState = explosion_grow_st;
    }
    // If a player missile has reached its destination, blow it up
    else if (missile->type == MISSILE_TYPE_PLAYER &&
             missile->length >= missile->total_length) {
      missile->currentState = explosion_grow_st;
    }
    // If another missile has reached its destination, kill it without exploding
    else if (missile->type != MISSILE_TYPE_PLAYER &&
             missile->length >= missile->total_length) {
      missile->impacted = true;
      missile->currentState = dead_st;
    }
    // Otherwise, redraw the missile with the updated position and continue the
    // missile's flight
    else {
      display_drawLine(missile->x_origin, missile->y_origin, missile->x_current,
                       missile->y_current, missile_color);
      missile->currentState = flying_st;
    }
    break;

  case explosion_grow_st:
    // Increment radius
    missile->radius +=
        CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK * TICK_RATE_FACTOR;

    // If explosion has reached max radius, start shrinking the explosion
    if (missile->radius >= MAX_EXPLOSION_RADIUS) {
      missile->currentState = explosion_shrink_st;
    }
    // Otherwise, continue drawing larger circles
    else {
      display_fillCircle(missile->x_current, missile->y_current,
                         (int16_t)missile->radius, missile_color);
      missile->currentState = explosion_grow_st;
    }
    break;

  case explosion_shrink_st:
    // Erase current explosion circle
    display_fillCircle(missile->x_current, missile->y_current,
                       (int16_t)missile->radius, BACKGROUND_COLOR);

    // Decrement radius
    missile->radius -=
        CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK * TICK_RATE_FACTOR;

    // If explosion has fizzled out, the missile is dead
    if (missile->radius <= 0.0) {
      missile->currentState = dead_st;
    }
    // Otherwise, continue drawing smaller circles
    else {
      display_fillCircle(missile->x_current, missile->y_current,
                         (int16_t)missile->radius, missile_color);
      missile->currentState = explosion_shrink_st;
    }
    break;
  case dead_st:
    // Just chill here until missile is re-initialized
    break;
  }

  // State action control (Moore actions are kinda cringe tbh)
  switch (missile->currentState) {
  case flying_st:
    break;
  case explosion_grow_st:
    break;
  case explosion_shrink_st:
    break;
  case dead_st:
    break;
  }
}

// Return whether the given missile is dead.
bool missile_is_dead(missile_t *missile) {
  return (missile->currentState == dead_st);
}

// Return whether the given missile is exploding.  This is needed when detecting
// whether a missile hits another exploding missile.
bool missile_is_exploding(missile_t *missile) {
  enum missile_st_t state = missile->currentState;
  return (state == explosion_grow_st || state == explosion_shrink_st);
}

// Return whether the given missile is flying.
bool missile_is_flying(missile_t *missile) {
  return (missile->currentState == flying_st);
}

// Used to indicate that a flying missile should be detonated.  This occurs when
// an enemy or plane missile is located within an explosion zone.
void missile_trigger_explosion(missile_t *missile) {
  missile->explode_me = true;
}