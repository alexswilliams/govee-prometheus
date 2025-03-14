#include "metrics.h"

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>

#include "aranet.h"
#include "govee_device_list.h"

#define BUF_SIZE 4096

void write_expanding(char **buf, const int offset, size_t *const buf_size, const char *const str, const size_t length) {
    if (length > *buf_size - offset - 1) {
        while (length > *buf_size - offset - 1)
            *buf_size *= 2;
        *buf = realloc(*buf, *buf_size);
    }
    strncpy(*buf + offset, str, *buf_size - offset - 1);
}

char *build_metrics_prometheus_response() {
    size_t buf_size = BUF_SIZE;
    int offset = 0;
    char *buf = malloc(buf_size);

    char *str = "# HELP govee_has_error Error flag reported by the sensor - 0 or 1\n"
            "# TYPE govee_has_error gauge\n";
    write_expanding(&buf, offset, &buf_size, str, strlen(str));
    offset += strlen(str);
    for_each_govee_device(device) {
        const int length = asprintf(
            &str, "govee_has_error{address=\"%s\",name=\"%s\",alias=\"%s\"} %d %lu\n",
            device->address, device->name, device->alias, device->data.has_error, device->last_seen);
        if (length == -1) goto failure;
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);
    }

    str = "# HELP govee_battery_level Battery level reported by the sensor - integer 0 to 99\n"
            "# TYPE govee_battery_level gauge\n";
    write_expanding(&buf, offset, &buf_size, str, strlen(str));
    offset += strlen(str);
    for_each_govee_device(device) {
        const int length = asprintf(
            &str, "govee_battery_level{address=\"%s\",name=\"%s\",alias=\"%s\"} %d %lu\n",
            device->address, device->name, device->alias, device->data.battery_percent, device->last_seen);
        if (length == -1) goto failure;
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);
    }

    str = "# HELP govee_temperature_celsius Temperature in Celsius to 1 decimal place - -99.9C to +99.9C\n"
            "# TYPE govee_temperature_celsius gauge\n";
    write_expanding(&buf, offset, &buf_size, str, strlen(str));
    offset += strlen(str);
    for_each_govee_device(device) {
        const int length = asprintf(
            &str, "govee_temperature_celsius{address=\"%s\",name=\"%s\",alias=\"%s\"} %.1f %lu\n",
            device->address, device->name, device->alias, device->data.temperature, device->last_seen);
        if (length == -1) goto failure;
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);
    }

    str = "# HELP govee_humidity_percent Relative humidity, percentage to 1 decimal place - 0.0 to 99.9\n"
            "# TYPE govee_humidity_percent gauge\n";
    write_expanding(&buf, offset, &buf_size, str, strlen(str));
    offset += strlen(str);
    for_each_govee_device(device) {
        const int length = asprintf(
            &str, "govee_humidity_percent{address=\"%s\",name=\"%s\",alias=\"%s\"} %.1f %lu\n",
            device->address, device->name, device->alias, device->data.humidity, device->last_seen);
        if (length == -1) goto failure;
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);
    }

    str = "# HELP govee_samples_counted Number of advertisements seen - integer >= 1\n"
            "# TYPE govee_samples_counted counter\n";
    write_expanding(&buf, offset, &buf_size, str, strlen(str));
    offset += strlen(str);
    for_each_govee_device(device) {
        const int length = asprintf(
            &str, "govee_samples_counted{address=\"%s\",name=\"%s\",alias=\"%s\"} %lu %lu\n",
            device->address, device->name, device->alias, device->samples_counted, device->last_seen);
        if (length == -1) goto failure;
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);
    }


    str = "# HELP aranet_battery_level Battery level reported by the sensor - integer 0 to 99\n"
            "# TYPE aranet_battery_level gauge\n";
    write_expanding(&buf, offset, &buf_size, str, strlen(str));
    offset += strlen(str);
    for_each_aranet_device(device) {
        const int length = asprintf(
            &str, "aranet_battery_level{address=\"%s\",name=\"%s\",alias=\"%s\"} %d %lu\n",
            device->address, device->name, device->alias, device->data.battery, device->last_seen);
        if (length == -1) goto failure;
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);
    }

    str = "# HELP aranet_temperature_celsius Temperature in Celsius to 20ths of a degree - -3276.8 to +3276.75C\n"
            "# TYPE aranet_temperature_celsius gauge\n";
    write_expanding(&buf, offset, &buf_size, str, strlen(str));
    offset += strlen(str);
    for_each_aranet_device(device) {
        const int length = asprintf(
            &str, "aranet_temperature_celsius{address=\"%s\",name=\"%s\",alias=\"%s\"} %.1f %lu\n",
            device->address, device->name, device->alias, get_aranet_temperature(&device->data), device->last_seen);
        if (length == -1) goto failure;
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);
    }

    str = "# HELP aranet_pressure_hpa Pressure in hPa to 10ths of a degree - -6553.6 to +6553.5C\n"
            "# TYPE aranet_pressure_hpa gauge\n";
    write_expanding(&buf, offset, &buf_size, str, strlen(str));
    offset += strlen(str);
    for_each_aranet_device(device) {
        const int length = asprintf(
            &str, "aranet_pressure_hpa{address=\"%s\",name=\"%s\",alias=\"%s\"} %.1f %lu\n",
            device->address, device->name, device->alias, get_aranet_pressure(&device->data), device->last_seen);
        if (length == -1) goto failure;
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);
    }

    str = "# HELP aranet_humidity_percent Relative humidity, percentage - 0 to 99\n"
            "# TYPE aranet_humidity_percent gauge\n";
    write_expanding(&buf, offset, &buf_size, str, strlen(str));
    offset += strlen(str);
    for_each_aranet_device(device) {
        const int length = asprintf(
            &str, "aranet_humidity_percent{address=\"%s\",name=\"%s\",alias=\"%s\"} %d %lu\n",
            device->address, device->name, device->alias, device->data.humidity, device->last_seen);
        if (length == -1) goto failure;
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);
    }

    str = "# HELP aranet_co2_ppm CO2 incidence in parts-per-million - 0 to 65535\n"
            "# TYPE aranet_co2_ppm gauge\n";
    write_expanding(&buf, offset, &buf_size, str, strlen(str));
    offset += strlen(str);
    for_each_aranet_device(device) {
        const int length = asprintf(
            &str, "aranet_co2_ppm{address=\"%s\",name=\"%s\",alias=\"%s\"} %d %lu\n",
            device->address, device->name, device->alias, device->data.co2, device->last_seen);
        if (length == -1) goto failure;
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);
    }

    str = "# HELP aranet_samples_counted Number of advertisements seen - integer >= 1\n"
            "# TYPE aranet_samples_counted counter\n";
    write_expanding(&buf, offset, &buf_size, str, strlen(str));
    offset += strlen(str);
    for_each_aranet_device(device) {
        const int length = asprintf(
            &str, "aranet_samples_counted{address=\"%s\",name=\"%s\",alias=\"%s\"} %lu %lu\n",
            device->address, device->name, device->alias, device->samples_counted, device->last_seen);
        if (length == -1) goto failure;
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);
    }


    buf[offset] = 0;
    return buf;
failure:
    free(buf);
    return NULL;
}
