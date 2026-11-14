#include "ft9366-capture.h"
#include <string.h>

/* Volatile stores prevent the compiler from dropping this local buffer wipe.
 * This does not erase caller buffers, allocator copies, swap or USB buffers. */
static void erase_frame(struct ft9366_capture *capture)
{
    volatile uint8_t *bytes = capture->bytes;
    for (size_t i = 0; i < sizeof capture->bytes; i++)
        bytes[i] = 0;
    capture->received = 0;
}

void ft9366_capture_reset(struct ft9366_capture *capture)
{
    if (capture == NULL)
        return;
    erase_frame(capture);
    capture->state = FT9366_CAPTURE_IDLE;
}

enum ft9366_capture_result ft9366_capture_begin(struct ft9366_capture *capture)
{
    if (capture == NULL)
        return FT9366_CAPTURE_BAD_ARGUMENT;
    if (capture->state != FT9366_CAPTURE_IDLE)
        return FT9366_CAPTURE_BAD_STATE;
    erase_frame(capture);
    capture->state = FT9366_CAPTURE_RECEIVING;
    return FT9366_CAPTURE_OK;
}

enum ft9366_capture_result
ft9366_capture_feed(struct ft9366_capture *capture, const uint8_t *chunk, size_t length)
{
    if (capture == NULL || chunk == NULL)
        return FT9366_CAPTURE_BAD_ARGUMENT;
    if (capture->state != FT9366_CAPTURE_RECEIVING)
        return FT9366_CAPTURE_BAD_STATE;
    if (length == 0)
        return FT9366_CAPTURE_BAD_LENGTH;
    /* Check remaining space before copying or adding an untrusted length. */
    if (capture->received > sizeof capture->bytes ||
        length > sizeof capture->bytes - capture->received) {
        erase_frame(capture);
        capture->state = FT9366_CAPTURE_PROTOCOL_ERROR;
        return FT9366_CAPTURE_BAD_LENGTH;
    }
    memcpy(capture->bytes + capture->received, chunk, length);
    capture->received += length;
    if (capture->received == sizeof capture->bytes)
        capture->state = FT9366_CAPTURE_READY;
    return FT9366_CAPTURE_OK;
}

enum ft9366_capture_result
ft9366_capture_take(struct ft9366_capture *capture, uint16_t *samples, size_t capacity)
{
    if (capture == NULL || samples == NULL)
        return FT9366_CAPTURE_BAD_ARGUMENT;
    if (capture->state != FT9366_CAPTURE_READY)
        return FT9366_CAPTURE_BAD_STATE;
    if (capacity < FT9366_PIXELS)
        return FT9366_CAPTURE_SMALL_OUTPUT;
    if (ft9366_decode_frame(capture->bytes, capture->received, samples, capacity) != FT9366_OK) {
        erase_frame(capture);
        capture->state = FT9366_CAPTURE_PROTOCOL_ERROR;
        return FT9366_CAPTURE_BAD_LENGTH;
    }
    ft9366_capture_reset(capture);
    return FT9366_CAPTURE_OK;
}

enum ft9366_capture_result
ft9366_capture_abort(struct ft9366_capture *capture, enum ft9366_capture_state reason)
{
    if (capture == NULL)
        return FT9366_CAPTURE_BAD_ARGUMENT;
    if (reason != FT9366_CAPTURE_CANCELLED && reason != FT9366_CAPTURE_TIMEOUT &&
        reason != FT9366_CAPTURE_DISCONNECTED)
        return FT9366_CAPTURE_BAD_ARGUMENT;
    if (capture->state != FT9366_CAPTURE_RECEIVING && capture->state != FT9366_CAPTURE_READY)
        return FT9366_CAPTURE_BAD_STATE;
    erase_frame(capture);
    capture->state = reason;
    return FT9366_CAPTURE_OK;
}
