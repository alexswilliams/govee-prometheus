#include <stdio.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "govee.h"
#include "scan.h"
#include "config.h"

#define DEFAULT_TIMEOUT 1000

#define SCAN_TYPE_PASSIVE 0
#define SCAN_TYPE_ACTIVE 1
#define OWN_TYPE_PUBLIC 0
#define OWN_TYPE_RANDOM 1
#define FILTER_POLICY_NONE 0
#define FILTER_POLICY_ALLOW_LIST 1
#define REMOVE_DUPLICATES 1
#define KEEP_DUPLICATES 0

void ble_thread_entrypoint() {
    const int device_id = hci_get_route(NULL);
    if (device_id < 0) {
        perror("No available bluetooth devices\n");
        fflush(stderr);
        return;
    }
    const int handle = hci_open_dev(device_id);
    if (handle < 0) {
        perror("Could not open socket to bluetooth device\n");
        fflush(stderr);
        return;
    }
    fprintf(stderr, "Opened hci%d on fd %d\n", device_id, handle);
    fflush(stderr);

    if (hci_le_set_scan_enable(handle, 0, REMOVE_DUPLICATES, DEFAULT_TIMEOUT) >= 0) {
        fputs("Stopping previous scan...\n", stderr);
        fflush(stderr);
        // this isn't a failure case - this happens in case a previous copy of this program died with a scan running.
    }

    if (hci_le_set_scan_parameters(handle, SCAN_TYPE_PASSIVE, htobs(cfg_scan_interval()), htobs(cfg_scan_window()),
                                   OWN_TYPE_RANDOM,FILTER_POLICY_NONE, DEFAULT_TIMEOUT) < 0) {
        perror("Could not set scan params");
        fflush(stderr);
        return;
    }

    if (hci_le_set_scan_enable(handle, 1, REMOVE_DUPLICATES, DEFAULT_TIMEOUT) < 0) {
        perror("Could not begin LE scan");
        fflush(stderr);
        return;
    }

    receive_event_loop(handle, handle_govee_event_advertising_packet);

    fputs("\nStopping scan\n", stderr);
    fflush(stderr);
    if (hci_le_set_scan_enable(handle, 0, REMOVE_DUPLICATES, DEFAULT_TIMEOUT) < 0) {
        perror("Could not end LE scan");
        fflush(stderr);
    }
    hci_close_dev(handle);
}
