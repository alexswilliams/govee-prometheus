#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>

int populate_config();

void destroy_config();

const char *alias_from_address_or_null(const char *address);

int cfg_is_verbose_enabled();

uint8_t cfg_ignore_duplicates();

uint16_t cfg_scan_window();

uint16_t cfg_scan_interval();

uint16_t cfg_metric_ttl_seconds();
#endif // CONFIG_H
