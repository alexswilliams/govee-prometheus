#include "interrupts.h"

#include <stdlib.h>
#include <signal.h>

static volatile int __trapped_signal = 0;
static volatile int __need_to_exit = 0;

static const void (*trap_callback)() = NULL;

static void signal_handler(const int signal) {
    __trapped_signal = signal;
    if (trap_callback != NULL) trap_callback();
}

void setup_interrupt_trapping(const void (*const callback)()) {
    struct sigaction signal = {0};
    signal.sa_handler = signal_handler;
    sigaction(SIGINT, &signal, NULL);
    trap_callback = callback;
}

static void no_op_handler(const int signal) {
}

void setup_dummy_trap(const int sig_num) {
    struct sigaction signal = {0};
    signal.sa_handler = no_op_handler;
    sigaction(sig_num, &signal, NULL);
}

void signal_exit_needed() {
    __need_to_exit = 1;
    if (trap_callback != NULL) trap_callback();
}

int exit_requested() {
    return __trapped_signal == SIGINT || __need_to_exit != 0;
}
