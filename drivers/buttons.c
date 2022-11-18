#include "buttons.h"
#include "xil_io.h"
#include "xparameters.h"

// Defining offsets from base address for the button device drivers
#define BUTTONS_DATA_ADDR_OFFSET 0x00
#define BUTTONS_TRI_ADDR_OFFSET 0x04

// Configure each button as an input by deactivating tri-state drivers
#define BUTTONS_TRI_DISABLE 0x0F

// Declare some helper functions
static uint32_t readRegister(uint32_t offset);
static void writeRegister(uint32_t offset, uint32_t value);

/***********HEADER FUNCTIONS************/

// Sets tri-state driver registers to correct state
// so the buttons' values can be read from
void buttons_init() {
  writeRegister(BUTTONS_TRI_ADDR_OFFSET, BUTTONS_TRI_DISABLE);
}

// Returns the state of the 4 buttons in one packaged integer
uint8_t buttons_read() { return readRegister(BUTTONS_DATA_ADDR_OFFSET); }

/***********HELPER FUNCTIONS************/

// Reads from the buttons' device base register address
// + the given address offset
static uint32_t readRegister(uint32_t offset) {
  return Xil_In32(XPAR_PUSH_BUTTONS_BASEADDR + offset);
}

// Writes the given values to the buttons' device base
// register address + the given address offset
static void writeRegister(uint32_t offset, uint32_t value) {
  Xil_Out32(XPAR_PUSH_BUTTONS_BASEADDR + offset, value);
}