#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static volatile struct config {
    uint16_t scan_interval;
    uint16_t scan_window;
} config = {
    53,
    17
};

uint16_t cfg_scan_interval() {
    return config.scan_interval;
}

uint16_t cfg_scan_window() {
    return config.scan_window;
}

static int parse_to_ushort(const char *const string, uint16_t *const dest) {
    char *first_invalid_digit;
    const uintmax_t number = strtoumax(string, &first_invalid_digit, 10);
    if (number == UINTMAX_MAX || first_invalid_digit[0] != '\0') return -1;
    if (number > 0xffff) return -1;
    *dest = number & 0xffff;
    return 0;
}

int populate_config() {
    const char *const env_scan_interval = getenv("BLE_SCAN_INTERVAL");
    if (env_scan_interval != NULL) {
        uint16_t result;
        if (parse_to_ushort(env_scan_interval, &result) != 0) {
            fprintf(stderr, "Invaid number passed to config: %s\n", env_scan_interval);
            return -1;
        }
        config.scan_interval = result;
    }
    const char *const env_scan_window = getenv("BLE_SCAN_WINDOW");
    if (env_scan_window != NULL) {
        uint16_t result;
        if (parse_to_ushort(env_scan_window, &result) != 0) {
            fprintf(stderr, "Invaid number passed to config: %s\n", env_scan_window);
            return -1;
        }
        config.scan_window = result;
    }
    if (config.scan_interval < config.scan_window) {
        fprintf(stderr, "Interval (%d = %.2fms) must be at least as long as the window (%d = %.2fms)\n",
                config.scan_interval, config.scan_interval * 0.625f, config.scan_window, config.scan_window * 0.625f);
        return -1;
    }
    if (config.scan_interval < 16 || config.scan_interval > 16384) {
        fprintf(stderr, "Interval (%d = %.2fms) must be between 16 (10ms) and 16384 (10.24s)\n",
                config.scan_interval, config.scan_interval * 0.625f);
        return -1;
    }
    if (config.scan_window < 16 || config.scan_window > 16384) {
        fprintf(stderr, "Window (%d = %.2fms) must be between 16 (10ms) and 16384 (10.24s)\n",
                config.scan_window, config.scan_window * 0.625f);
        return -1;
    }

    fprintf(stderr, "Starting with config:\n"
            " • Scan interval: %d (%.3fms)\n"
            " • Scan window: %d (%.3fms)\n",
            config.scan_interval, config.scan_interval * 0.625f,
            config.scan_window, config.scan_window * 0.625f
    );
    fflush(stderr);
    return 0;
}
