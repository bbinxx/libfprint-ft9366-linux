/* Original Track B frame boundary; protocol assumptions are recorded in docs/usb-protocol.md. */
#include "ft9366-frame.h"
enum ft9366_result
ft9366_decode_frame(const uint8_t *raw, size_t length,
                    uint16_t *samples, size_t capacity)
{
    if (raw == NULL || samples == NULL)
        return FT9366_INVALID_ARGUMENT;
    if (length != FT9366_FRAME_BYTES)
        return FT9366_INVALID_LENGTH;
    if (capacity < FT9366_PIXELS)
        return FT9366_OUTPUT_TOO_SMALL;
    for (size_t i = 0; i < FT9366_PIXELS; i++)
        samples[i] = (uint16_t)((uint16_t)raw[2 * i] * 256u + raw[2 * i + 1]);
    return FT9366_OK;
}
