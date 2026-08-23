#include "ft9366-frame.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    /* Wrong byte order, partial writes on error, or omitted bounds checks fail these tests. */
    uint8_t raw[FT9366_FRAME_BYTES + 1] = {0};
    uint16_t out[FT9366_PIXELS + 1];
    for (size_t i = 0; i < FT9366_PIXELS + 1; i++) out[i] = 0xbeef;
    raw[0] = 0x12; raw[1] = 0x34;
    raw[FT9366_FRAME_BYTES - 2] = 0xff;
    raw[FT9366_FRAME_BYTES - 1] = 0xfe;
    assert(ft9366_decode_frame(NULL, FT9366_FRAME_BYTES, out, FT9366_PIXELS) == FT9366_INVALID_ARGUMENT);
    assert(ft9366_decode_frame(raw, FT9366_FRAME_BYTES, NULL, FT9366_PIXELS) == FT9366_INVALID_ARGUMENT);
    const size_t invalid[] = {0, 1, FT9366_FRAME_BYTES - 1, FT9366_FRAME_BYTES + 1, SIZE_MAX};
    for (size_t i = 0; i < sizeof invalid / sizeof invalid[0]; i++)
        assert(ft9366_decode_frame(raw, invalid[i], out, FT9366_PIXELS) == FT9366_INVALID_LENGTH);
    assert(ft9366_decode_frame(raw, FT9366_FRAME_BYTES, out, FT9366_PIXELS - 1) == FT9366_OUTPUT_TOO_SMALL);
    for (size_t i = 0; i < FT9366_PIXELS + 1; i++) assert(out[i] == 0xbeef);
    assert(ft9366_decode_frame(raw, FT9366_FRAME_BYTES, out, FT9366_PIXELS) == FT9366_OK);
    assert(out[0] == 0x1234);
    for (size_t i = 1; i < FT9366_PIXELS - 1; i++) assert(out[i] == 0);
    assert(out[FT9366_PIXELS - 1] == 0xfffe);
    assert(out[FT9366_PIXELS] == 0xbeef);
    puts("frame decoder: all checks passed (synthetic data only)");
    return 0;
}
