#ifndef INTERRUPTS_H
#define INTERRUPTS_H

void setup_interrupt_trapping(void (*callback)());

void setup_dummy_trap(int sig_num);

void signal_exit_needed();

int exit_requested();
#endif // INTERRUPTS_H
