#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H
#include "govee.h"

typedef struct _device_list_entry {
    struct _device_list_entry *next;
    sensor_data data;
    char *address;
    char *name;
    char *alias;
    uint64_t last_seen;
    uint64_t samples_counted;
} device_list_entry;

device_list_entry *device_list_raw();

void add_or_update_sensor_by_address(const char *address, const char *name, const char *alias, const sensor_data *data);

void destory_device_list();
#endif // DEVICE_LIST_H
