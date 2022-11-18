#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "clockControl.h"
#include "clockDisplay.h"
#include "touchscreen.h"

// Time the button must be held before entering fast update mode
#define LONG_PRESS_DELAY_SECONDS 0.5
// Period between increments/decrements during fast update mode
#define FAST_UPDATE_PERIOD_SECONDS 0.1

/*************SM VARIABLES**************/

// States defining the clock controller's behavior
enum clockControl_st_t {
  waiting_st,
  inc_dec_st,
  long_press_delay_st,
  fast_update_st
};
static enum clockControl_st_t currentState;

// Number of ticks spent waiting for a 0.5 sec long press
static uint16_t delay_cnt;
// Number of ticks required for 0.5 sec to pass
static uint16_t delay_num_ticks;
// Number of ticks spent in fast update mode before updating
static uint16_t update_cnt;
// Number of ticks required for 0.1 sec to pass
static uint16_t update_num_ticks;

/***********HELPER FUNCTIONS************/

// All printed messages for states are given here:
#define WAITING_ST_MSG "waiting_st\n"
#define INC_DEC_ST_MSG "inc_dec_st\n"
#define LONG_PRESS_DELAY_ST_MSG "long_press_delay_st\n"
#define FAST_UPDATE_ST_MSG "fast_update_st\n"
// This is a debug state print routine. It will print the names of the states
// each time tick() is called. It only prints states if they are different than
// the previous state.
static void debugStatePrint() {
  static enum clockControl_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This is the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state
  // name over and over
  if (previousState != currentState || firstPass) {
    firstPass = false; // previousState will be defined, firstPass is false
    previousState = currentState;
    // keep track of the last state that you were in
    switch (currentState) { // This prints messages based upon the state that
                            // you were in
    case waiting_st:
      printf(WAITING_ST_MSG);
      break;
    case inc_dec_st:
      printf(INC_DEC_ST_MSG);
      break;
    case long_press_delay_st:
      printf(LONG_PRESS_DELAY_ST_MSG);
      break;
    case fast_update_st:
      printf(FAST_UPDATE_ST_MSG);
      break;
    }
  }
}

/***********HEADER FUNCTIONS************/

// Initializes the clock control state machine, with a given period in seconds.
void clockControl_init(double period_s) {
  // Initialize current state as waiting for a press
  currentState = waiting_st;

  // Initialize other state machine variables
  delay_cnt = 0;
  delay_num_ticks = ceil(LONG_PRESS_DELAY_SECONDS / period_s);
  update_cnt = 0;
  update_num_ticks = ceil(FAST_UPDATE_PERIOD_SECONDS / period_s);
}

// State machine error messages are defined here:
#define TRANSITION_ERR_MSG "ERROR: No state transitions specified for state\n"
#define ACTION_ERR_MSG "ERROR: No state action specified for state\n"
// Ticks the clock control state machine
void clockControl_tick() {
  // Debug state printing
  debugStatePrint();

  // Perform state update
  switch (currentState) {
  case waiting_st:
    // If the display is (for some reason) still being released, move back to
    // inc_dec_st.
    if (touchscreen_get_status() == TOUCHSCREEN_RELEASED) {
      currentState = inc_dec_st;
    }
    // If the display is pressed, move to long_press_delay_st and clear the
    // delay_cnt
    else if (touchscreen_get_status() == TOUCHSCREEN_PRESSED) {
      currentState = long_press_delay_st;
      delay_cnt = 0;
    }
    // Otherwise, stay in this state
    else {
      currentState = waiting_st;
    }
    break;

  case inc_dec_st:
    // If the touchscreen is released, return to waiting state and acknowledge
    // the touch
    if (touchscreen_get_status() == TOUCHSCREEN_RELEASED) {
      currentState = waiting_st;
      touchscreen_ack_touch();
    }
    // Otherwise, stay in this state
    else {
      currentState = inc_dec_st;
    }
    break;

  case long_press_delay_st:
    // If touchscreen is released, enter inc_dec state and only update once
    if (touchscreen_get_status() == TOUCHSCREEN_RELEASED) {
      currentState = inc_dec_st;
    }
    // If touchscreen is held more than 0.5 sec, enter fast update mode and
    // clear update_cnt
    else if (delay_cnt == delay_num_ticks) {
      currentState = fast_update_st;
      update_cnt = 0;
    }
    // Otherwise, increment the count and continue waiting for 0.5 sec to pass
    else {
      currentState = long_press_delay_st;
      delay_cnt++;
    }
    break;

  case fast_update_st:
    // If touchscreen is released, return to waiting state and acknowledge touch
    if (touchscreen_get_status() == TOUCHSCREEN_RELEASED) {
      currentState = waiting_st;
    }
    // If more than 0.1 sec have passed, reset counter and incr or decr the time
    else if ((touchscreen_get_status() != TOUCHSCREEN_RELEASED) &&
             (update_cnt == update_num_ticks)) {
      currentState = fast_update_st;
      update_cnt = 0;
      clockDisplay_performIncDec(touchscreen_get_location());
    }
    // Otherwise, continue waiting for 0.1 sec to pass
    else {
      currentState = fast_update_st;
    }
    break;

  // State transitions not given for current state
  default:
    printf(TRANSITION_ERR_MSG);
    break;
  }

  // Perform state action
  switch (currentState) {
  // Do nothing
  case waiting_st:
    break;

  // Increment/decrement the timer
  case inc_dec_st:
    clockDisplay_performIncDec(touchscreen_get_location());
    break;

  // Do nothing
  case long_press_delay_st:
    break;

  // Increment update count
  case fast_update_st:
    update_cnt++;
    break;

  // State action not provided for current state
  default:
    printf(ACTION_ERR_MSG);
    break;
  }
}