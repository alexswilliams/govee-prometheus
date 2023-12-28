#include "interrupts.h"

#include <stdlib.h>
#include <signal.h>

static int __trapped_signal = 0;
static int __need_to_exit = 0;

static void signal_handler(const int signal) {
    __trapped_signal = signal;
}

void setup_interrupt_trapping() {
    struct sigaction signal = {0};
    signal.sa_handler = signal_handler;
    sigaction(SIGINT, &signal, NULL);
}

void signal_exit_needed() {
    __need_to_exit = 1;
}

int exit_requested() {
    return __trapped_signal == SIGINT || __need_to_exit != 0;
}
