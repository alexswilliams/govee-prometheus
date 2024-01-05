#include "aranet.h"

#include <stdlib.h>
#include <time.h>

#include "bluetooth_eir.h"
#include "config.h"

static aranet_device_list_entry aranet_single_item = {
    .next = NULL,
    .data = {0},
    .address = NULL,
    .name = NULL,
    .alias = NULL,
    .last_seen = 0,
    .samples_counted = 0
};

aranet_device_list_entry *aranet_raw() {
    if (aranet_single_item.address == NULL) return NULL;
    return &aranet_single_item;
}

float get_aranet_temperature(const aranet_data *data) {
    return data->temperature / 20.0;
}

float get_aranet_pressure(const aranet_data *data) {
    return data->pressure / 10.0;
}

static void now_as_string(char *const buf, const size_t buf_size) {
    struct timespec tp;
    struct tm tm;
    clock_gettime(CLOCK_REALTIME, &tp);
    strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", gmtime_r(&tp.tv_sec, &tm));
}

static uint64_t now() {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (uint64_t) tp.tv_sec * 1000 + (uint64_t) tp.tv_nsec / 1000000;
}

// It seems to show up as a separate company coming from the same MAC
#define ARANET_DATA_COMPANY_ID 1794

void handle_aranet_event_advertising_packet(const le_advertising_info *const info) {
    char address[18] = {0};
    char name[30] = {0};
    ba2str(&info->bdaddr, address);
    const char *const alias = alias_from_address_or_null(address);
    if (read_name_from_eir(info->data, info->length, name, sizeof(name) - 1) < 0)
        strcpy(name, "(unknown)");

    const meta_manufacturer_payload *const meta_payload = read_manufacturer_data_from_eir(info->data, info->length);
    if (meta_payload == NULL) {
        fprintf(stderr, "Could not read manufacturer data from payload\n");
        fflush(stderr);
        return;
    }
    if (btohs(meta_payload->company_id) != ARANET_DATA_COMPANY_ID) {
        return;
    }

    memcpy(&aranet_single_item.data, meta_payload->data, sizeof(aranet_single_item.data));
    aranet_single_item.last_seen = now();
    aranet_single_item.samples_counted++;
    if (aranet_single_item.address != NULL) {
        char *old = aranet_single_item.address;
        aranet_single_item.address = strdup(address);
        free(old);
    } else aranet_single_item.address = strdup(address);
    if (aranet_single_item.name != NULL) {
        char *old = aranet_single_item.name;
        aranet_single_item.name = strdup(name);
        free(old);
    } else aranet_single_item.name = strdup(name);
    if (aranet_single_item.alias != NULL) {
        char *old = aranet_single_item.alias;
        if (alias != NULL) aranet_single_item.alias = strdup(alias);
        else aranet_single_item.alias = "(unknwon)";
        free(old);
    } else if (alias != NULL) aranet_single_item.alias = strdup(alias);
    else aranet_single_item.alias = "(unknown)";


    if (cfg_is_verbose_enabled()) {
        char time_string[22] = {0};
        now_as_string(time_string, sizeof(time_string));
        printf(
            "%s: ARANET DEVICE - MAC: %s, Name: %s, Device: %s, T: %.2f°C, H: %d%%, P: %.1f hPa, bat: %d%%, CO2: %d ppm\n",
            time_string, address, name, alias == NULL ? "Unknown" : alias,
            get_aranet_temperature(&aranet_single_item.data), aranet_single_item.data.humidity,
            get_aranet_pressure(&aranet_single_item.data), aranet_single_item.data.battery,
            aranet_single_item.data.co2);
        fflush(stdout);
    }
}
