#include "govee_device_list.h"

#include <stdlib.h>

#include "govee.h"

// Everything in this linked list is shared between the two worker threads - one writer and one reader.
// Volatile as it must not be optimised out; but no real bother about atomicity - assuming word-length writes are
// atomic it will always be updated in a way that preserves a sensible list.
static govee_device_list_entry *volatile list = NULL;

govee_device_list_entry *govee_device_list_raw() {
    return list;
}

static govee_device_list_entry *find_sensor_by_address(const char *const address) {
    for (govee_device_list_entry *node = list; node != NULL; node = node->next) {
        const int comparison = strcmp(address, node->address);
        if (comparison == 0) return node;
        if (comparison < 0) return NULL;
    }
    return NULL;
}

static void add_new_sensor(const char *const address, const char *const name, const char *const alias,
                           const govee_sensor_data *const data) {
    govee_device_list_entry *const new_sensor = malloc(sizeof(govee_device_list_entry));
    new_sensor->address = strdup(address);
    new_sensor->name = strdup(name);
    new_sensor->alias = strdup(alias);
    memcpy(&new_sensor->data, data, sizeof(new_sensor->data));
    new_sensor->last_seen = now();
    new_sensor->samples_counted = 1;

    govee_device_list_entry *volatile *node_ptr = &list;
    while (*node_ptr != NULL && strcmp(address, (*node_ptr)->address) > 0) node_ptr = &(*node_ptr)->next;
    new_sensor->next = *node_ptr;
    *node_ptr = new_sensor;
}

void add_or_update_govee_sensor_by_address(const char *const address, const char *const name, const char *const alias,
                                           const govee_sensor_data *const data) {
    govee_device_list_entry *const existing_sensor = find_sensor_by_address(address);
    if (existing_sensor == NULL) {
        add_new_sensor(address, name, alias, data);
    } else {
        if (strcmp(existing_sensor->name, name) != 0) {
            char *old = existing_sensor->name;
            existing_sensor->name = strdup(name);
            free(old);
        }
        if (strcmp(existing_sensor->alias, alias) != 0) {
            char *old = existing_sensor->alias;
            existing_sensor->alias = strdup(alias);
            free(old);
        }
        memcpy(&existing_sensor->data, data, sizeof(existing_sensor->data));
        existing_sensor->last_seen = now();
        existing_sensor->samples_counted++;
    }
}


void destroy_govee_device_list() {
    govee_device_list_entry *this_node = list;
    list = NULL;
    while (this_node != NULL) {
        govee_device_list_entry *next_node = this_node->next;
        free(this_node->address);
        free(this_node->alias);
        free(this_node->name);
        free(this_node);
        this_node = next_node;
    }
}
