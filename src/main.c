#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <bits/sigthread.h>

#include "device_list.h"
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
    fflush(stderr);
    exit(1);
}

static pthread_t prom_thread;
static pthread_t ble_thread;

static void interrupt_all() {
    pthread_kill(prom_thread, SIGALRM);
    pthread_kill(ble_thread, SIGALRM);
}

int main(void) {
    if (pthread_create(&prom_thread, NULL, prom_routine, NULL) != 0) panic("Could not create Prometheus thread");
    if (pthread_create(&ble_thread, NULL, ble_routine, NULL) != 0) panic("Could not create BLE thread");

    setup_dummy_trap(SIGALRM);
    setup_interrupt_trapping(interrupt_all);

    if (pthread_join(ble_thread, NULL) != 0) panic("Could not join with BLE thread");
    printf("BLE thread exited\n");
    fflush(stdout);
    if (pthread_join(prom_thread, NULL) != 0) panic("Could not join with Prometheus thread");
    printf("Prom thread exited\n");
    fflush(stdout);

    destory_device_list();
    exit(0);
}
