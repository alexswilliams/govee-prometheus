#ifndef BLUETOOTH_EIR_H
#define BLUETOOTH_EIR_H
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t length;
    uint8_t data_type;
    uint8_t data[];
} __attribute__ ((packed)) eir_structure;

const eir_structure *find_first_eir_structure(const uint8_t *eir, size_t eir_reported_size,
                                              uint8_t data_type);

int read_flags_from_eir(const uint8_t *eir, size_t eir_size, uint8_t *ret_flags);

int read_name_from_eir(const uint8_t *eir, size_t eir_reported_size, char *buf,
                       size_t buf_len);
#endif // BLUETOOTH_EIR_H
