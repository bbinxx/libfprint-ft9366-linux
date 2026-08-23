#include "ft9366-capture.h"
#include <assert.h>
#include <stdio.h>
static void assert_erased(const struct ft9366_capture *c)
{
    assert(c->received == 0);
    for (size_t i = 0; i < sizeof c->bytes; i++) assert(c->bytes[i] == 0);
}
int main(void)
{
    struct ft9366_capture c;
    uint8_t raw[FT9366_FRAME_BYTES] = {0x12, 0x34};
    uint16_t out[FT9366_PIXELS];
    raw[10238] = 0xab; raw[10239] = 0xcd;
    ft9366_capture_reset(&c);
    assert(c.state == FT9366_CAPTURE_IDLE); assert_erased(&c);
    assert(ft9366_capture_feed(&c, raw, 1) == FT9366_CAPTURE_BAD_STATE);
    assert(ft9366_capture_take(&c, out, FT9366_PIXELS) == FT9366_CAPTURE_BAD_STATE);
    assert(ft9366_capture_begin(&c) == FT9366_CAPTURE_OK);
    assert(ft9366_capture_begin(&c) == FT9366_CAPTURE_BAD_STATE);
    assert(ft9366_capture_feed(&c, NULL, 1) == FT9366_CAPTURE_BAD_ARGUMENT);
    assert(ft9366_capture_feed(&c, raw, 0) == FT9366_CAPTURE_BAD_LENGTH);
    assert(ft9366_capture_feed(&c, raw, 1) == FT9366_CAPTURE_OK);
    assert(ft9366_capture_feed(&c, raw + 1, 511) == FT9366_CAPTURE_OK);
    assert(c.state == FT9366_CAPTURE_RECEIVING);
    out[0] = 0xbeef;
    assert(ft9366_capture_take(&c, out, FT9366_PIXELS) == FT9366_CAPTURE_BAD_STATE);
    assert(out[0] == 0xbeef);
    for (size_t i = 512; i < sizeof raw; i += 512)
        assert(ft9366_capture_feed(&c, raw + i, 512) == FT9366_CAPTURE_OK);
    assert(c.state == FT9366_CAPTURE_READY);
    assert(ft9366_capture_take(&c, NULL, FT9366_PIXELS) == FT9366_CAPTURE_BAD_ARGUMENT);
    assert(ft9366_capture_feed(&c, raw, 1) == FT9366_CAPTURE_BAD_STATE);
    assert(ft9366_capture_take(&c, out, FT9366_PIXELS - 1) == FT9366_CAPTURE_SMALL_OUTPUT);
    assert(c.state == FT9366_CAPTURE_READY);
    assert(ft9366_capture_take(&c, out, FT9366_PIXELS) == FT9366_CAPTURE_OK);
    assert(out[0] == 0x1234 && out[5119] == 0xabcd);
    assert(c.state == FT9366_CAPTURE_IDLE); assert_erased(&c);
    /* Every possible split of a response must decode identically. */
    for (size_t split = 1; split < sizeof raw; split++) {
        assert(ft9366_capture_begin(&c) == FT9366_CAPTURE_OK);
        assert(ft9366_capture_feed(&c, raw, split) == FT9366_CAPTURE_OK);
        assert(ft9366_capture_feed(&c, raw + split, sizeof raw - split) == FT9366_CAPTURE_OK);
        assert(ft9366_capture_take(&c, out, FT9366_PIXELS) == FT9366_CAPTURE_OK);
        assert(out[0] == 0x1234 && out[5119] == 0xabcd);
    }
    const enum ft9366_capture_state reasons[] = {
        FT9366_CAPTURE_CANCELLED, FT9366_CAPTURE_TIMEOUT, FT9366_CAPTURE_DISCONNECTED
    };
    for (size_t i = 0; i < sizeof reasons / sizeof reasons[0]; i++) {
        assert(ft9366_capture_begin(&c) == FT9366_CAPTURE_OK);
        assert(ft9366_capture_feed(&c, raw, 7) == FT9366_CAPTURE_OK);
        assert(ft9366_capture_abort(&c, reasons[i]) == FT9366_CAPTURE_OK);
        assert(c.state == reasons[i]); assert_erased(&c);
        assert(ft9366_capture_feed(&c, raw, 7) == FT9366_CAPTURE_BAD_STATE);
        assert(ft9366_capture_begin(&c) == FT9366_CAPTURE_BAD_STATE);
        ft9366_capture_reset(&c);
    }
    assert(ft9366_capture_begin(&c) == FT9366_CAPTURE_OK);
    assert(ft9366_capture_feed(&c, raw, 7) == FT9366_CAPTURE_OK);
    assert(ft9366_capture_feed(&c, raw, SIZE_MAX) == FT9366_CAPTURE_BAD_LENGTH);
    assert(c.state == FT9366_CAPTURE_PROTOCOL_ERROR); assert_erased(&c);
    ft9366_capture_reset(&c);
    assert(ft9366_capture_begin(&c) == FT9366_CAPTURE_OK);
    assert(ft9366_capture_feed(&c, raw, sizeof raw) == FT9366_CAPTURE_OK);
    assert(ft9366_capture_abort(&c, FT9366_CAPTURE_CANCELLED) == FT9366_CAPTURE_OK);
    assert_erased(&c);
    ft9366_capture_reset(&c);
    assert(ft9366_capture_abort(&c, FT9366_CAPTURE_READY) == FT9366_CAPTURE_BAD_ARGUMENT);
    assert(ft9366_capture_begin(NULL) == FT9366_CAPTURE_BAD_ARGUMENT);
    assert(ft9366_capture_feed(NULL, raw, 1) == FT9366_CAPTURE_BAD_ARGUMENT);
    assert(ft9366_capture_take(NULL, out, FT9366_PIXELS) == FT9366_CAPTURE_BAD_ARGUMENT);
    assert(ft9366_capture_abort(NULL, FT9366_CAPTURE_CANCELLED) == FT9366_CAPTURE_BAD_ARGUMENT);
    ft9366_capture_reset(NULL);
    puts("capture state: all checks passed (synthetic data only)");
}
