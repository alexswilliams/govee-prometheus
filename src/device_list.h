#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H
#include "govee.h"

void add_or_update_sensor_by_address(const char *address, const char *name, const sensor_data *data);

void destory_device_list();
#endif // DEVICE_LIST_H
