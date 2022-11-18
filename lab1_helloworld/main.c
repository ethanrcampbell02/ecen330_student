/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.

Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.

For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include <stdio.h>

#include "display.h"

// Defining positions on the display
#define DISPLAY_TOP 0
#define DISPLAY_BOTTOM (DISPLAY_TOP + DISPLAY_HEIGHT)
#define DISPLAY_LEFT 0
#define DISPLAY_RIGHT (DISPLAY_LEFT + DISPLAY_WIDTH)
#define DISPLAY_CENTER_X (DISPLAY_WIDTH / 2)
#define DISPLAY_CENTER_Y (DISPLAY_HEIGHT / 2)

// Defining circle parameters
#define CIRCLE_CENTER_OFFSET                                                   \
  (DISPLAY_WIDTH / 4) // Distance of the circle's center from screen edge
#define CIRCLE_RADIUS 25

// Defining triangle parameters
#define TRIANGLE_HEIGHT 50
#define TRIANGLE_WIDTH 50
#define TRIANGLE_BASE_OFFSET                                                   \
  (DISPLAY_HEIGHT / 5) // Distance of the triangle's base from screen edge
#define TRIANGLE_HALF_WIDTH (TRIANGLE_WIDTH / 2)

// Draw some shapes on the board's LCD screen
int main() {

  // Initialize display driver, and fill scren with black
  display_init();
  display_fillScreen(DISPLAY_BLACK); // Blank the screen.

  // Draw two green lines connecting opposite corners
  display_drawLine(DISPLAY_LEFT, DISPLAY_TOP, DISPLAY_RIGHT, DISPLAY_BOTTOM,
                   DISPLAY_GREEN);
  display_drawLine(DISPLAY_LEFT, DISPLAY_BOTTOM, DISPLAY_RIGHT, DISPLAY_TOP,
                   DISPLAY_GREEN);

  // Draw two circles on the right and left of the screen
  display_drawCircle(DISPLAY_LEFT + CIRCLE_CENTER_OFFSET, DISPLAY_CENTER_Y,
                     CIRCLE_RADIUS, DISPLAY_RED);
  display_fillCircle(DISPLAY_RIGHT - CIRCLE_CENTER_OFFSET, DISPLAY_CENTER_Y,
                     CIRCLE_RADIUS, DISPLAY_RED);

  // Draw two triangles on the top and bottom of the screen
  display_drawTriangle(DISPLAY_CENTER_X - TRIANGLE_HALF_WIDTH,
                       DISPLAY_BOTTOM - TRIANGLE_BASE_OFFSET,
                       DISPLAY_CENTER_X + TRIANGLE_HALF_WIDTH,
                       DISPLAY_BOTTOM - TRIANGLE_BASE_OFFSET, DISPLAY_CENTER_X,
                       DISPLAY_BOTTOM - TRIANGLE_BASE_OFFSET - TRIANGLE_HEIGHT,
                       DISPLAY_YELLOW);

  display_fillTriangle(DISPLAY_CENTER_X - TRIANGLE_HALF_WIDTH,
                       DISPLAY_TOP + TRIANGLE_BASE_OFFSET,
                       DISPLAY_CENTER_X + TRIANGLE_HALF_WIDTH,
                       DISPLAY_TOP + TRIANGLE_BASE_OFFSET, DISPLAY_CENTER_X,
                       DISPLAY_TOP + TRIANGLE_BASE_OFFSET + TRIANGLE_HEIGHT,
                       DISPLAY_YELLOW);

  return 0;
}
