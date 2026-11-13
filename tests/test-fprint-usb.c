/* Real libfprint header/GLib types; simulated FpiUsbTransfer functions only.
 * A plain GObject stands in for the opaque device; no FpDevice methods are used. */
#include "ft9366-fprint-usb.h"
#include "fpi-usb-transfer.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
static FpiUsbTransfer *pending;
static GCancellable *pending_cancel;
static guint submitted_timeout;
static unsigned notifications;
FpiUsbTransfer *fpi_usb_transfer_new(FpDevice *device)
{
    FpiUsbTransfer *transfer = g_new0(FpiUsbTransfer, 1);
    transfer->device = g_object_ref(device);
    return transfer;
}
void fpi_usb_transfer_fill_bulk_full(FpiUsbTransfer *transfer, guint8 endpoint,
                                    guint8 *buffer, gsize length, GDestroyNotify free_func)
{
    transfer->endpoint = endpoint; transfer->buffer = buffer;
    transfer->length = (gssize)length; transfer->free_buffer = free_func;
}
void fpi_usb_transfer_submit(FpiUsbTransfer *transfer, guint timeout_ms,
                             GCancellable *cancellable, FpiUsbTransferCallback callback,
                             gpointer user_data)
{
    assert(pending == NULL);
    pending = transfer; pending_cancel = g_object_ref(cancellable);
    submitted_timeout = timeout_ms;
    transfer->callback = callback; transfer->user_data = user_data;
}
static void finish(gssize actual, GError *error)
{
    FpiUsbTransfer *transfer = pending;
    assert(transfer != NULL);
    pending = NULL;
    g_clear_object(&pending_cancel);
    transfer->actual_length = actual;
    transfer->callback(transfer, transfer->device, transfer->user_data, error);
    /* Match libfprint's transfer-full submit and post-callback unref. */
    assert(transfer->free_buffer == NULL);
    g_object_unref(transfer->device);
    g_free(transfer);
}
static void done(struct ft9366_fprint_usb *adapter, void *user_data)
{
    assert(user_data == &notifications);
    assert(ft9366_fprint_usb_transport(adapter)->state != FT9366_USB_ACTIVE);
    notifications++;
}
static void free_when_done(struct ft9366_fprint_usb *adapter, void *user_data)
{
    bool *freed = user_data;
    *freed = ft9366_fprint_usb_free(adapter);
}
int main(void)
{
    GObject *device = g_object_new(G_TYPE_OBJECT, NULL);
    assert(ft9366_fprint_usb_new(NULL, done, NULL) == NULL);
    struct ft9366_fprint_usb *adapter = ft9366_fprint_usb_new((FpDevice *)device, done, &notifications);
    assert(adapter != NULL);
    struct ft9366_transport *transport = ft9366_fprint_usb_transport(adapter);
    assert(ft9366_transport_start(transport, FT9366_USB_READ, NULL, 4, 321) == FT9366_USB_OK);
    assert(pending != NULL && pending->endpoint == 0x82 && pending->length == 4);
    assert(submitted_timeout == 321 && !g_cancellable_is_cancelled(pending_cancel));
    assert(!ft9366_fprint_usb_free(adapter));
    memcpy(pending->buffer, "test", 4);
    finish(4, NULL);
    uint8_t result[4];
    assert(ft9366_transport_read(transport, result, 4) == FT9366_USB_OK);
    assert(memcmp(result, "test", 4) == 0 && notifications == 1);
    assert(ft9366_transport_release(transport) == FT9366_USB_OK);
    const uint8_t command[] = {0x12, 0x34}; /* Synthetic only. */
    assert(ft9366_transport_start(transport, FT9366_USB_WRITE, command, 2, 100) == FT9366_USB_OK);
    assert(pending->endpoint == 0x01 && memcmp(pending->buffer, command, 2) == 0);
    assert(ft9366_transport_cancel(transport, FT9366_USB_CANCELLED) == FT9366_USB_OK);
    assert(g_cancellable_is_cancelled(pending_cancel));
    assert(!ft9366_fprint_usb_free(adapter));
    finish(-1, g_error_new_literal(G_IO_ERROR, G_IO_ERROR_CANCELLED, "synthetic cancellation"));
    assert(transport->state == FT9366_USB_CANCELLED && notifications == 2);
    assert(ft9366_transport_release(transport) == FT9366_USB_OK);
    const struct { gint code; enum ft9366_transport_state expected; } errors[] = {
        {G_USB_DEVICE_ERROR_TIMED_OUT, FT9366_USB_TIMEOUT},
        {G_USB_DEVICE_ERROR_NO_DEVICE, FT9366_USB_DISCONNECTED},
        {G_USB_DEVICE_ERROR_CANCELLED, FT9366_USB_CANCELLED},
        {G_USB_DEVICE_ERROR_IO, FT9366_USB_IO_ERROR}
    };
    for (size_t i = 0; i < sizeof errors / sizeof errors[0]; i++) {
        assert(ft9366_transport_start(transport, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
        assert(!g_cancellable_is_cancelled(pending_cancel));
        finish(-1, g_error_new_literal(G_USB_DEVICE_ERROR, errors[i].code, "synthetic USB error"));
        assert(transport->state == errors[i].expected);
        assert(ft9366_transport_release(transport) == FT9366_USB_OK);
    }
    const gssize lengths[] = {-1, 0, 3, 5};
    for (size_t i = 0; i < sizeof lengths / sizeof lengths[0]; i++) {
        assert(ft9366_transport_start(transport, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
        finish(lengths[i], NULL);
        assert(transport->state == FT9366_USB_LENGTH_ERROR);
        assert(ft9366_transport_release(transport) == FT9366_USB_OK);
    }
    /* Successful completion racing with cancellation must still discard data. */
    assert(ft9366_transport_start(transport, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
    assert(ft9366_transport_cancel(transport, FT9366_USB_CANCELLED) == FT9366_USB_OK);
    finish(4, NULL);
    assert(transport->state == FT9366_USB_CANCELLED);
    assert(ft9366_transport_release(transport) == FT9366_USB_OK);
    assert(ft9366_transport_start(transport, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
    finish(-1, g_error_new_literal(G_IO_ERROR, G_IO_ERROR_TIMED_OUT, "synthetic I/O timeout"));
    assert(transport->state == FT9366_USB_TIMEOUT);
    assert(ft9366_transport_release(transport) == FT9366_USB_OK);
    assert(ft9366_fprint_usb_free(adapter));
    bool freed = false;
    adapter = ft9366_fprint_usb_new((FpDevice *)device, free_when_done, &freed);
    assert(ft9366_transport_start(ft9366_fprint_usb_transport(adapter), FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
    finish(4, NULL);
    assert(freed);
    g_object_unref(device);
    assert(ft9366_fprint_usb_free(NULL));
    assert(ft9366_fprint_usb_transport(NULL) == NULL);
    puts("libfprint adapter: all checks passed (simulated USB only)");
}
