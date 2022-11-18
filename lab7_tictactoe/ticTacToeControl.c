#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "minimax.h"
#include "ticTacToe.h"
#include "ticTacToeControl.h"
#include "ticTacToeDisplay.h"

#include "buttons.h"
#include "display.h"
#include "touchscreen.h"

// Duration to show instructions before the game begins
#define INSTRUCTIONS_PERIOD_SECONDS 5

// Duration the computer waits for the player to input a move
// at the beginning of the game before making its move instead
#define INITIAL_WAIT_PERIOD_SECONDS 5

// Define parameters for printing instructions to the screen
#define INSTRUCTIONS_CURSOR_X 0
#define INSTRUCTIONS_CURSOR_Y 75
#define INSTRUCTIONS_TEXT_SIZE 2

// Mask for button 0 on the board
#define BTN0_MASK 0x01

static void displayInstructionsScreen();
static void eraseInstructions();
static void playInLocation(tictactoe_location_t location);
static bool locationIsOccupied(tictactoe_location_t location);
static void clearBoard();
static void debugStatePrint();

/*************SM VARIABLES**************/

// States defining the clock controller's behavior
enum clockControl_st_t {
  instructions_st,
  game_begin_st,
  display_touch_st,
  computer_turn_st,
  player_turn_st,
  game_end_st
};
static enum clockControl_st_t currentState;

// State machine global variables
static tictactoe_board_t board;     // Current state of the board
static bool is_Xs_turn;             // Keep track of whose turn it is
static uint16_t instructionsCycles; // Num cycles to display instructions
static uint16_t
    instructionsCyclesCnt;         // Count cycles in display_instructions_st
static uint16_t initWaitCycles;    // Num cycles to wait for player to go first
static uint16_t initWaitCyclesCnt; // Count cycles in game_start_st
static tictactoe_location_t
    touchedLocation; // Most recent valid touched location

/***********HEADER FUNCTIONS************/

// Initialize the tic-tac-toe controller state machine,
// providing the tick period, in seconds.
void ticTacToeControl_init(double period_s) {
  // Initialize button and display drivers
  buttons_init();
  display_init();

  // Initialize state machine variables
  minimax_initBoard(&board);
  is_Xs_turn = true;
  instructionsCycles = ceil(INSTRUCTIONS_PERIOD_SECONDS / period_s);
  instructionsCyclesCnt = 0;
  initWaitCycles = ceil(INITIAL_WAIT_PERIOD_SECONDS / period_s);
  initWaitCyclesCnt = 0;
  touchedLocation.row = touchedLocation.column = 0;

  // Enter initial state where instructions will be displayed for
  // 5 seconds before the game begins
  currentState = instructions_st;

  // Display instructions
  displayInstructionsScreen();
}

// State machine error messages are defined here:
#define TRANSITION_ERR_MSG "ERROR: No state transitions specified for state\n"
#define ACTION_ERR_MSG "ERROR: No state action specified for state\n"
// Tick the tic-tac-toe controller state machine
void ticTacToeControl_tick() {
  // Debug state printing
  // debugStatePrint();

  // State transition control
  switch (currentState) {
  // Show instructions for 5 seconds
  case instructions_st:
    // If the equivalent number of SM periods have passed for
    // 5 seconds to pass, begin the game and reset the count
    if (instructionsCyclesCnt >= instructionsCycles) {
      currentState = game_begin_st;
      initWaitCyclesCnt = 0;
      eraseInstructions();
      ticTacToeDisplay_init();
    }
    // Otherwise, continue waiting
    else {
      currentState = instructions_st;
    }
    break;

  // Determine turn order
  case game_begin_st:
    // If the equivalent number of SM periods have passed for
    // 5 seconds to pass without the player starting, the
    // computer will instead move first as X
    if (initWaitCyclesCnt >= initWaitCycles) {
      initWaitCyclesCnt = 0;

      // Computer starts in top-left corner
      tictactoe_location_t firstMove = {0, 0};
      playInLocation(firstMove);

      // Transition to player's turn
      currentState = player_turn_st;
    }

    // Otherwise, check if the screen has been touched. If it
    // has been, move to the state where the touchscreen is
    // being held on the player's turn
    else if (touchscreen_get_status() == TOUCHSCREEN_PRESSED) {
      initWaitCyclesCnt = 0;
      touchedLocation =
          ticTacToeDisplay_getLocationFromPoint(touchscreen_get_location());
      currentState = display_touch_st;
    }

    // Continue waiting for either player input or for the computer
    // to get impatient and move first
    else {
      currentState = game_begin_st;
    }
    break;

  // It is the player's turn and the display has been touched
  case display_touch_st:
    // If location is unoccupied and touch is released, play there
    if (touchscreen_get_status() == TOUCHSCREEN_RELEASED &&
        !locationIsOccupied(touchedLocation)) {
      touchscreen_ack_touch();
      playInLocation(touchedLocation);
      currentState = computer_turn_st;
    }

    // If location is occupied and touch is released, return to the
    // player's turn state to wait for another input
    else if (touchscreen_get_status() == TOUCHSCREEN_RELEASED &&
             locationIsOccupied(touchedLocation)) {
      touchscreen_ack_touch();
      currentState = player_turn_st;
    }

    // If touchscreen still held, wait for a release
    else if (touchscreen_get_status() == TOUCHSCREEN_PRESSED) {
      currentState = display_touch_st;
    }
    break;

  // It is the computer's turn
  case computer_turn_st:
    // If the game is over, move to the game end state
    if (minimax_isGameOver(minimax_computeBoardScore(&board, is_Xs_turn))) {
      currentState = game_end_st;
    }

    // Otherwise, compute next move and play there
    else {
      playInLocation(minimax_computeNextMove(&board, is_Xs_turn));
      currentState = player_turn_st;
    }
    break;

  case player_turn_st:
    // If the game is over, move to the game end state
    if (minimax_isGameOver(minimax_computeBoardScore(&board, is_Xs_turn))) {
      currentState = game_end_st;
    }

    // Otherwise, check if the display has been toucned
    else if (touchscreen_get_status() == TOUCHSCREEN_PRESSED) {
      touchedLocation =
          ticTacToeDisplay_getLocationFromPoint(touchscreen_get_location());
      currentState = display_touch_st;
    }

    // Continue waiting for the display to be touched
    else {
      currentState = player_turn_st;
    }
    break;

  case game_end_st:
    // If the button is pressed, clear the board and restart the game
    if (buttons_read() && BTN0_MASK) {
      clearBoard();
      is_Xs_turn = true;
      currentState = game_begin_st;
    }

    // Otherwise, continue waiting
    else {
      currentState = game_end_st;
    }
    break;

  // State transitions not given for current state
  default:
    printf(TRANSITION_ERR_MSG);
    break;
  }

  // State action control
  switch (currentState) {

  // Increment the count of the number of cycles that the
  // instructions have been displayed for
  case instructions_st:
    instructionsCyclesCnt++;
    break;

  // Increment the count of the number of cycles that the computer
  // has spent waiting for the player to start
  case game_begin_st:
    initWaitCyclesCnt++;
    break;

  case display_touch_st:
    break;

  case computer_turn_st:
    break;

  case player_turn_st:
    break;

  case game_end_st:
    break;

  // State actions not given for current state
  default:
    printf(ACTION_ERR_MSG);
  }
}

