#include "ft9366-fprint-usb.h"
#include "fpi-usb-transfer.h"

struct ft9366_fprint_usb {
    struct ft9366_transport transport;
    FpDevice *device;
    GCancellable *cancellable;
    ft9366_fprint_usb_done done;
    void *user_data;
    uint64_t pending_token;
    bool pending;
};

static enum ft9366_transport_state outcome_for_error(const GError *error)
{
    if (error == NULL)
        return FT9366_USB_DONE;
    if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED) ||
        g_error_matches(error, G_USB_DEVICE_ERROR, G_USB_DEVICE_ERROR_CANCELLED))
        return FT9366_USB_CANCELLED;
    if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_TIMED_OUT) ||
        g_error_matches(error, G_USB_DEVICE_ERROR, G_USB_DEVICE_ERROR_TIMED_OUT))
        return FT9366_USB_TIMEOUT;
    if (g_error_matches(error, G_USB_DEVICE_ERROR, G_USB_DEVICE_ERROR_NO_DEVICE))
        return FT9366_USB_DISCONNECTED;
    return FT9366_USB_IO_ERROR;
}

static void transfer_done(FpiUsbTransfer *transfer, FpDevice *device,
                           gpointer user_data, GError *error)
{
    struct ft9366_fprint_usb *adapter = user_data;
    enum ft9366_transport_state outcome = outcome_for_error(error);
    size_t actual = transfer->actual_length < 0 ? SIZE_MAX : (size_t)transfer->actual_length;
    (void)device;
    /* The USB callback has returned ownership of the borrowed buffer. */
    adapter->pending = false;
    g_clear_object(&adapter->cancellable);
    (void)ft9366_transport_complete(&adapter->transport, adapter->pending_token, outcome, actual);
    g_clear_error(&error);
    /* Final access: the owner may free adapter or submit another transfer here. */
    if (adapter->done != NULL)
        adapter->done(adapter, adapter->user_data);
}

static bool submit(void *context, uint64_t token, uint8_t endpoint,
                    uint8_t *buffer, size_t length, uint32_t timeout_ms)
{
    struct ft9366_fprint_usb *adapter = context;
    if (adapter->pending)
        return false;
    FpiUsbTransfer *transfer = fpi_usb_transfer_new(adapter->device);
    adapter->cancellable = g_cancellable_new();
    adapter->pending_token = token;
    adapter->pending = true;
    /* No destroy callback: transport owns storage until completion drains.
     * Leave short_is_error false so the core can distinguish LENGTH_ERROR. */
    fpi_usb_transfer_fill_bulk_full(transfer, endpoint, buffer, length, NULL);
    fpi_usb_transfer_submit(transfer, timeout_ms, adapter->cancellable, transfer_done, adapter);
    /* libfprint consumes transfer and unrefs it after invoking transfer_done. */
    return true;
}

static void cancel(void *context, uint64_t token)
{
    struct ft9366_fprint_usb *adapter = context;
    if (adapter->pending && token == adapter->pending_token)
        g_cancellable_cancel(adapter->cancellable);
}

struct ft9366_fprint_usb *
ft9366_fprint_usb_new(FpDevice *device, ft9366_fprint_usb_done done, void *user_data)
{
    if (device == NULL)
        return NULL;
    struct ft9366_fprint_usb *adapter = g_new0(struct ft9366_fprint_usb, 1);
    adapter->device = g_object_ref(device);
    adapter->done = done;
    adapter->user_data = user_data;
    const struct ft9366_usb_backend backend = {submit, cancel, adapter};
    (void)ft9366_transport_init(&adapter->transport, &backend);
    return adapter;
}

struct ft9366_transport *ft9366_fprint_usb_transport(struct ft9366_fprint_usb *adapter)
{
    return adapter == NULL ? NULL : &adapter->transport;
}

bool ft9366_fprint_usb_free(struct ft9366_fprint_usb *adapter)
{
    if (adapter == NULL)
        return true;
    if (adapter->pending)
        return false;
    /* Clear any retained read result before releasing adapter storage. */
    if (adapter->transport.state != FT9366_USB_IDLE)
        (void)ft9366_transport_release(&adapter->transport);
    g_clear_object(&adapter->cancellable);
    g_clear_object(&adapter->device);
    g_free(adapter);
    return true;
}
