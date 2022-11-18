#include "buttons.h"
#include "display.h"
#include "leds.h"
#include "switches.h"

// Constants used to check if all switches or buttons are on
#define ALL_BUTTONS_PRESSED 0x0F
#define ALL_SWITCHES_ON 0x0F

// Defining positions on the display
#define DISPLAY_TOP 0

// Defining parameters for the four rectangles (RECT0 corresponds to button 0)
#define RECT0_X (DISPLAY_WIDTH * 3 / 4)
#define RECT1_X (DISPLAY_WIDTH / 2)
#define RECT2_X (DISPLAY_WIDTH / 4)
#define RECT3_X 0

#define RECT_WIDTH (DISPLAY_WIDTH / 4)
#define RECT_HEIGHT (DISPLAY_HEIGHT / 2)

// Defining parameters for the display text
#define CURSOR_RECT_OFFSET 15 // X offset from the left edge of the rectangle
#define CURSOR_Y (RECT_HEIGHT / 2 - 5)
#define TEXT_SIZE 2

// Runs a test of the buttons. As you push the buttons, graphics and messages
// will be written to the LCD panel. The test will until all 4 pushbuttons are
// simultaneously pressed.
void gpioTest_buttons() {
  // Initialize switch values to be seen as off
  uint8_t buttons = 0x00;

  // Initialize button and display drivers so we can read and write
  display_init();
  buttons_init();

  // Initialize some display properties
  display_fillScreen(DISPLAY_BLACK); // Blank the display
  display_setTextSize(TEXT_SIZE);    // Set text size

  // Previous state of the buttons
  uint8_t buttons_prev = 0x00;

  // Print a rectangle to the display corresponding to each button being
  // pressed until all buttons are being pressed at the same time
  while (buttons != ALL_BUTTONS_PRESSED) {
    buttons = buttons_read();

    // RECTANGLE FOR BUTTON 0:
    // Print a white rectangle if button 0 has been pressed
    if ((buttons & BUTTONS_BTN0_MASK) && !(buttons_prev & BUTTONS_BTN0_MASK)) {
      display_fillRect(RECT0_X, DISPLAY_TOP, RECT_WIDTH, RECT_HEIGHT,
                       DISPLAY_WHITE);

      // Write button name on top of the rectangle
      display_setTextColor(DISPLAY_BLACK);
      display_setCursor(RECT0_X + CURSOR_RECT_OFFSET, CURSOR_Y);
      display_println("BTN0");
    }
    // Erase the rectangle if button 0 has been released
    if (!(buttons & BUTTONS_BTN0_MASK) && (buttons_prev & BUTTONS_BTN0_MASK)) {
      display_fillRect(RECT0_X, DISPLAY_TOP, RECT_WIDTH, RECT_HEIGHT,
                       DISPLAY_BLACK);
    }

    // RECTANGLE FOR BUTTON 1:
    // Print a cyan rectangle if button 1 has been pressed
    if ((buttons & BUTTONS_BTN1_MASK) && !(buttons_prev & BUTTONS_BTN1_MASK)) {
      display_fillRect(RECT1_X, DISPLAY_TOP, RECT_WIDTH, RECT_HEIGHT,
                       DISPLAY_CYAN);

      // Write button name on top of the rectangle
      display_setTextColor(DISPLAY_BLACK);
      display_setCursor(RECT1_X + CURSOR_RECT_OFFSET, CURSOR_Y);
      display_println("BTN1");
    }
    // Erase the rectangle if button 1 has been released
    if (!(buttons & BUTTONS_BTN1_MASK) && (buttons_prev & BUTTONS_BTN1_MASK)) {
      display_fillRect(RECT1_X, DISPLAY_TOP, RECT_WIDTH, RECT_HEIGHT,
                       DISPLAY_BLACK);
    }

    // RECTANGLE FOR BUTTON 2:
    // Print a red rectangle if button 2 has been pressed
    if ((buttons & BUTTONS_BTN2_MASK) && !(buttons_prev & BUTTONS_BTN2_MASK)) {
      display_fillRect(RECT2_X, DISPLAY_TOP, RECT_WIDTH, RECT_HEIGHT,
                       DISPLAY_RED);

      // Write button name on top of the rectangle
      display_setTextColor(DISPLAY_WHITE);
      display_setCursor(RECT2_X + CURSOR_RECT_OFFSET, CURSOR_Y);
      display_println("BTN2");
    }
    // Erase the rectangle if button 2 has been released
    if (!(buttons & BUTTONS_BTN2_MASK) && (buttons_prev & BUTTONS_BTN2_MASK)) {
      display_fillRect(RECT2_X, DISPLAY_TOP, RECT_WIDTH, RECT_HEIGHT,
                       DISPLAY_BLACK);
    }

    // RECTANGLE FOR BUTTON 3:
    // Print a blue rectangle if button 3 has been pressed
    if ((buttons & BUTTONS_BTN3_MASK) && !(buttons_prev & BUTTONS_BTN3_MASK)) {
      display_fillRect(RECT3_X, DISPLAY_TOP, RECT_WIDTH, RECT_HEIGHT,
                       DISPLAY_BLUE);

      // Write button name on top of the rectangle
      display_setTextColor(DISPLAY_WHITE);
      display_setCursor(RECT3_X + CURSOR_RECT_OFFSET, CURSOR_Y);
      display_println("BTN3");
    }
    // Erase the rectangle if button 3 has been released
    if (!(buttons & BUTTONS_BTN3_MASK) && (buttons_prev & BUTTONS_BTN3_MASK)) {
      display_fillRect(RECT3_X, DISPLAY_TOP, RECT_WIDTH, RECT_HEIGHT,
                       DISPLAY_BLACK);
    }

    buttons_prev = buttons;
  }

  // Clear the screen once all buttons have been pressed
  display_fillScreen(DISPLAY_BLACK);
}

// Runs a test of the switches. As you slide the switches, LEDs directly above
// the switches will illuminate. The test will run until all switches are slid
// upwards. When all 4 slide switches are slid upward, this function will
// return.
void gpioTest_switches() {
  // Initialize switch values to be seen as off
  uint8_t switches = 0x00;

  // Initialize switch and LED drivers so we can read and write
  switches_init();
  leds_init();

  // Turn on each switch's corresponding LED until all switches are on at the
  // same time
  while (switches != ALL_SWITCHES_ON) {
    switches = switches_read();
    leds_write(switches);
  }

  // Turn off the LEDs after all switches have turned on
  leds_write(0x00);
}