#ifndef GOVEE_H
#define GOVEE_H
#include <stdbool.h>
#include <stdint.h>


typedef struct {
    bool has_error;
    uint8_t battery_percent;
    float temperature;
    float humidity;
} sensor_data;

typedef struct {
    uint16_t company_id;
    uint32_t data_word;
    uint8_t battery_level_and_error;
    uint8_t padding_2;
} __attribute__ ((packed)) govee_payload;

void interpret_payload(const govee_payload *data, sensor_data *out);
#endif // GOVEE_H
