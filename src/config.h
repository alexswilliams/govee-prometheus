#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>

int populate_config();

const char *alias_from_address_or_null(const char *address);

uint8_t cfg_ignore_duplicates();

uint16_t cfg_scan_window();

uint16_t cfg_scan_interval();
#endif // CONFIG_H
