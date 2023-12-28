#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "interrupts.h"

void *ble_thread_entrypoint(void *arg);
void *prom_thread_entrypoint(void *arg);

static void panic(const char *const string) {
    perror(string);
    exit(1);
}

int main(void) {
    setup_interrupt_trapping();

    pthread_t ble_thread;
    if (pthread_create(&ble_thread, NULL, ble_thread_entrypoint, NULL) != 0)
        panic("Could not create BLE thread");
    pthread_t prom_thread;
    if (pthread_create(&prom_thread, NULL, prom_thread_entrypoint, NULL) != 0)
        panic("Could not create Prometheus thread");

    if (pthread_join(ble_thread, NULL) < 0)
        panic("Could not join with BLE thread");
    if (pthread_join(prom_thread, NULL) < 0)
        panic("Could not join with Prometheus thread");
    printf("Ended both threads successfully\n");
    exit(0);
}
