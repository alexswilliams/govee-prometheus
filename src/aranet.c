#include "aranet.h"

#include <stdlib.h>

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

// It seems to show up as a separate company coming from the same MAC
#define ARANET_DATA_COMPANY_ID 1794

void handle_aranet_event_advertising_packet(const le_advertising_info *const info) {
    char address[18] = {0};
    char name[30] = {0};
    ba2str(&info->bdaddr, address);
    const char *const alias = alias_from_address_or_null(address);
    if (read_name_from_eir(info->data, info->length, name, sizeof(name) - 1) < 0)
        strcpy(name, UNKNOWN);

    const meta_manufacturer_payload *const meta_payload = read_manufacturer_data_from_eir(info->data, info->length);
    if (meta_payload == NULL || btohs(meta_payload->company_id) != ARANET_DATA_COMPANY_ID) {
        return;
    }

    memcpy(&aranet_single_item.data, meta_payload->data, sizeof(aranet_single_item.data));
    aranet_single_item.last_seen = now();
    aranet_single_item.samples_counted++;

    if (aranet_single_item.address == NULL) {
        aranet_single_item.address = strdup(address);
    } else {
        if (strncmp(aranet_single_item.address, address, 18) != 0) {
            char *old = aranet_single_item.address;
            aranet_single_item.address = strdup(address);
            free(old);
        }
    }

    if (aranet_single_item.name == NULL) {
        aranet_single_item.name = strdup(name);
    } else {
        if (strncmp(aranet_single_item.name, name, 30) != 0) {
            char *old = aranet_single_item.name;
            aranet_single_item.name = strdup(name);
            free(old);
        }
    }

    const char *const new_alias = alias == NULL ? UNKNOWN : alias;
    if (aranet_single_item.alias == NULL) {
        aranet_single_item.alias = strdup(new_alias);
    } else {
        if (strcmp(aranet_single_item.alias, new_alias) != 0) {
            char *old = aranet_single_item.alias;
            aranet_single_item.alias = strdup(new_alias);
            free(old);
        }
    }

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

void destroy_aranet_event_data() {
    aranet_device_list_entry *this_node = &aranet_single_item;
    while (this_node != NULL) {
        aranet_device_list_entry *next_node = this_node->next;
        if (this_node->address != NULL) free(this_node->address);
        if (this_node->name != NULL) free(this_node->name);
        if (this_node->alias != NULL) free(this_node->alias);
        if (this_node != &aranet_single_item) free(this_node);
        this_node = next_node;
    }
}