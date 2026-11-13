#ifndef FT9366_TRANSPORT_H
#define FT9366_TRANSPORT_H
#include "ft9366-frame.h"
#include <stdbool.h>

enum ft9366_transport_state {
    FT9366_USB_IDLE, FT9366_USB_ACTIVE, FT9366_USB_CANCELLING,
    FT9366_USB_DONE, FT9366_USB_CANCELLED, FT9366_USB_TIMEOUT,
    FT9366_USB_DISCONNECTED, FT9366_USB_IO_ERROR, FT9366_USB_LENGTH_ERROR
};
enum ft9366_transport_result {
    FT9366_USB_OK, FT9366_USB_BAD_ARGUMENT, FT9366_USB_BAD_STATE,
    FT9366_USB_STALE, FT9366_USB_SMALL_OUTPUT
};
enum ft9366_direction { FT9366_USB_READ, FT9366_USB_WRITE };

/* submit must return before any callback. false means no transfer or callback exists.
 * true borrows buffer until exactly one completion callback. cancel requests that
 * callback; it does not release the buffer. Backend must enforce timeout_ms.
 * All functions and callbacks must run serially on the owner's event context. */
struct ft9366_usb_backend {
    bool (*submit)(void *context, uint64_t token, uint8_t endpoint,
                   uint8_t *buffer, size_t length, uint32_t timeout_ms);
    void (*cancel)(void *context, uint64_t token);
    void *context;
};
struct ft9366_transport {
    struct ft9366_usb_backend backend;
    enum ft9366_transport_state state, abort_reason;
    enum ft9366_direction direction;
    uint64_t token;
    size_t length;
    uint8_t buffer[FT9366_FRAME_BYTES];
};
/* Initialize exactly once per lifetime; never reinitialize while callbacks exist. */
enum ft9366_transport_result ft9366_transport_init(
    struct ft9366_transport *transport, const struct ft9366_usb_backend *backend);
/* WRITE copies data before submission; data must not overlap transport storage.
 * READ requires data == NULL. OK means accepted by this core; state may already
 * be IO_ERROR if backend submission failed. No automatic retry is performed. */
enum ft9366_transport_result ft9366_transport_start(
    struct ft9366_transport *transport, enum ft9366_direction direction,
    const uint8_t *data, size_t length, uint32_t timeout_ms);
/* Adapter reports DONE, CANCELLED, TIMEOUT, DISCONNECTED or IO_ERROR.
 * actual_length must match the request on success. Other lengths fail closed. */
enum ft9366_transport_result ft9366_transport_complete(
    struct ft9366_transport *transport, uint64_t token,
    enum ft9366_transport_state outcome, size_t actual_length);
/* Cancel/timeout/disconnect requests drain before their final state is published. */
enum ft9366_transport_result ft9366_transport_cancel(
    struct ft9366_transport *transport, enum ft9366_transport_state reason);
/* READ results only. Does not consume result; release erases it after copying.
 * Destination must not overlap transport storage. */
enum ft9366_transport_result ft9366_transport_read(
    const struct ft9366_transport *transport, uint8_t *destination, size_t capacity);
/* Legal only after submission failed or completion drained. Retains token counter. */
enum ft9366_transport_result ft9366_transport_release(struct ft9366_transport *transport);
#endif
