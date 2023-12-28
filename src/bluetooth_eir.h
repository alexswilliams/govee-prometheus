#ifndef BLUETOOTH_EIR_H
#define BLUETOOTH_EIR_H
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint16_t company_id;
    uint8_t data[];
} __attribute__((packed)) meta_manufacturer_payload;


int read_flags_from_eir(const uint8_t *eir, size_t eir_size, uint8_t *ret_flags);

int read_name_from_eir(const uint8_t *eir, size_t eir_reported_size, char *buf, size_t buf_len);

const meta_manufacturer_payload *read_manufacturer_data_from_eir(const uint8_t *eir, size_t eir_reported_size);
#endif // BLUETOOTH_EIR_H
