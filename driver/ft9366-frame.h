#ifndef FT9366_FRAME_H
#define FT9366_FRAME_H
#include <stddef.h>
#include <stdint.h>
/* Reference-derived geometry; not yet confirmed by local capture. */
#define FT9366_WIDTH 64u
#define FT9366_HEIGHT 80u
#define FT9366_PIXELS (FT9366_WIDTH * FT9366_HEIGHT)
#define FT9366_FRAME_BYTES (FT9366_PIXELS * 2u)
enum ft9366_result {
    FT9366_OK = 0,
    FT9366_INVALID_ARGUMENT,
    FT9366_INVALID_LENGTH,
    FT9366_OUTPUT_TOO_SMALL
};
/* Decode big-endian samples. Buffers must not overlap. On error, output is unchanged. */
enum ft9366_result ft9366_decode_frame(const uint8_t *raw, size_t length,
                                       uint16_t *samples, size_t capacity);
#endif
