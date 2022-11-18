#include "intervalTimer.h"
#include "xil_io.h"
#include "xparameters.h"

// Defining offsets from the base address for the different timer registers
#define TCSR0_ADDR_OFFSET 0x00
#define TLR0_ADDR_OFFSET 0x04
#define TCR0_ADDR_OFFSET 0x08
#define TCSR1_ADDR_OFFSET 0x10
#define TLR1_ADDR_OFFSET 0x14
#define TCR1_ADDR_OFFSET 0x18

// Masks for the configuration bits in the status registers
#define UDT0_MASK 0x002
#define ARHT0_MASK 0x010
#define LOAD0_MASK 0x020
#define LOAD1_MASK 0x020
#define ENIT0_MASK 0x040
#define ENT0_MASK 0x080
#define T0INT_MASK 0x100
#define CASC_MASK 0x800

// Shift a 64-bit value by 32 bits to split / concatenate values
#define SHIFT_32_BITS 32U

// Declaring some helper functions
static uint32_t readRegister(uint32_t timerNumber, uint32_t offset);
static void writeRegister(uint32_t timerNumber, uint32_t offset,
                          uint32_t value);
static uint64_t convSecondsToCycles(uint32_t timerNumber, double period);

/***********HEADER FUNCTIONS************/

// Configures the interval timer before use:
// 1. Sets the Timer Control/Status Registers such that:
//  - The timer is in 64-bit cascade mode
//  - The timer counts up
// 2. Initializes both LOAD registers with zeros
// 3. Calls the _reload function to move the LOAD values into the Counters
void intervalTimer_initCountUp(uint32_t timerNumber) {
  // Set timers in cascade mode
  uint32_t configData0 = 0x00000000 | CASC_MASK;
  uint32_t configData1 = 0x00000000;

  // Write the timer configuration data to the corresponding TCSR registers
  writeRegister(timerNumber, TCSR0_ADDR_OFFSET, configData0);
  writeRegister(timerNumber, TCSR1_ADDR_OFFSET, configData1);
}

// Configures the interval timer before use:
// 1. Sets the Timer Control/Status Registers such that:
//  - The timer is in 64-bit cascade mode
//  - The timer counts down
//  - The timer automatically reloads when reaching zero
// 2. Initializes LOAD registers with appropriate values, given the `period`.
// 3. Calls the _reload function to move the LOAD values into the Counters
void intervalTimer_initCountDown(uint32_t timerNumber, double period) {
  // Set timers in cascade, down-count, and auto-reload modes
  uint32_t configData0 = 0x00000000 | CASC_MASK | UDT0_MASK | ARHT0_MASK;
  uint32_t configData1 = 0x00000000;

  // Write the timer configuration data to the corresponding TCSR registers
  writeRegister(timerNumber, TCSR0_ADDR_OFFSET, configData0);
  writeRegister(timerNumber, TCSR1_ADDR_OFFSET, configData1);

  // Convert the period to clock cycles and split into upper/lower 32 bits
  uint64_t cyclesInPeriod = convSecondsToCycles(timerNumber, period);
  uint32_t cyclesLower32 = cyclesInPeriod;
  uint32_t cyclesUpper32 = cyclesInPeriod >> SHIFT_32_BITS;

  // Write lower 32 bits to counter 0 and upper 32 bits to counter 1
  writeRegister(timerNumber, TLR0_ADDR_OFFSET, cyclesLower32);
  writeRegister(timerNumber, TLR1_ADDR_OFFSET, cyclesUpper32);

  // Load the counter with the initial values
  intervalTimer_reload(timerNumber);
}

// This function starts the interval timer running.
// If the interval timer is already running, this function does nothing.
// timerNumber indicates which timer should start running.
// Make sure to only change the Enable Timer bit of the register and not modify
// the other bits.
void intervalTimer_start(uint32_t timerNumber) {
  // Get current configuration data of timer 0's configuration register
  uint32_t configData = readRegister(timerNumber, TCSR0_ADDR_OFFSET);

  // Set the enable bit in the configuration data and write the register
  configData |= ENT0_MASK;
  writeRegister(timerNumber, TCSR0_ADDR_OFFSET, configData);
}

