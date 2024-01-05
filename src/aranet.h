#ifndef ARANET_H
#define ARANET_H
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

// Sourced from https://github.com/Anrijs/Aranet4-ESP32/blob/main/src/Aranet4.h
#define AR4_PACKING_ARANET4     1

// e.g. 210e0401000c0f010704860196263721023c002c0023

typedef struct {
    uint8_t disconnected: 1, // 21
            __unknown1: 1,
            calib_state: 2,
            dfu_mode: 1,
            integrations: 1,
            __unknown2: 2;

    uint8_t version_patch; // 0e
    uint8_t version_minor; // 04
    uint16_t version_major; // 0100
    uint8_t hw_rev; // 0c
    uint8_t __unknown3; // 0f
    uint8_t packing; // 01

    uint16_t co2; // 0704
    uint16_t temperature; // 8601
    uint16_t pressure; // 9626
    uint8_t humidity; // 37
    uint8_t battery; // 21
    uint8_t status; // 02
    uint16_t interval; // 3c00
    uint16_t ago; // 2c00
    uint8_t __unknown4; // 23
} __attribute__((packed)) aranet_data;

inline float get_temperature(const aranet_data *data) {
    return data->temperature / 20.0;
}

inline float get_pressure(const aranet_data *data) {
    return data->pressure / 10.0;
}

void handle_aranet_event_advertising_packet(const le_advertising_info *info);

typedef struct _aranet_device_list_entry {
    struct _aranet_device_list_entry *next;
    aranet_data data;
    char *address;
    char *name;
    char *alias;
    uint64_t last_seen;
    uint64_t samples_counted;
} aranet_device_list_entry;

aranet_device_list_entry *aranet_raw();

#define for_each_aranet_device(_dev) \
    for (aranet_device_list_entry* _dev = aranet_raw(); _dev != NULL; _dev = _dev->next)

#endif // ARANET_H
