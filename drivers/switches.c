#include "switches.h"
#include "xil_io.h"
#include "xparameters.h"

#include <stdio.h>

// Defining offsets from base address for the switch device drivers
#define SWITCHES_DATA_ADDR_OFFSET 0x00
#define SWITCHES_TRI_ADDR_OFFSET 0x04

// Configure each switch as an input by deactivating tri-state drivers
#define SWITCHES_TRI_DISABLE 0x0F

// Declare some helper functions
static uint32_t readRegister(uint32_t offset);
static void writeRegister(uint32_t offset, uint32_t value);

/***********HEADER FUNCTIONS************/

// Sets tri-state driver registers to correct state
// so the switches' values can be read from
void switches_init() {
  writeRegister(SWITCHES_TRI_ADDR_OFFSET, SWITCHES_TRI_DISABLE);
}

// Returns the state of the 4 switches in one packaged integer
uint8_t switches_read() { return readRegister(SWITCHES_DATA_ADDR_OFFSET); }

/***********HELPER FUNCTIONS************/

// Reads from the switches' device base register address
// + the given address offset
static uint32_t readRegister(uint32_t offset) {
  return Xil_In32(XPAR_SLIDE_SWITCHES_BASEADDR + offset);
}

// Writes the given values to the switches' device base
// register address + the given address offset
static void writeRegister(uint32_t offset, uint32_t value) {
  Xil_Out32(XPAR_SLIDE_SWITCHES_BASEADDR + offset, value);
}