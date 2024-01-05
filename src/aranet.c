#include "aranet.h"

#include <time.h>

#include "bluetooth_eir.h"
#include "config.h"

// It seems to show up as a separate company coming from the same MAC
#define ARANET_DATA_COMPANY_ID 1794

static void now_as_string(char *const buf, const size_t buf_size) {
    struct timespec tp;
    struct tm tm;
    clock_gettime(CLOCK_REALTIME, &tp);
    strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", gmtime_r(&tp.tv_sec, &tm));
}

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

    if (cfg_is_verbose_enabled()) {
        char time_string[22] = {0};
        now_as_string(time_string, sizeof(time_string));
        printf(
            "%s: ARANET DEVICE - MAC: %s, Name: %s, Device: %s\n",
            time_string, address, name, alias == NULL ? "Unknown" : alias);
        fflush(stdout);
    }
}
