#include "metrics.h"

#include <stdlib.h>

#include "device_list.h"

#define BUF_SIZE 4096

void write_expanding(char **buf, const int offset, size_t *const buf_size, const char *const str, const int length) {
    if (length > *buf_size - offset - 1) {
        while (length > *buf_size - offset - 1)
            *buf_size *= 2;
        *buf = realloc(*buf, *buf_size);
    }
    strncpy(*buf + offset, str, *buf_size - offset - 1);
}

char *build_metrics() {
    size_t buf_size = BUF_SIZE;
    int offset = 0;
    char *buf = malloc(buf_size);

    char *str = "# TYPE has_error gauge\n"
            "# TYPE battery_level gauge\n"
            "# TYPE temperature_celcius gauge\n"
            "# TYPE humidity_percent gauge\n";
    int length = strlen(str);
    write_expanding(&buf, offset, &buf_size, str, length);
    offset += length;

    const device_list_entry *device = device_list_raw();
    while (device != NULL) {
        length = asprintf(
            &str, "has_error{address=\"%s\",name=\"%s\",alias=\"%s\"} %d %lu\n", device->address, device->name,
            "unknown", device->data.has_error, device->last_seen);
        if (length == -1) {
            free(buf);
            return NULL;
        }
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);

        length = asprintf(
            &str, "battery_level{address=\"%s\",name=\"%s\",alias=\"%s\"} %d %lu\n", device->address, device->name,
            "unknown", device->data.battery_percent, device->last_seen);
        if (length == -1) {
            free(buf);
            return NULL;
        }
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);

        length = asprintf(
            &str, "temperature_celcius{address=\"%s\",name=\"%s\",alias=\"%s\"} %.1f %lu\n", device->address,
            device->name, "unknown", device->data.temperature, device->last_seen);
        if (length == -1) {
            free(buf);
            return NULL;
        }
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);

        length = asprintf(
            &str, "humidity_percent{address=\"%s\",name=\"%s\",alias=\"%s\"} %.1f %lu\n", device->address, device->name,
            "unknown", device->data.humidity, device->last_seen);
        if (length == -1) {
            free(buf);
            return NULL;
        }
        write_expanding(&buf, offset, &buf_size, str, length);
        offset += length;
        free(str);
        device = device->next;
    }
    buf[offset] = 0;
    return buf;
}
