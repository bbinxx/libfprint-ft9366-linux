#include "ft9366-transport.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct fake_usb {
    bool accept;
    unsigned submits, cancels;
    uint64_t token;
    uint8_t endpoint, *buffer;
    size_t length;
    uint32_t timeout;
};
static bool submit(void *context, uint64_t token, uint8_t endpoint,
                   uint8_t *buffer, size_t length, uint32_t timeout)
{
    struct fake_usb *usb = context;
    usb->submits++;
    usb->token = token; usb->endpoint = endpoint; usb->buffer = buffer;
    usb->length = length; usb->timeout = timeout;
    return usb->accept;
}
static void cancel(void *context, uint64_t token)
{
    struct fake_usb *usb = context;
    assert(token == usb->token);
    usb->cancels++;
}
static void erased(const struct ft9366_transport *t)
{
    for (size_t i = 0; i < sizeof t->buffer; i++) assert(t->buffer[i] == 0);
}
int main(void)
{
    struct fake_usb usb = {.accept = true};
    const struct ft9366_usb_backend backend = {submit, cancel, &usb};
    struct ft9366_transport t;
    assert(ft9366_transport_init(NULL, &backend) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_transport_init(&t, NULL) == FT9366_USB_BAD_ARGUMENT);
    struct ft9366_usb_backend invalid = {0};
    assert(ft9366_transport_init(&t, &invalid) == FT9366_USB_BAD_ARGUMENT);
    uint8_t out[4] = {0};
    uint8_t command[] = {0x12, 0x34}; /* Synthetic, never sent to hardware. */
    assert(ft9366_transport_init(&t, &backend) == FT9366_USB_OK);
    assert(t.state == FT9366_USB_IDLE); erased(&t);
    assert(ft9366_transport_start(&t, FT9366_USB_READ, NULL, 0, 100) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_transport_start(&t, FT9366_USB_READ, NULL, SIZE_MAX, 100) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_transport_start(&t, FT9366_USB_READ, NULL, 4, 0) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_transport_start(&t, FT9366_USB_WRITE, NULL, 2, 100) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_transport_start(&t, (enum ft9366_direction)99, NULL, 2, 100) == FT9366_USB_BAD_ARGUMENT);
    assert(usb.submits == 0);
    assert(ft9366_transport_start(&t, FT9366_USB_WRITE, command, 2, 250) == FT9366_USB_OK);
    assert(usb.endpoint == 0x01 && usb.length == 2 && usb.timeout == 250);
    command[0] = 0xff; /* Submission owns a copy. */
    assert(usb.buffer[0] == 0x12);
    assert(ft9366_transport_start(&t, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_BAD_STATE);
    assert(ft9366_transport_release(&t) == FT9366_USB_BAD_STATE);
    assert(ft9366_transport_complete(&t, usb.token, FT9366_USB_DONE, 2) == FT9366_USB_OK);
    assert(t.state == FT9366_USB_DONE); erased(&t);
    assert(ft9366_transport_read(&t, out, sizeof out) == FT9366_USB_BAD_STATE);
    uint64_t previous = usb.token;
    assert(ft9366_transport_release(&t) == FT9366_USB_OK);
    assert(ft9366_transport_start(&t, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
    assert(usb.endpoint == 0x82 && usb.token > previous);
    assert(ft9366_transport_complete(&t, previous, FT9366_USB_DONE, 2) == FT9366_USB_STALE);
    assert(t.state == FT9366_USB_ACTIVE);
    memcpy(usb.buffer, "abcd", 4);
    assert(ft9366_transport_complete(&t, usb.token, FT9366_USB_DONE, 4) == FT9366_USB_OK);
    assert(ft9366_transport_read(&t, out, 3) == FT9366_USB_SMALL_OUTPUT);
    assert(ft9366_transport_read(&t, out, 4) == FT9366_USB_OK);
    assert(memcmp(out, "abcd", 4) == 0);
    assert(ft9366_transport_complete(&t, usb.token, FT9366_USB_DONE, 4) == FT9366_USB_STALE);
    assert(ft9366_transport_release(&t) == FT9366_USB_OK); erased(&t);
    /* A cancellation acknowledgement may race with successful transfer completion.
     * Buffer remains borrowed until the callback; cancellation wins either outcome. */
    const enum ft9366_transport_state reasons[] = {
        FT9366_USB_CANCELLED, FT9366_USB_TIMEOUT, FT9366_USB_DISCONNECTED
    };
    for (size_t i = 0; i < sizeof reasons / sizeof reasons[0]; i++) {
        unsigned before = usb.cancels;
        assert(ft9366_transport_start(&t, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
        memset(usb.buffer, 0xa5, 4);
        assert(ft9366_transport_cancel(&t, reasons[i]) == FT9366_USB_OK);
        assert(t.state == FT9366_USB_CANCELLING);
        assert(usb.buffer[0] == 0xa5); /* Backend still owns the borrowed buffer. */
        assert(usb.cancels == before + 1);
        assert(ft9366_transport_cancel(&t, reasons[i]) == FT9366_USB_BAD_STATE);
        assert(ft9366_transport_release(&t) == FT9366_USB_BAD_STATE);
        assert(ft9366_transport_start(&t, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_BAD_STATE);
        assert(ft9366_transport_complete(&t, usb.token, FT9366_USB_DONE, 4) == FT9366_USB_OK);
        assert(t.state == reasons[i]); erased(&t);
        assert(ft9366_transport_release(&t) == FT9366_USB_OK);
    }
    const size_t bad_lengths[] = {0, 3, 5, SIZE_MAX};
    for (size_t i = 0; i < sizeof bad_lengths / sizeof bad_lengths[0]; i++) {
        assert(ft9366_transport_start(&t, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
        memset(usb.buffer, 0xa5, 4);
        assert(ft9366_transport_complete(&t, usb.token, FT9366_USB_DONE, bad_lengths[i]) == FT9366_USB_OK);
        assert(t.state == FT9366_USB_LENGTH_ERROR); erased(&t);
        assert(ft9366_transport_release(&t) == FT9366_USB_OK);
    }
    const enum ft9366_transport_state errors[] = {
        FT9366_USB_CANCELLED, FT9366_USB_TIMEOUT, FT9366_USB_DISCONNECTED, FT9366_USB_IO_ERROR
    };
    for (size_t i = 0; i < sizeof errors / sizeof errors[0]; i++) {
        assert(ft9366_transport_start(&t, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
        memset(usb.buffer, 0xa5, 4);
        assert(ft9366_transport_complete(&t, usb.token, errors[i], 2) == FT9366_USB_OK);
        assert(t.state == errors[i]); erased(&t);
        assert(ft9366_transport_release(&t) == FT9366_USB_OK);
    }
    /* Maximum-sized frame response and caller output bound. */
    uint8_t frame[FT9366_FRAME_BYTES + 1];
    memset(frame, 0xee, sizeof frame);
    assert(ft9366_transport_start(&t, FT9366_USB_READ, NULL, FT9366_FRAME_BYTES, 100) == FT9366_USB_OK);
    for (size_t i = 0; i < FT9366_FRAME_BYTES; i++) usb.buffer[i] = (uint8_t)i;
    assert(ft9366_transport_cancel(&t, FT9366_USB_DONE) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_transport_complete(&t, usb.token, FT9366_USB_ACTIVE, 0) == FT9366_USB_BAD_ARGUMENT);
    assert(t.state == FT9366_USB_ACTIVE);
    assert(ft9366_transport_complete(&t, usb.token, FT9366_USB_DONE, FT9366_FRAME_BYTES) == FT9366_USB_OK);
    assert(ft9366_transport_read(&t, frame, FT9366_FRAME_BYTES) == FT9366_USB_OK);
    for (size_t i = 0; i < FT9366_FRAME_BYTES; i++) assert(frame[i] == (uint8_t)i);
    assert(frame[FT9366_FRAME_BYTES] == 0xee);
    assert(ft9366_transport_release(&t) == FT9366_USB_OK);
    assert(ft9366_transport_cancel(&t, FT9366_USB_CANCELLED) == FT9366_USB_BAD_STATE);
    assert(ft9366_transport_release(&t) == FT9366_USB_BAD_STATE);
    assert(ft9366_transport_start(NULL, FT9366_USB_READ, NULL, 1, 100) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_transport_start(&t, FT9366_USB_READ, command, 1, 100) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_transport_complete(NULL, 0, FT9366_USB_DONE, 0) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_transport_cancel(NULL, FT9366_USB_CANCELLED) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_transport_read(NULL, out, sizeof out) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_transport_read(&t, NULL, 0) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_transport_release(NULL) == FT9366_USB_BAD_ARGUMENT);
    usb.accept = false;
    assert(ft9366_transport_start(&t, FT9366_USB_WRITE, command, 2, 100) == FT9366_USB_OK);
    assert(t.state == FT9366_USB_IO_ERROR); erased(&t);
    assert(ft9366_transport_release(&t) == FT9366_USB_OK);
    /* Never reuse a generation token, even at wraparound. */
    t.token = UINT64_MAX;
    unsigned before = usb.submits;
    assert(ft9366_transport_start(&t, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_BAD_STATE);
    assert(usb.submits == before);
    puts("USB transport: all checks passed (simulated backend only)");
}
