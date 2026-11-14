#ifndef FT9366_CAPTURE_H
#define FT9366_CAPTURE_H
#include "ft9366-frame.h"
/* Single-owner receive state; the USB adapter must serialize callbacks. */
enum ft9366_capture_state {
    FT9366_CAPTURE_IDLE, FT9366_CAPTURE_RECEIVING, FT9366_CAPTURE_READY,
    FT9366_CAPTURE_CANCELLED, FT9366_CAPTURE_TIMEOUT,
    FT9366_CAPTURE_DISCONNECTED, FT9366_CAPTURE_PROTOCOL_ERROR
};
enum ft9366_capture_result {
    FT9366_CAPTURE_OK, FT9366_CAPTURE_BAD_ARGUMENT,
    FT9366_CAPTURE_BAD_STATE, FT9366_CAPTURE_BAD_LENGTH,
    FT9366_CAPTURE_SMALL_OUTPUT
};
struct ft9366_capture {
    enum ft9366_capture_state state;
    size_t received;
    uint8_t bytes[FT9366_FRAME_BYTES];
};
/* Initialize once before use. Reset only after the adapter has drained old callbacks. */
void ft9366_capture_reset(struct ft9366_capture *capture);
enum ft9366_capture_result ft9366_capture_begin(struct ft9366_capture *capture);
/* Chunk must not overlap capture storage. Feed only the payload of a known frame response. */
enum ft9366_capture_result ft9366_capture_feed(struct ft9366_capture *capture,
                                              const uint8_t *chunk, size_t length);
/* Samples must not overlap capture storage. Only READY can be consumed. */
enum ft9366_capture_result ft9366_capture_take(struct ft9366_capture *capture,
                                              uint16_t *samples, size_t capacity);
/* Abort accepts cancellation, timeout or disconnect; wipes any pending frame. */
enum ft9366_capture_result ft9366_capture_abort(struct ft9366_capture *capture,
                                               enum ft9366_capture_state reason);
#endif
