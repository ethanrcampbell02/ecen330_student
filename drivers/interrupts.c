#include "interrupts.h"
#include "armInterrupts.h"
#include "intervalTimer.h"
#include "xil_io.h"
#include "xparameters.h"

// Defining offsets for the relevant interrupt controller addresses
#define IPR_BASE_OFFSET 0x04 // Interrupt Pending Register
#define IER_BASE_OFFSET 0x08 // Interrupt Enable Register
#define IAR_BASE_OFFSET 0x0C // Interrupt Acknowledge Register
#define SIE_BASE_OFFSET 0x10 // Set Interrupt Enables
#define CIE_BASE_OFFSET 0x14 // Clear Interrupt Enables
#define MER_BASE_OFFSET 0x1C // Master Interrupt Register

// Master Interrupt Register bit masks
#define MER_ME_MASK 0x1  // Master IRQ Enable
#define MER_HIE_MASK 0x2 // Hardware Interrupt Enable

// Global array of user-provided interrupt service routines
#define NUM_INTR_INPUTS XPAR_AXI_INTC_0_NUM_INTR_INPUTS
static void (*isrFcnPtrs[NUM_INTR_INPUTS])() = {NULL};

// Declare some helper functions
static uint32_t readRegister(uint32_t offset);
static void writeRegister(uint32_t offset, uint32_t value);
static void interrupts_isr();

/***********HEADER FUNCTIONS************/

// Initializes interrupt hardware:
// 1. Configures AXI INTC registers to:
//  - Enable interrupt output
//  - Disable all interrupt input lines.
// 2. Enables the Interrupt system on the ARM processor, and
//    registers an ISR handler function.
//  - armInterrupts_init()
//  - armInterrupts_setupIntc(isr_fcn_ptr)
//  - armInterrupts_enable()
void interrupts_init() {
  // Configure the Master Interrupt Register
  //  - Enable Irq output signal
  //  - Enable hardware interrupts / disable software interrupts
  uint32_t MER_config_bits = 0x0000 | MER_ME_MASK | MER_HIE_MASK;
  writeRegister(MER_BASE_OFFSET, MER_config_bits);

  // Setup interrupts for the ARM processor
  armInterrupts_init();
  armInterrupts_setupIntc(interrupts_isr);
  armInterrupts_enable();

  // Disable all interrupt inputs (just in case)
  uint32_t disable_interrupt_inputs = 0x0000;
  writeRegister(IER_BASE_OFFSET, disable_interrupt_inputs);
}

// Registers a callback function (fcn is a function pointer to this callback
// function) for a given interrupt input number (irq).  When this interrupt
// input is active, fcn will be called.
void interrupts_register(uint8_t irq, void (*fcn)()) {
  // Add function pointer to the array of ISR pointers
  isrFcnPtrs[irq] = fcn;
}

// Enables single input interrupt line, given by irq number.
void interrupts_irq_enable(uint8_t irq) {
  // Enable the specified interrupt input
  writeRegister(SIE_BASE_OFFSET, 1 << irq);
}

// Disable single input interrupt line, given by irq number.
void interrupts_irq_disable(uint8_t irq) {
  // Enable the specified interrupt input
  writeRegister(CIE_BASE_OFFSET, 1 << irq);
}

/***********HELPER FUNCTIONS************/

// Reads from the interrupt controller's base address
// + the given address offset
static uint32_t readRegister(uint32_t offset) {
  return Xil_In32(XPAR_AXI_INTC_0_BASEADDR + offset);
}

// Writes the given values to the interrupts controller's
// base address + the given address offset
static void writeRegister(uint32_t offset, uint32_t value) {
  Xil_Out32(XPAR_AXI_INTC_0_BASEADDR + offset, value);
}

// Checks for interrupts and calls appropriate interrupt
// service routines given by the user
static void interrupts_isr() {
  // Loop through each interrupt input and check if
  // there is an interrupt pending
  for (uint32_t i = 0; i < NUM_INTR_INPUTS; i++) {
    // Read IPR to check if there is a pending interrupt
    // for the current iqr number
    bool pendingIntr = readRegister(IPR_BASE_OFFSET) & (1 << i);

    // If there is a pending interrupt and a function exists
    // for that interrupt input, execute the function
    if (pendingIntr && isrFcnPtrs[i]) {
      isrFcnPtrs[i]();
    }

    // Acknowledge the interrupt to the controller
    writeRegister(IAR_BASE_OFFSET, (1 << i));
  }
}