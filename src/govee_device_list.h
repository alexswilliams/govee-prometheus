#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H
#include "govee.h"
#include "timeutil.h"
#include "config.h"

typedef struct _govee_device_list_entry {
    struct _govee_device_list_entry *next;
    govee_sensor_data data;
    char *address;
    char *name;
    char *alias;
    uint64_t last_seen;
    uint64_t samples_counted;
} govee_device_list_entry;

govee_device_list_entry *govee_device_list_raw();

#define for_each_govee_device(_dev) \
    for (const govee_device_list_entry *_dev = govee_device_list_raw(); _dev != NULL; _dev = _dev->next) \
        if (now() - _dev->last_seen < cfg_metric_ttl_seconds() * 1000)

void add_or_update_govee_sensor_by_address(const char *address, const char *name, const char *alias,
                                           const govee_sensor_data *data);

void destroy_govee_device_list();
#endif // DEVICE_LIST_H
