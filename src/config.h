#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>

int populate_config();

uint16_t cfg_scan_window();

uint16_t cfg_scan_interval();
#endif // CONFIG_H
