#include <stdlib.h>
#include <signal.h>
#include "interrupts.h"

static int __trapped_signal = 0;

static void signal_handler(const int signal) {
    __trapped_signal = signal;
}

void setup_interrupt_trapping() {
    struct sigaction signal = {0};
    signal.sa_handler = signal_handler;
    sigaction(SIGINT, &signal, NULL);
}

int get_last_signal() {
    return __trapped_signal;
}
