#include "timeutil.h"

#include <time.h>

uint64_t now() {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (uint64_t) tp.tv_sec * 1000 + (uint64_t) tp.tv_nsec / 1000000;
}

void now_as_string(char *const buf, const size_t buf_size) {
    struct timespec tp;
    struct tm tm;
    clock_gettime(CLOCK_REALTIME, &tp);
    strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", gmtime_r(&tp.tv_sec, &tm));
}
