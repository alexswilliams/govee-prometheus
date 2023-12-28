#include "govee.h"
#include <byteswap.h>

void interpret_payload(const govee_payload *const data, sensor_data *const out) {
    out->has_error = (data->battery_level_and_error & 0x80) != 0;
    out->battery_percent = data->battery_level_and_error & 0x7f;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    const int word = bswap_32(data->data_word);
#else
    const int word = data->data_word;
#endif
    const float temperature = (word & 0x7fffff) / 1000 / 10.0f;
    out->temperature = word & 0x800000 ? -temperature : temperature;
    out->humidity = (word & 0x7fffff) % 1000 / 10.0f;
}
