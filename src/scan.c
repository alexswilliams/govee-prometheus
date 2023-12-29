#include "scan.h"

#include <errno.h>
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "bluetooth_eir.h"
#include "interrupts.h"

#define FLAGS_LIMITED_MODE_BIT 0x01

static int is_limited_mode(const le_advertising_info *const info) {
    uint8_t flags;
    if (read_flags_from_eir(info->data, info->length, &flags) < 0) return 0;
    if (flags & FLAGS_LIMITED_MODE_BIT) return 1;
    return 0;
}

static void dump_packet(const uint8_t *const buf, const size_t size) {
    char hex[size * 3 + 1];
    char *ptr = hex;
    for (int i = 0; i < size; i++) {
        ptr[0] = buf[i] >> 4 > 9 ? 'a' - 10 + (buf[i] >> 4) : '0' + (buf[i] >> 4);
        ptr[1] = (buf[i] & 0x0f) > 9 ? 'a' - 10 + (buf[i] & 0x0f) : '0' + (buf[i] & 0x0f);
        ptr[2] = ' ';
        ptr += 3;
    }
    ptr[0] = 0;
    fputs(hex, stderr);
}

static void receive_events(const int device_handle, const void (*handle_matching_event)(const le_advertising_info *)) {
    while (!exit_requested()) {
        uint8_t buf[HCI_MAX_EVENT_SIZE];
        ssize_t length_read;
        while ((length_read = read(device_handle, buf, sizeof(buf))) <= 0) {
            if (errno == EINTR && exit_requested()) return;
            if (errno != EAGAIN && errno != EINTR) {
                perror("Could not read BLE event");
                return;
            }
            if (length_read == 0) return; // EoF, e.g. perhaps the device was removed
            usleep(100);
        }

        if (length_read < HCI_EVENT_HDR_SIZE + EVT_LE_META_EVENT_SIZE) {
            fprintf(stderr, "Received event that was too short to be an LE Event: %ld bytes", length_read);
            dump_packet(buf, length_read);
            fflush(stderr);
            continue;
        }

        if (((hci_event_hdr *) buf)->evt != HCI_EVENT_PKT
            || ((evt_le_meta_event *) (buf + HCI_EVENT_HDR_SIZE + 1))->subevent != EVT_LE_ADVERTISING_REPORT) {
            fprintf(stderr, "Received BLE message that wasn't an EVENT packet or a advertising report: ");
            dump_packet(buf, length_read);
            fflush(stderr);
            continue;
        }

        const le_advertising_info *const advertising_info =
                (le_advertising_info *) (((evt_le_meta_event *) (buf + HCI_EVENT_HDR_SIZE + 1))->data + 1);

        if (!is_limited_mode(advertising_info)) continue;

        handle_matching_event(advertising_info);
    }
}

void receive_event_loop(const int device_handle, const void (*handle_matching_event)(const le_advertising_info *)) {
    struct hci_filter old_filter;
    socklen_t old_filter_length = sizeof(old_filter);
    if (getsockopt(device_handle, SOL_HCI, HCI_FILTER, &old_filter, &old_filter_length) < 0) {
        perror("Could not retrieve old filter");
        signal_exit_needed();
        return;
    }

    struct hci_filter new_filter = {0};
    hci_filter_set_ptype(HCI_EVENT_PKT, &new_filter);
    hci_filter_set_event(EVT_LE_META_EVENT, &new_filter);
    if (setsockopt(device_handle, SOL_HCI, HCI_FILTER, &new_filter, sizeof(new_filter)) < 0) {
        perror("Could not set filter on HCI device");
        signal_exit_needed();
        return;
    }

    receive_events(device_handle, handle_matching_event);

    if (setsockopt(device_handle, SOL_HCI, HCI_FILTER, &old_filter, sizeof(old_filter)) < 0)
        perror("Warning: could not restore old filter on HCI device");
}
