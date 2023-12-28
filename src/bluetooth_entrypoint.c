#include <stdio.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "govee.h"
#include "scan.h"

static void *panic(const char *const string) {
    perror(string);
    return NULL;
}

#define DEFAULT_TIMEOUT 1000

#define SCAN_TYPE_PASSIVE 0
#define SCAN_TYPE_ACTIVE 1
#define OWN_TYPE_PUBLIC 0
#define OWN_TYPE_RANDOM 1
#define FILTER_POLICY_NONE 0
#define FILTER_POLICY_ALLOW_LIST 1
#define REMOVE_DUPLICATES 1
#define KEEP_DUPLICATES 0

void *ble_thread_entrypoint(void *arg) {
    const int device_id = hci_get_route(NULL);
    if (device_id < 0) return panic("No available bluetooth devices\n");
    const int handle = hci_open_dev(device_id);
    if (handle < 0) return panic("Could not open socket to bluetooth device\n");
    fprintf(stderr, "Opened hci%d on fd %d\n", device_id, handle);

    // Stop any previous scan from running
    if (hci_le_set_scan_enable(handle, 0, REMOVE_DUPLICATES, DEFAULT_TIMEOUT) >= 0)
        fputs("Stopping previous scan...\n", stderr);

    if (hci_le_set_scan_parameters(handle, SCAN_TYPE_PASSIVE, htobs(19), htobs(13),
                                   OWN_TYPE_RANDOM,FILTER_POLICY_NONE, DEFAULT_TIMEOUT) < 0)
        return panic("Could not set scan params");

    if (hci_le_set_scan_enable(handle, 1, REMOVE_DUPLICATES, DEFAULT_TIMEOUT) < 0)
        return panic("Could not begin LE scan");

    receive_event_loop(handle, handle_govee_event_advertising_packet);

    fputs("\nStopping scan\n", stderr);
    if (hci_le_set_scan_enable(handle, 0, REMOVE_DUPLICATES, DEFAULT_TIMEOUT) < 0)
        perror("Could not end LE scan");
    hci_close_dev(handle);
    return NULL;
}
