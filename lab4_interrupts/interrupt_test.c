#include "interrupt_test.h"
#include "interrupts.h"
#include "intervalTimer.h"
#include "leds.h"

// Defining constants for the timer numbers
#define TIMER_0 0U
#define TIMER_1 1U
#define TIMER_2 2U

// Definintg constants for the timer interrupts IRQ
#define TIMER_0_IRQ TIMER_0
#define TIMER_1_IRQ TIMER_1
#define TIMER_2_IRQ TIMER_2

// Defining the periods of the three timers (seconds)
#define TIMER_0_PERIOD 0.1
#define TIMER_1_PERIOD 1.0
#define TIMER_2_PERIOD 10.0

// Defining constants for the led bit masks
#define LED_0_MASK 0x1
#define LED_1_MASK 0x2
#define LED_2_MASK 0x4

// Declare some helper functions
static void timer0_isr();
static void timer1_isr();
static void timer2_isr();

/***********HEADER FUNCTIONS************/

void interrupt_test_run() {
  // Initialize interrupt controller driver
  interrupts_init();

  // Register the ISRs for the three timers
  interrupts_register(TIMER_0_IRQ, timer0_isr);
  interrupts_register(TIMER_1_IRQ, timer1_isr);
  interrupts_register(TIMER_2_IRQ, timer2_isr);

  // Enable interrupt reading for the timers in the
  // interrupt controller
  interrupts_irq_enable(TIMER_0_IRQ);
  interrupts_irq_enable(TIMER_1_IRQ);
  interrupts_irq_enable(TIMER_2_IRQ);

  // Initialize led drivers
  leds_init();

  // Initialize the timers in count down mode with their
  // respective periods
  intervalTimer_initCountDown(TIMER_0, TIMER_0_PERIOD);
  intervalTimer_initCountDown(TIMER_1, TIMER_1_PERIOD);
  intervalTimer_initCountDown(TIMER_2, TIMER_2_PERIOD);

  // Enable interrupts within the timers
  intervalTimer_enableInterrupt(TIMER_0);
  intervalTimer_enableInterrupt(TIMER_1);
  intervalTimer_enableInterrupt(TIMER_2);

  // Start the timers
  intervalTimer_start(TIMER_0);
  intervalTimer_start(TIMER_1);
  intervalTimer_start(TIMER_2);

  // Loop forever blinking leds
  while (1)
    ;
}

/***********HELPER FUNCTIONS************/

// Invert led 0 whenever the interrupt occurs
static void timer0_isr() {
  // Acknowledge the timer interrupt
  intervalTimer_ackInterrupt(TIMER_0);
  // Invert the state of led 0
  leds_write(leds_read() ^ LED_0_MASK);
}

// Invert led 1 whenever the interrupt occurs
static void timer1_isr() {
  // Acknowledge the timer interrupt
  intervalTimer_ackInterrupt(TIMER_1);
  // Invert the state of led 1
  leds_write(leds_read() ^ LED_1_MASK);
}

// Invert led 2 whenever the interrupt occurs
static void timer2_isr() {
  // Acknowledge the timer interrupt
  intervalTimer_ackInterrupt(TIMER_2);
  // Invert the state of led 2
  leds_write(leds_read() ^ LED_2_MASK);
}