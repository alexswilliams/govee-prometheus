#include "bluetooth_eir.h"

#include <string.h>
#include <asm-generic/errno-base.h>

#define min(a, b) (a<b?a:b)

// From <bluetooth/hci.h>
#define HCI_MAX_EIR_LENGTH 240

typedef struct {
    uint8_t length;
    uint8_t data_type;
    uint8_t data[];
} __attribute__ ((packed)) eir_structure;

static const eir_structure *find_first_eir_structure(const uint8_t *const eir, const size_t eir_reported_size,
                                                     const uint8_t data_type) {
    if (eir == NULL) return NULL;
    const size_t eir_size = min(eir_reported_size, HCI_MAX_EIR_LENGTH);
    size_t offset = 0;
    do {
        const eir_structure *structure = (eir_structure *) (eir + offset);
        if (structure->length == 0) return NULL;
        if (structure->length + offset > eir_size) return NULL;
        if (structure->data_type == data_type) {
            return structure;
        }
        offset += structure->length + 1;
    } while (offset < eir_size);
    return NULL;
}

#define EIR_FLAGS 0x01
#define EIR_NAME_SHORT 0x08
#define EIR_NAME_COMPLETE 0x09
#define EIR_MANUFACTURER_DATA 0xff

int read_flags_from_eir(const uint8_t *const eir, const size_t eir_size, uint8_t *const ret_flags) {
    const eir_structure *flags_structure = find_first_eir_structure(eir, eir_size, EIR_FLAGS);
    if (flags_structure == NULL) return -ENOENT;
    *ret_flags = flags_structure->data[0];
    return 0;
}

int read_name_from_eir(const uint8_t *const eir, const size_t eir_reported_size, char *const buf,
                       const size_t buf_len) {
    const eir_structure *name_structure = find_first_eir_structure(eir, eir_reported_size, EIR_NAME_COMPLETE);
    if (name_structure == NULL) name_structure = find_first_eir_structure(eir, eir_reported_size, EIR_NAME_SHORT);
    if (name_structure == NULL) return -ENOENT;
    if (name_structure->length - 1u > buf_len) return -EINVAL;
    memcpy(buf, name_structure->data, name_structure->length - 1);
    return 0;
}

const meta_manufacturer_payload *read_manufacturer_data_from_eir(const uint8_t *const eir,
                                                                 const size_t eir_reported_size) {
    const eir_structure *manufacturer = find_first_eir_structure(eir, eir_reported_size, EIR_MANUFACTURER_DATA);
    if (manufacturer == NULL) return NULL;
    if (manufacturer->length - 1 < 2) return NULL;
    return (meta_manufacturer_payload *) manufacturer->data;
}
