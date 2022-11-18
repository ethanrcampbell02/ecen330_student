#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "interrupts.h"
#include "intervalTimer.h"
#include "touchscreen.h"

#define ADC_SETTLE_PERIOD_SECONDS 0.05

// States defining the behavior of the higher-level touchscreen driver
enum touchscreen_st_t {
  waiting_st,      // Waiting for display to be touched
  adc_settling_st, // Wait 50 ms for the analog-to-digital converter to settle
  pressed_st       // Output the coordinates of the touched location
};
static enum touchscreen_st_t currentState;

// Period of the state machine
static double sm_period_seconds;
// Flag indicating touchscreen has been pressed and the ADC has settled
static bool pressed;
// Number of ticks that have occured while waiting for ADC to settle
static uint32_t adc_timer;
// Number of ticks required for 50 ms to pass and the ADC to settle
static uint32_t adc_settle_ticks;

// Coordinates of the most recent touched point
static uint16_t touch_x, touch_y;
// Amount of pressure used form the most recent touched point
static uint8_t touch_z;

// Declare helper function
static void debugStatePrint();

// Initialize the touchscreen driver state machine, with a given tick period (in
// seconds).
void touchscreen_init(double period_seconds) {
  // Initialize current state as waiting for stimulus
  currentState = waiting_st;

  // Initialize state machine variables
  sm_period_seconds = period_seconds;
  pressed = false;
  adc_timer = 0;
  adc_settle_ticks = ceil(ADC_SETTLE_PERIOD_SECONDS / period_seconds);
}

// State machine error messages are defined here:
#define TRANSITION_ERR_MSG                                                     \
  "ERROR: No state transitions specified for currentState\n"
#define ACTION_ERR_MSG "ERROR: No state action specified for currentState\n"
// State machine tick function for the touchscreen
void touchscreen_tick() {
  // Perform state update
  switch (currentState) {
  case waiting_st:
    // If the display has been touched, move to adc_settling_st
    // and clear any old touchscreen data
    if (display_isTouched()) {
      currentState = adc_settling_st;
      display_clearOldTouchData();
    }
    // Otherwise, continue waiting in this state
    else {
      currentState = waiting_st;
    }
    break;

  case adc_settling_st:
    // If at any point the display stops being touched, return to waiting_st
    if (!display_isTouched()) {
      currentState = waiting_st;
    }
    // If more than 50 ms have passed, continue to pressed_st and get the
    // coordinates of the touched location
    else if (display_isTouched() && adc_timer == adc_settle_ticks) {
      currentState = pressed_st;
      display_getTouchedPoint(&touch_x, &touch_y, &touch_z);

    }
    // Otherwise, continue waiting for ADC to settle in this state
    else {
      currentState = adc_settling_st;
    }
    break;

  case pressed_st:
    // If the touchscreen stops being touched, return to waiting state
    if (!display_isTouched()) {
      currentState = waiting_st;
    }
    // Otherwise, stay in this state
    else {
      currentState = pressed_st;
    }
    break;

  // State transitions not given for current state
  default:
    printf(TRANSITION_ERR_MSG);
  }

  // Perform state actions
  switch (currentState) {
  // Set the ADC settling tick count to 0
  case waiting_st:
    adc_timer = 0;
    break;

  // Increment the ADC settling tick count
  case adc_settling_st:
    adc_timer++;
    break;

  // Raise the "pressed" flag
  case pressed_st:
    pressed = true;
    break;

  // State actions not given for current state
  default:
    printf(ACTION_ERR_MSG);
  }
}

// Return the current status of the touchscreen
touchscreen_status_t touchscreen_get_status() {
  // Return TOUCHSCREEN_IDLE if the "pressed" flag is not raised
  if (!pressed) {
    return TOUCHSCREEN_IDLE;
  }

  // Return TOUCHSCREEN_PRESSED if the "pressed" flag is being raised
  // and the touchscreen has not been released (still in pressed_st)
  if (pressed && currentState == pressed_st) {
    return TOUCHSCREEN_PRESSED;
  }

  // Return TOUCHSCREEN_RELEASED if the "pressed" flag is being raised
  // but the touchscreen has been released and the state machine has
  // returned to the idle state, indicating and unacknowledged press
  if (pressed && currentState != pressed_st) {
    return TOUCHSCREEN_RELEASED;
  }
}

// Acknowledge the touchscreen touch.  This function will only have effect when
// the touchscreen is in the TOUCHSCREEN_RELEASED status, and will cause it to
// switch to the TOUCHSCREEN_IDLE status.
void touchscreen_ack_touch() {
  // If there is an unacknowledged press, lower the pressed flag
  if (touchscreen_get_status() == TOUCHSCREEN_RELEASED) {
    pressed = false;
  }
}

// Get the (x,y) location of the last touchscreen touch
display_point_t touchscreen_get_location() {
  display_point_t touchedCoords = {touch_x, touch_y};
  return touchedCoords;
}

/***********HELPER FUNCTIONS************/

// All printed messages for states are provided here
#define WAITING_ST_MSG "waiting_st\n"
#define ADC_SETTLING_ST_MSG "adc_settling_st\n"
#define PRESSED_ST_MSG "pressed_st\n"
// This is a debug state print routine. It will print the names of the states
// each time tick() is called. It only prints states if they are different than
// the previous state.
static void debugStatePrint() {
  static enum touchscreen_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This is the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state
  // name over and over
  if (previousState != currentState || firstPass) {
    firstPass = false; // previousState will be defined, firstPass is false
    previousState =
        currentState;       // keep track of the last state that you were in
    switch (currentState) { // This prints messages based upon the state that
                            // you were in
    case waiting_st:
      printf(WAITING_ST_MSG);
      break;
    case adc_settling_st:
      printf(ADC_SETTLING_ST_MSG);
      break;
    case pressed_st:
      printf(PRESSED_ST_MSG);
      break;
    }
  }
}