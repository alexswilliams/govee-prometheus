#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "interrupts.h"
#include "govee.h"
#include "bluetooth_eir.h"

#define SCAN_TYPE_PASSIVE 0
#define SCAN_TYPE_ACTIVE 1
#define OWN_TYPE_PUBLIC 0
#define OWN_TYPE_RANDOM 1
#define FILTER_POLICY_NONE 0
#define FILTER_POLICY_ALLOW_LIST 1
#define REMOVE_DUPLICATES 1
#define KEEP_DUPLICATES 0
#define FILTER_TYPE_NONE 0
#define FILTER_TYPE_LIMITED 1
#define FILTER_TYPE_GENERAL 2

#define EIR_MANUFACTURER_DATA 0xff

#define GOVEE_COMPANY_ID 60552


void panic(const char *const string) {
    perror(string);
    exit(1);
}

int get_any_device() {
    const int device_id = hci_get_route(BDADDR_ANY);
    if (device_id < 0) panic("No available bluetooth devices\n");
    return device_id;
}


#define FLAGS_LIMITED_MODE_BIT 0b00000001
#define FLAGS_GENERAL_MODE_BIT 0b00000010

int check_filter(const int filter_type, const le_advertising_info *const info) {
    if (filter_type == FILTER_TYPE_NONE) return 1;
    uint8_t flags;
    if (read_flags_from_eir(info->data, info->length, &flags) < 0) return 0;

    switch (filter_type) {
        case FILTER_TYPE_LIMITED:
            if (flags & FLAGS_LIMITED_MODE_BIT) return 1;
            break;
        case FILTER_TYPE_GENERAL:
            if (flags & (FLAGS_LIMITED_MODE_BIT | FLAGS_GENERAL_MODE_BIT)) return 1;
            break;
        default:
            fputs("Unknown discovery type\n", stderr);
    }
    return 0;
}


const char *device_from_address(const char *const address) {
    if (strcmp(address, "A4:C1:38:BC:EE:53") == 0) return "Fridge";
    if (strcmp(address, "A4:C1:38:43:97:32") == 0) return "Bedroom";
    if (strcmp(address, "A4:C1:38:58:2A:4B") == 0) return "Bathroom";
    if (strcmp(address, "A4:C1:38:C7:EF:61") == 0) return "Kitchen";
    if (strcmp(address, "A4:C1:38:38:2D:72") == 0) return "Office";
    return "Unknown";
}

void report_events(const int device_handle, const int filter_type) {
    struct hci_filter old_filter;
    socklen_t old_filter_length = sizeof(old_filter);
    if (getsockopt(device_handle, SOL_HCI, HCI_FILTER, &old_filter, &old_filter_length) < 0) {
        perror("Could not retrieve old filter");
        return;
    }
    struct hci_filter new_filter;
    hci_filter_clear(&new_filter);
    hci_filter_set_ptype(HCI_EVENT_PKT, &new_filter);
    hci_filter_set_event(EVT_LE_META_EVENT, &new_filter);
    if (setsockopt(device_handle, SOL_HCI, HCI_FILTER, &new_filter, sizeof(new_filter)) < 0) {
        perror("Could not set filter on HCI device");
        return;
    }

    setup_interrupt_trapping();

    do {
        unsigned char buf[HCI_MAX_EVENT_SIZE];
        while (read(device_handle, buf, sizeof(buf)) < 0) {
            if (errno == EINTR && get_last_signal() == SIGINT) goto done;
            if (errno != EAGAIN && errno != EINTR) goto done;
        }
        const evt_le_meta_event *const meta_event = (evt_le_meta_event *) (buf + (1 + HCI_EVENT_HDR_SIZE));
        if (meta_event->subevent != EVT_LE_ADVERTISING_REPORT) goto done;

        const le_advertising_info *const advertising_info = (void *) (meta_event->data + 1);
        if (check_filter(filter_type, advertising_info)) {
            char address[18];
            ba2str(&advertising_info->bdaddr, address);

            char name[30] = {0};
            if (read_name_from_eir(advertising_info->data, advertising_info->length, name, sizeof(name) - 1) < 0)
                strcpy(name, "(unknown)");

            const eir_structure *manufacturer = find_first_eir_structure(
                advertising_info->data, advertising_info->length, EIR_MANUFACTURER_DATA);
            if (manufacturer == NULL || manufacturer->length - 1 != 8) {
                printf("%s, %s\n", address, name);
            } else {
                const govee_payload *payload = (govee_payload *) manufacturer->data;
                if (btohs(payload->company_id) != GOVEE_COMPANY_ID) printf("%s, %s\n", address, name);
                else {
                    sensor_data result;
                    interpret_payload(payload, &result);
                    printf(
                        "Error: %d, Battery: %d%%; Temp: %4.1f°C; Humidity: %4.1f%%, MAC: %s, Name: %s, Device: %s\n",
                        result.has_error, result.battery_percent, result.temperature,
                        result.humidity, address, name, device_from_address(address));
                }
            }
        }
    } while (1);
done:
    setsockopt(device_handle, SOL_HCI, HCI_FILTER, &old_filter, sizeof(old_filter));
}


int main(void) {
    const int device_id = get_any_device();
    const int handle = hci_open_dev(device_id);
    if (handle < 0) panic("Could not open socket to bluetooth device\n");
    printf("Opened hci%d on fd %d\n", device_id, handle);

    // Stop any previous scan from running
    if (hci_le_set_scan_enable(handle, 0, REMOVE_DUPLICATES, 1000) < 0)
        panic("Could not stop previous scan");

    int err = hci_le_set_scan_parameters(handle, SCAN_TYPE_PASSIVE, htobs(19), htobs(5), OWN_TYPE_RANDOM,
                                         FILTER_POLICY_NONE, 1000);
    if (err < 0) panic("Could not set scan params");
    err = hci_le_set_scan_enable(handle, 1, REMOVE_DUPLICATES, 1000);
    if (err < 0) panic("Could not begin LE scan");

    report_events(handle, FILTER_TYPE_LIMITED);

    puts("\nStopping scan");
    err = hci_le_set_scan_enable(handle, 0, REMOVE_DUPLICATES, 1000);
    if (err < 0) perror("Could not end LE scan");
    hci_close_dev(handle);
    exit(0);
}
