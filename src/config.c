#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEEP_DUPLICATES 0
#define REMOVE_DUPLICATES 1

#define QUIET 0
#define VERBOSE 1

typedef struct _device_alias {
    char *address;
    char *alias;
    struct _device_alias *next;
} device_alias_item;

static volatile struct config {
    uint8_t scan_ignore_duplicates;
    uint16_t scan_interval;
    uint16_t scan_window;
    device_alias_item *aliases;
    int verbose;
} config = {
    KEEP_DUPLICATES,
    53,
    17,
    NULL,
    QUIET
};

uint16_t cfg_scan_interval() {
    return config.scan_interval;
}

uint16_t cfg_scan_window() {
    return config.scan_window;
}

uint8_t cfg_ignore_duplicates() {
    return config.scan_ignore_duplicates;
}

int cfg_is_verbose_enabled() {
    return config.verbose == VERBOSE;
}

static int parse_to_ushort(const char *const string, uint16_t *const dest) {
    char *first_invalid_digit;
    const uintmax_t number = strtoumax(string, &first_invalid_digit, 10);
    if (number == UINTMAX_MAX || first_invalid_digit[0] != '\0') return -1;
    if (number > 0xffff) return -1;
    *dest = number & 0xffff;
    return 0;
}

#define ALIAS_FILE_MAX_LINE_LENGTH 255

static int load_aliases(const char *const env_aliases_file) {
    FILE *const fd = fopen(env_aliases_file, "r");
    if (fd == NULL) {
        perror("Could not open aliases file");
        return -1;
    }
    size_t buffer_size = ALIAS_FILE_MAX_LINE_LENGTH;
    char *line = malloc(buffer_size);
    int line_num = 1;
    for (size_t read = getline(&line, &buffer_size, fd);
         read != -1;
         read = getline(&line, &buffer_size, fd), line_num++) {
        if (read == 0) continue;

        const char *ptr = line;
        while (ptr[0] != '\0' && ptr[0] != '\n' && isblank(ptr[0])) ptr++; // skip blanks before address
        if (ptr[0] == '\0' || ptr[0] == '\n') continue; // line was blank but not empty
        const char *const addr_start = ptr;
        while ((isxdigit(ptr[0]) || ptr[0] == ':') && ptr - addr_start < 17) ptr++;
        if (ptr - addr_start != 17 || (!isblank(ptr[0]) && ptr[0] != ',')) {
            fprintf(stderr, "Alias file line %d contained invalid address: %s\n", line_num, line);
            goto invalid_file;
        }
        const char *const addr_end = ptr;
        while (ptr[0] != '\0' && ptr[0] != '\n' && isblank(ptr[0])) ptr++; // skip blank space before comma
        if (ptr[0] != ',') {
            fprintf(stderr, "Alias file line %d: expected comma: %s\n", line_num, line);
            goto invalid_file;
        }
        ptr++;
        while (ptr[0] != '\0' && ptr[0] != '\n' && isblank(ptr[0])) ptr++; // skip blank space before alias
        if (ptr[0] == '\0' || ptr[0] == '\n') {
            fprintf(stderr, "Alias file line %d: Alias missing: %s\n", line_num, line);
            goto invalid_file;
        }
        const char *const alias_start = ptr;
        while (ptr[0] != '\0' && ptr[0] != '\n'
               && !iscntrl(ptr[0])
               && ptr[0] != '"' && ptr[0] != '\'' && ptr[0] != '\\')
            ptr++;
        if (ptr[0] != '\n' && ptr[0] != '\0') {
            fprintf(stderr, "Alias file line %d: Alias contains disallowed characters: %s\n", line_num, line);
            goto invalid_file;
        }
        while (isspace(ptr[-1]) && ptr > alias_start) ptr--; // reverse back over blank space after alias
        const char *const alias_end = ptr;
        if (alias_start == alias_end) {
            fprintf(stderr, "Alias file line %d: Alias was 0-sized: %s\n", line_num, line);
            goto invalid_file;
        }

        device_alias_item *alias_entry = malloc(sizeof(device_alias_item));
        alias_entry->address = strndup(addr_start, addr_end - addr_start);
        alias_entry->alias = strndup(alias_start, alias_end - alias_start);
        alias_entry->next = config.aliases;
        config.aliases = alias_entry;
    }

    if (line != NULL) free(line);
    fclose(fd);
    return 0;
invalid_file:
    fflush(stderr);
    if (line != NULL) free(line);
    fclose(fd);
    return -1;
}

int populate_config() {
    const char *const env_verbose = getenv("VERBOSE");
    if (env_verbose != NULL) {
        if (strcasecmp(env_verbose, "true") || strcasecmp(env_verbose, "verbose")) {
            config.verbose = VERBOSE;
        } else {
            config.verbose = QUIET;
        }
    }

    const char *const env_ignore_duplicates = getenv("BLE_IGNORE_DUPLICATES");
    if (env_ignore_duplicates != NULL) {
        if (strcasecmp(env_ignore_duplicates, "true")) {
            config.scan_ignore_duplicates = REMOVE_DUPLICATES;
        } else {
            config.scan_ignore_duplicates = KEEP_DUPLICATES;
        }
    }

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

    const char *const env_aliases_file = getenv("BT_ALIASES");
    if (env_aliases_file != NULL) {
        if (load_aliases(env_aliases_file) != 0)
            return -1;
    }

    fprintf(stderr, "Starting with config:\n"
            " • Log output: %s\n"
            " • Duplicate messages: %s\n"
            " • Scan interval: %d (%.3fms)\n"
            " • Scan window: %d (%.3fms)\n",
            config.verbose == VERBOSE ? "verbose" : "quiet",
            config.scan_ignore_duplicates == REMOVE_DUPLICATES ? "remove" : "keep",
            config.scan_interval, config.scan_interval * 0.625f,
            config.scan_window, config.scan_window * 0.625f
    );
    if (config.aliases != NULL) {
        fprintf(stderr, "Known aliases:\n");
        for (const device_alias_item *dev = config.aliases; dev != NULL; dev = dev->next) {
            fprintf(stderr, " • [%s] -> [%s]\n", dev->address, dev->alias);
        }
    }
    fflush(stderr);
    return 0;
}

const char *alias_from_address_or_null(const char *const address) {
    for (const device_alias_item *dev = config.aliases; dev != NULL; dev = dev->next) {
        if (strcasecmp(address, dev->address) == 0) return dev->alias;
    }
    return NULL;
}

void destroy_config() {
    device_alias_item *this_node = config.aliases;
    config.aliases = NULL;
    while (this_node != NULL) {
        device_alias_item *next_node = this_node->next;
        free(this_node->address);
        free(this_node->alias);
        free(this_node);
        this_node = next_node;
    }
}
