#include "ft9366-transport.h"
#include <string.h>

static void wipe(struct ft9366_transport *transport)
{
    volatile uint8_t *buffer = transport->buffer;
    for (size_t i = 0; i < sizeof transport->buffer; i++)
        buffer[i] = 0;
}

static bool abort_reason(enum ft9366_transport_state state)
{
    return state == FT9366_USB_CANCELLED || state == FT9366_USB_TIMEOUT ||
           state == FT9366_USB_DISCONNECTED;
}

static bool terminal(enum ft9366_transport_state state)
{
    return state == FT9366_USB_DONE || abort_reason(state) ||
           state == FT9366_USB_IO_ERROR || state == FT9366_USB_LENGTH_ERROR;
}

enum ft9366_transport_result
ft9366_transport_init(struct ft9366_transport *transport,
                       const struct ft9366_usb_backend *backend)
{
    if (transport == NULL || backend == NULL ||
        backend->submit == NULL || backend->cancel == NULL)
        return FT9366_USB_BAD_ARGUMENT;
    transport->backend = *backend;
    transport->state = FT9366_USB_IDLE;
    transport->abort_reason = FT9366_USB_IDLE;
    transport->direction = FT9366_USB_READ;
    transport->token = 0;
    transport->length = 0;
    wipe(transport);
    return FT9366_USB_OK;
}

enum ft9366_transport_result
ft9366_transport_start(struct ft9366_transport *transport,
                        enum ft9366_direction direction, const uint8_t *data,
                        size_t length, uint32_t timeout_ms)
{
    if (transport == NULL || length == 0 || length > FT9366_FRAME_BYTES ||
        timeout_ms == 0 || (direction != FT9366_USB_READ && direction != FT9366_USB_WRITE) ||
        (direction == FT9366_USB_WRITE && data == NULL) ||
        (direction == FT9366_USB_READ && data != NULL))
        return FT9366_USB_BAD_ARGUMENT;
    if (transport->state != FT9366_USB_IDLE || transport->token == UINT64_MAX)
        return FT9366_USB_BAD_STATE;
    /* WRITE callers must not alias transport storage. */
    wipe(transport);
    if (direction == FT9366_USB_WRITE)
        memcpy(transport->buffer, data, length);
    transport->direction = direction;
    transport->length = length;
    transport->token++;
    transport->state = FT9366_USB_ACTIVE;
    if (!transport->backend.submit(transport->backend.context, transport->token,
                                    direction == FT9366_USB_READ ? 0x82 : 0x01,
                                    transport->buffer, length, timeout_ms)) {
        wipe(transport);
        transport->state = FT9366_USB_IO_ERROR;
    }
    /* OK means a valid operation was started. Inspect state for immediate failure. */
    return FT9366_USB_OK;
}

enum ft9366_transport_result
ft9366_transport_complete(struct ft9366_transport *transport, uint64_t token,
                           enum ft9366_transport_state outcome, size_t actual_length)
{
    if (transport == NULL)
        return FT9366_USB_BAD_ARGUMENT;
    if (token != transport->token ||
        (transport->state != FT9366_USB_ACTIVE && transport->state != FT9366_USB_CANCELLING))
        return FT9366_USB_STALE;
    if (outcome != FT9366_USB_DONE && outcome != FT9366_USB_IO_ERROR && !abort_reason(outcome))
        return FT9366_USB_BAD_ARGUMENT;
    if (transport->state == FT9366_USB_CANCELLING)
        transport->state = transport->abort_reason;
    else if (outcome == FT9366_USB_DONE && actual_length != transport->length)
        transport->state = FT9366_USB_LENGTH_ERROR;
    else
        transport->state = outcome;
    if (transport->state != FT9366_USB_DONE || transport->direction == FT9366_USB_WRITE)
        wipe(transport);
    return FT9366_USB_OK;
}

enum ft9366_transport_result
ft9366_transport_cancel(struct ft9366_transport *transport, enum ft9366_transport_state reason)
{
    if (transport == NULL || !abort_reason(reason))
        return FT9366_USB_BAD_ARGUMENT;
    if (transport->state != FT9366_USB_ACTIVE)
        return FT9366_USB_BAD_STATE;
    transport->abort_reason = reason;
    transport->state = FT9366_USB_CANCELLING;
    transport->backend.cancel(transport->backend.context, transport->token);
    return FT9366_USB_OK;
}

enum ft9366_transport_result
ft9366_transport_read(const struct ft9366_transport *transport, uint8_t *destination, size_t capacity)
{
    if (transport == NULL || destination == NULL)
        return FT9366_USB_BAD_ARGUMENT;
    if (transport->state != FT9366_USB_DONE || transport->direction != FT9366_USB_READ)
        return FT9366_USB_BAD_STATE;
    if (capacity < transport->length)
        return FT9366_USB_SMALL_OUTPUT;
    memcpy(destination, transport->buffer, transport->length);
    return FT9366_USB_OK;
}

enum ft9366_transport_result
ft9366_transport_release(struct ft9366_transport *transport)
{
    if (transport == NULL)
        return FT9366_USB_BAD_ARGUMENT;
    if (!terminal(transport->state))
        return FT9366_USB_BAD_STATE;
    wipe(transport);
    transport->length = 0;
    transport->abort_reason = FT9366_USB_IDLE;
    transport->state = FT9366_USB_IDLE;
    return FT9366_USB_OK;
}
