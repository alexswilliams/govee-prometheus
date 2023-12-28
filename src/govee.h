#ifndef GOVEE_H
#define GOVEE_H
// bluetooth.h is needed as hci.h does not include e.g. inttypes.h, or a definition of bdaddr_t
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

typedef struct {
    int has_error;
    uint8_t battery_percent;
    float temperature;
    float humidity;
} sensor_data;

void handle_govee_event_advertising_packet(const le_advertising_info *info);
#endif // GOVEE_H