/***********HELPER FUNCTIONS************/

// Displays the instructions screen that will show for
// 5 seconds upon program start.
static void displayInstructionsScreen() {
  // Initialize screen to black
  display_fillScreen(DISPLAY_DARK_BLUE);

  // Set parameters for the cursor
  display_setTextSize(INSTRUCTIONS_TEXT_SIZE);
  display_setCursor(INSTRUCTIONS_CURSOR_X, INSTRUCTIONS_CURSOR_Y);

  // Print the instructions for the game
  display_print("   Touch board to play X\n");
  display_print("          -or-\n");
  display_print("   wait for the computer\n");
  display_print("        and play O.");
}

// Draws the instructions text in black, effectively erasing it
static void eraseInstructions() {
  // Set parameters for the cursor
  display_setTextColor(DISPLAY_DARK_BLUE);
  display_setTextSize(INSTRUCTIONS_TEXT_SIZE);
  display_setCursor(INSTRUCTIONS_CURSOR_X, INSTRUCTIONS_CURSOR_Y);

  // Print the instructions for the game
  display_print("   Touch board to play X\n");
  display_print("          -or-\n");
  display_print("   wait for the computer\n");
  display_print("        and play O.");
}

// Plays in the given location on the tic-tac-toe board, both in the data
// structure and the displayed board
static void playInLocation(tictactoe_location_t location) {
  // Place a token depending on whose turn it is
  if (is_Xs_turn) {
    board.squares[location.row][location.column] = MINIMAX_X_SQUARE;
    ticTacToeDisplay_drawX(location, false);
  } else {
    board.squares[location.row][location.column] = MINIMAX_O_SQUARE;
    ticTacToeDisplay_drawO(location, false);
  }

  // Transition to opponent's turn
  is_Xs_turn = !is_Xs_turn;
}

// Checks if the given location is occupied
static bool locationIsOccupied(tictactoe_location_t location) {
  return board.squares[location.row][location.column] != MINIMAX_EMPTY_SQUARE;
}

// Clears the board, both in the data structure and on the display
static void clearBoard() {
  tictactoe_location_t currPos;

  // Iterate through each row of the board
  for (uint8_t rowPos = 0; rowPos < TICTACTOE_BOARD_ROWS; rowPos++) {
    currPos.row = rowPos;
    // Iterate through each column in that row
    for (uint8_t colPos = 0; colPos < TICTACTOE_BOARD_COLUMNS; colPos++) {
      currPos.column = colPos;

      // Erase the current square on the display if it is occupied
      if (board.squares[rowPos][colPos] == MINIMAX_X_SQUARE) {
        ticTacToeDisplay_drawX(currPos, true);
      } else if (board.squares[rowPos][colPos] == MINIMAX_O_SQUARE) {
        ticTacToeDisplay_drawO(currPos, true);
      }

      // Clear the current square in the data structure
      board.squares[rowPos][colPos] = MINIMAX_EMPTY_SQUARE;
    }
  }
}

// All printed messages for states are given here:
#define INSTRUCTIONS_ST_MSG "instructions_st\n"
#define GAME_BEGIN_ST_MSG "game_begin_st\n"
#define DISPLAY_TOUCH_ST_MSG "display_touch_st\n"
#define COMPUTER_TURN_ST_MSG "computer_turn_st\n"
#define PLAYER_TURN_ST_MSG "player_turn_st\n"
#define GAME_END_ST_MSG "game_end_st\n"

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

    // Print the current state if it changed since the last pass
    switch (currentState) {
    case instructions_st:
      printf(INSTRUCTIONS_ST_MSG);
      break;
    case game_begin_st:
      printf(GAME_BEGIN_ST_MSG);
      break;
    case display_touch_st:
      printf(DISPLAY_TOUCH_ST_MSG);
      break;
    case computer_turn_st:
      printf(COMPUTER_TURN_ST_MSG);
      break;
    case player_turn_st:
      printf(PLAYER_TURN_ST_MSG);
      break;
    case game_end_st:
      printf(GAME_END_ST_MSG);
      break;
    }
  }
}