#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "interrupts.h"

void ble_thread_entrypoint();

void prom_thread_entrypoint();

static void *ble_routine(void *args) {
    ble_thread_entrypoint();
    signal_exit_needed();
    pthread_exit(NULL);
}

static void *prom_routine(void *args) {
    prom_thread_entrypoint();
    signal_exit_needed();
    pthread_exit(NULL);
}

static void panic(const char *const string) {
    perror(string);
    exit(1);
}

int main(void) {
    setup_interrupt_trapping();

    pthread_t prom_thread;
    if (pthread_create(&prom_thread, NULL, prom_routine, NULL) != 0)
        panic("Could not create Prometheus thread");
    pthread_t ble_thread;
    if (pthread_create(&ble_thread, NULL, ble_routine, NULL) != 0)
        panic("Could not create BLE thread");

    if (pthread_join(ble_thread, NULL) != 0)
        panic("Could not join with BLE thread");
    printf("BLE thread exited\n");
    if (pthread_join(prom_thread, NULL) != 0)
        panic("Could not join with Prometheus thread");
    printf("Prom thread exited\n");
    exit(0);
}
