#include "config.h"
#include "gameControl.h"

#include "intervalTimer.h"
#include "interrupts.h"
#include "buttons.h"

// Interrupt service routine to update game state
void game_isr() {
  intervalTimer_ackInterrupt(INTERVAL_TIMER_0);
  // gameControl_tick();
}

// Initialize drivers and the game control, run the game
int main() {
    gameControl_init();

    // Initialize timer interrupts
    interrupts_init();
    interrupts_register(INTERVAL_TIMER_0_INTERRUPT_IRQ, game_isr);
    interrupts_irq_enable(INTERVAL_TIMER_0_INTERRUPT_IRQ);

    intervalTimer_initCountDown(INTERVAL_TIMER_0, CONFIG_GAME_TIMER_PERIOD);
    intervalTimer_enableInterrupt(INTERVAL_TIMER_0);
    intervalTimer_start(INTERVAL_TIMER_0);

    // Main game loop
    while(1)
        ;
}