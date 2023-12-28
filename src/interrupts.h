#ifndef INTERRUPTS_H
#define INTERRUPTS_H

void setup_interrupt_trapping();

void signal_exit_needed();

int exit_requested();
#endif // INTERRUPTS_H