// This function stops a running interval timer.
// If the interval time is currently stopped, this function does nothing.
// timerNumber indicates which timer should stop running.
// Make sure to only change the Enable Timer bit of the register and not modify
// the other bits.
void intervalTimer_stop(uint32_t timerNumber) {
  // Get current configuration data of timer 0's configuration register
  uint32_t configData = readRegister(timerNumber, TCSR0_ADDR_OFFSET);

  // Clear the enable bit in the configuration data and write the register
  configData &= ~ENT0_MASK;
  writeRegister(timerNumber, TCSR0_ADDR_OFFSET, configData);
}

// Loads the counter register with the data in the load registers
void intervalTimer_reload(uint32_t timerNumber) {
  // Read current values stored in the configuration registers
  uint32_t oldConfigData0 = readRegister(timerNumber, TCSR0_ADDR_OFFSET);
  uint32_t oldConfigData1 = readRegister(timerNumber, TCSR1_ADDR_OFFSET);

  // Turn on the load bit in the configuration data
  uint32_t newConfigData0 = oldConfigData0 | LOAD0_MASK;
  uint32_t newConfigData1 = oldConfigData1 | LOAD1_MASK;

  // Pulse the load bit on and off in the configuration registers
  // to reload the generate value
  writeRegister(timerNumber, TCSR0_ADDR_OFFSET, newConfigData0);
  writeRegister(timerNumber, TCSR1_ADDR_OFFSET, newConfigData1);
  writeRegister(timerNumber, TCSR0_ADDR_OFFSET, oldConfigData0);
  writeRegister(timerNumber, TCSR1_ADDR_OFFSET, oldConfigData1);
}

// Converts the current time in the counter from clock cycles to seconds
double intervalTimer_getTotalDurationInSeconds(uint32_t timerNumber) {
  // Read the value stored in upper 32-bits counter register
  uint64_t prevCyclesCountUpper32 = readRegister(timerNumber, TCR1_ADDR_OFFSET);

  // Declare variables for the next reads of the count registers
  uint64_t cyclesCountLower32;
  uint64_t cyclesCountUpper32;

  // Continue reading the lower 32 bits and upper 32 bits until
  // the previous read of the upper 32 bits matches the current
  // read of the upper 32 bits. We do this to account for the
  // lower register possibly rolling over.
  do {
    cyclesCountLower32 = readRegister(timerNumber, TCR0_ADDR_OFFSET);
    cyclesCountUpper32 = readRegister(timerNumber, TCR1_ADDR_OFFSET);
  } while (prevCyclesCountUpper32 != cyclesCountUpper32);

  // Combine lower and upper bits into one 64-bit value
  uint64_t cyclesCount =
      (cyclesCountUpper32 << SHIFT_32_BITS) | cyclesCountLower32;

  uint64_t clockFrequency;
  // Get the correct timer's clock frequency based on the timer number
  if (timerNumber == XPAR_AXI_TIMER_0_DEVICE_ID) {
    clockFrequency = XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ;
  } else if (timerNumber == XPAR_AXI_TIMER_1_DEVICE_ID) {
    clockFrequency = XPAR_AXI_TIMER_1_CLOCK_FREQ_HZ;
  } else if (timerNumber == XPAR_AXI_TIMER_2_DEVICE_ID) {
    clockFrequency = XPAR_AXI_TIMER_2_CLOCK_FREQ_HZ;
  }

  // Divide the number of clock cycles by the clock frequency to get
  // the duration in seconds
  double durationSeconds = (double)cyclesCount / (double)clockFrequency;

  return durationSeconds;
}

