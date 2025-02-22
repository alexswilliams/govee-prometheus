#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <stddef.h>
#include <bits/stdint-uintn.h>

uint64_t now();
void now_as_string(char *buf, size_t buf_size);

#endif // TIMEUTIL_H
