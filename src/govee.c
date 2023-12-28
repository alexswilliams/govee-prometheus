#include "govee.h"

#include <byteswap.h>
#include <stdio.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "bluetooth_eir.h"
#include "device_list.h"


typedef struct {
    uint32_t data_word;
    uint8_t battery_level_and_error;
    uint8_t padding;
} __attribute__ ((packed)) govee_payload;

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define govee_ntohl(it) bswap_32(it)
#else
#define govee_ntohl(it) it
#endif

static void interpret_payload(const govee_payload *const data, sensor_data *const out) {
    out->has_error = (data->battery_level_and_error & 0x80) != 0;
    out->battery_percent = data->battery_level_and_error & 0x7f;
    const int word = govee_ntohl(data->data_word);
    const float temperature = (word & 0x7fffff) / 1000 / 10.0f;
    out->temperature = word & 0x800000 ? -temperature : temperature;
    out->humidity = (word & 0x7fffff) % 1000 / 10.0f;
}

static const char *device_from_address(const char *const address) {
    if (strcmp(address, "A4:C1:38:BC:EE:53") == 0) return "Fridge";
    if (strcmp(address, "A4:C1:38:43:97:32") == 0) return "Bedroom";
    if (strcmp(address, "A4:C1:38:58:2A:4B") == 0) return "Bathroom";
    if (strcmp(address, "A4:C1:38:C7:EF:61") == 0) return "Kitchen";
    if (strcmp(address, "A4:C1:38:38:2D:72") == 0) return "Office";
    return "Unknown";
}

#define GOVEE_COMPANY_ID 60552

void handle_govee_event_advertising_packet(const le_advertising_info *const info) {
    char address[18] = {0};
    char name[30] = {0};
    ba2str(&info->bdaddr, address);
    if (read_name_from_eir(info->data, info->length, name, sizeof(name) - 1) < 0)
        strcpy(name, "(unknown)");

    const meta_manufacturer_payload *const meta_payload = read_manufacturer_data_from_eir(info->data, info->length);
    if (meta_payload == NULL) {
        fprintf(stderr, "Could not read manufacturer data from payload\n");
        return;
    }
    if (btohs(meta_payload->company_id) != GOVEE_COMPANY_ID) {
        printf("Non-Govee Device: %s (%s)\n", address, name);
        return;
    }

    const govee_payload *const payload = (govee_payload *) meta_payload->data;
    sensor_data result;
    interpret_payload(payload, &result);
    add_or_update_sensor_by_address(address, name, &result);
    printf(
        "Error: %d, Battery: %d%%; Temp: %4.1f°C; Humidity: %4.1f%%, MAC: %s, Name: %s, Device: %s\n",
        result.has_error, result.battery_percent, result.temperature,
        result.humidity, address, name, device_from_address(address));
}