// Enable the interrupt output of the given timer.
void intervalTimer_enableInterrupt(uint8_t timerNumber) {
  // Read current value stored in the configuration register
  uint32_t configData = readRegister(timerNumber, TCSR0_ADDR_OFFSET);

  // Set the interrupt enable bit in the configuration data
  configData |= ENIT0_MASK;

  // Write to the configuration register
  writeRegister(timerNumber, TCSR0_ADDR_OFFSET, configData);
}

// Disable the interrupt output of the given timer.
void intervalTimer_disableInterrupt(uint8_t timerNumber) {
  // Read current value stored in the configuration register
  uint32_t configData = readRegister(timerNumber, TCSR0_ADDR_OFFSET);

  // Clear the interrupt enable bit in the configuration data
  configData &= ~ENIT0_MASK;

  // Write to the configuration register
  writeRegister(timerNumber, TCSR0_ADDR_OFFSET, configData);
}

// Acknowledge the rollover to clear the interrupt output.
void intervalTimer_ackInterrupt(uint8_t timerNumber) {
  // Read current value stored in the configuration register
  uint32_t configData = readRegister(timerNumber, TCSR0_ADDR_OFFSET);

  // Set the Timer Interrupt bit, which counterintuitively clears itself
  configData |= T0INT_MASK;

  // Write to the configuration register
  writeRegister(timerNumber, TCSR0_ADDR_OFFSET, configData);
}

/***********HELPER FUNCTIONS************/

// Reads from the specified timer's device base register address
// + the given address offset
static uint32_t readRegister(uint32_t timerNumber, uint32_t offset) {
  uint32_t timerBaseAddr;

  // Get the correct timer's base address based on the timer number
  if (timerNumber == XPAR_AXI_TIMER_0_DEVICE_ID) {
    timerBaseAddr = XPAR_AXI_TIMER_0_BASEADDR;
  } else if (timerNumber == XPAR_AXI_TIMER_1_DEVICE_ID) {
    timerBaseAddr = XPAR_AXI_TIMER_1_BASEADDR;
  } else if (timerNumber == XPAR_AXI_TIMER_2_DEVICE_ID) {
    timerBaseAddr = XPAR_AXI_TIMER_2_BASEADDR;
  }

  return Xil_In32(timerBaseAddr + offset);
}

// Writes the given values to the specified timer's device base
// register address + the given address offset
static void writeRegister(uint32_t timerNumber, uint32_t offset,
                          uint32_t value) {
  uint32_t timerBaseAddr;

  // Get the correct timer's base address based on the timer number
  if (timerNumber == XPAR_AXI_TIMER_0_DEVICE_ID) {
    timerBaseAddr = XPAR_AXI_TIMER_0_BASEADDR;
  } else if (timerNumber == XPAR_AXI_TIMER_1_DEVICE_ID) {
    timerBaseAddr = XPAR_AXI_TIMER_1_BASEADDR;
  } else if (timerNumber == XPAR_AXI_TIMER_2_DEVICE_ID) {
    timerBaseAddr = XPAR_AXI_TIMER_2_BASEADDR;
  }

  Xil_Out32(timerBaseAddr + offset, value);
}

// Converts a period given in seconds to the equivalent number of
// clock cycles for the given timer's clock
static uint64_t convSecondsToCycles(uint32_t timerNumber, double period) {
  uint64_t clockFrequency;

  // Get the correct timer's clock frequency based on the timer number
  if (timerNumber == XPAR_AXI_TIMER_0_DEVICE_ID) {
    clockFrequency = XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ;
  } else if (timerNumber == XPAR_AXI_TIMER_1_DEVICE_ID) {
    clockFrequency = XPAR_AXI_TIMER_1_CLOCK_FREQ_HZ;
  } else if (timerNumber == XPAR_AXI_TIMER_2_DEVICE_ID) {
    clockFrequency = XPAR_AXI_TIMER_2_CLOCK_FREQ_HZ;
  }

  // Calculate and return the number of clock cycles in the given period
  // based off of the timer's clock frequency
  uint64_t numCycles = clockFrequency * period;
  return numCycles;
}