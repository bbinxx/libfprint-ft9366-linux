/* Lifecycle test doubles use real libfprint/libgusb declarations. No USB I/O.
 * Plain GObjects stand in for references; no real FpDevice methods are invoked. */
#include "ft9366-device.h"
#include "fpi-usb-transfer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static GObject *fake_usb;
static FpiUsbTransfer *pending;
static GCancellable *cancellable;
static unsigned claims, releases, transfers, closes;
static bool claim_ok = true, release_ok = true, free_in_callback;
static gint release_error = G_USB_DEVICE_ERROR_IO;
static bool close_error;
GUsbDevice *fpi_device_get_usb_device(FpDevice *device)
{ (void)device; return (GUsbDevice *)fake_usb; }
gboolean g_usb_device_claim_interface(GUsbDevice *usb, gint interface,
    GUsbDeviceClaimInterfaceFlags flags, GError **error)
{
    assert(usb == (GUsbDevice *)fake_usb && interface == 0 && flags == 0);
    claims++;
    if (!claim_ok) g_set_error_literal(error, G_USB_DEVICE_ERROR, G_USB_DEVICE_ERROR_BUSY, "synthetic claim failure");
    return claim_ok;
}
gboolean g_usb_device_release_interface(GUsbDevice *usb, gint interface,
    GUsbDeviceClaimInterfaceFlags flags, GError **error)
{
    assert(usb == (GUsbDevice *)fake_usb && interface == 0 && flags == 0);
    assert(pending == NULL); /* Never release while a transfer owns its buffer. */
    releases++;
    if (!release_ok) g_set_error_literal(error, G_USB_DEVICE_ERROR, release_error, "synthetic release failure");
    return release_ok;
}
FpiUsbTransfer *fpi_usb_transfer_new(FpDevice *device)
{
    FpiUsbTransfer *transfer = g_new0(FpiUsbTransfer, 1);
    transfer->device = g_object_ref(device);
    return transfer;
}
void fpi_usb_transfer_fill_bulk_full(FpiUsbTransfer *transfer, guint8 endpoint,
    guint8 *buffer, gsize length, GDestroyNotify destroy)
{
    transfer->buffer = buffer; transfer->length = (gssize)length;
    transfer->endpoint = endpoint; transfer->free_buffer = destroy;
}
void fpi_usb_transfer_submit(FpiUsbTransfer *transfer, guint timeout,
    GCancellable *cancel, FpiUsbTransferCallback callback, gpointer user_data)
{
    assert(pending == NULL && timeout > 0);
    pending = transfer; cancellable = g_object_ref(cancel);
    transfer->callback = callback; transfer->user_data = user_data;
}
/* Enforce that driver lifecycle never duplicates libfprint's handle ownership. */
gboolean g_usb_device_open(GUsbDevice *usb, GError **error)
{ (void)usb; (void)error; assert(!"lifecycle must not open USB handle"); return FALSE; }
gboolean g_usb_device_close(GUsbDevice *usb, GError **error)
{ (void)usb; (void)error; assert(!"lifecycle must not close USB handle"); return FALSE; }
gboolean g_usb_device_reset(GUsbDevice *usb, GError **error)
{ (void)usb; (void)error; assert(!"unreviewed reset is forbidden"); return FALSE; }
static void finish(GError *error)
{
    FpiUsbTransfer *transfer = pending;
    assert(transfer != NULL);
    pending = NULL;
    g_clear_object(&cancellable);
    transfer->actual_length = error ? -1 : transfer->length;
    transfer->callback(transfer, transfer->device, transfer->user_data, error);
    assert(transfer->free_buffer == NULL);
    g_object_unref(transfer->device);
    g_free(transfer);
}
static void notify(struct ft9366_device *device, enum ft9366_device_event event,
    const GError *error, void *user_data)
{
    assert(user_data == &closes);
    if (event == FT9366_DEVICE_TRANSFER_DONE) {
        assert(error == NULL); /* Transfer outcome is queried from transport state. */
        transfers++;
    } else {
        closes++;
        close_error = error != NULL;
        if (free_in_callback) assert(ft9366_device_free(device));
    }
}
int main(void)
{
    GObject *fp = g_object_new(G_TYPE_OBJECT, NULL);
    fake_usb = g_object_new(G_TYPE_OBJECT, NULL);
    gpointer weak_fp = fp, weak_usb = fake_usb;
    g_object_add_weak_pointer(fp, &weak_fp);
    g_object_add_weak_pointer(fake_usb, &weak_usb);
    assert(ft9366_device_new(NULL, notify, NULL) == NULL);
    assert(ft9366_device_new((FpDevice *)fp, NULL, NULL) == NULL);
    struct ft9366_device *device = ft9366_device_new((FpDevice *)fp, notify, &closes);
    assert(device != NULL && claims == 0 && releases == 0);
    assert(ft9366_device_start(device, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_BAD_STATE);
    GError *error = NULL;
    claim_ok = false;
    assert(!ft9366_device_claim(device, &error));
    assert(g_error_matches(error, G_USB_DEVICE_ERROR, G_USB_DEVICE_ERROR_BUSY));
    g_clear_error(&error);
    assert(ft9366_device_state(device) == FT9366_DEVICE_CLOSED && releases == 0);
    claim_ok = true;
    assert(ft9366_device_claim(device, &error));
    assert(ft9366_device_state(device) == FT9366_DEVICE_CLAIMED);
    assert(!ft9366_device_claim(device, &error)); g_clear_error(&error);
    assert(claims == 2 && !ft9366_device_free(device));
    assert(ft9366_device_start(device, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
    memcpy(pending->buffer, "test", 4);
    finish(NULL);
    uint8_t out[4];
    assert(transfers == 1 && ft9366_device_result(device) == FT9366_USB_DONE);
    assert(ft9366_device_read(device, out, 4) == FT9366_USB_OK && memcmp(out, "test", 4) == 0);
    assert(ft9366_device_release_result(device) == FT9366_USB_OK);
    /* Close waits for cancellation and rejects all new operations. */
    assert(ft9366_device_start(device, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
    assert(ft9366_device_close(device, &error));
    assert(ft9366_device_state(device) == FT9366_DEVICE_CLOSING);
    assert(g_cancellable_is_cancelled(cancellable) && releases == 0 && closes == 0);
    assert(!ft9366_device_free(device));
    assert(!ft9366_device_close(device, &error)); g_clear_error(&error);
    assert(ft9366_device_start(device, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_BAD_STATE);
    finish(NULL); /* Successful callback raced with cancellation. */
    assert(releases == 1 && closes == 1 && !close_error && transfers == 1);
    assert(ft9366_device_state(device) == FT9366_DEVICE_CLOSED);
    assert(ft9366_device_read(device, out, 4) == FT9366_USB_BAD_STATE);
    /* Release failure retains ownership and blocks reopen/destruction until retried. */
    assert(ft9366_device_claim(device, &error));
    release_ok = false;
    assert(ft9366_device_close(device, &error));
    assert(close_error && ft9366_device_state(device) == FT9366_DEVICE_RELEASE_FAILED);
    assert(!ft9366_device_free(device));
    assert(!ft9366_device_claim(device, &error)); g_clear_error(&error);
    release_ok = true;
    assert(ft9366_device_close(device, &error));
    assert(!close_error && ft9366_device_state(device) == FT9366_DEVICE_CLOSED);
    /* An already-cancelling operation is drained once rather than cancelled twice. */
    assert(ft9366_device_claim(device, &error));
    assert(ft9366_device_start(device, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
    assert(ft9366_device_cancel(device) == FT9366_USB_OK);
    unsigned before = releases;
    assert(ft9366_device_close(device, &error));
    assert(releases == before);
    finish(g_error_new_literal(G_IO_ERROR, G_IO_ERROR_CANCELLED, "synthetic cancellation"));
    assert(releases == before + 1 && ft9366_device_state(device) == FT9366_DEVICE_CLOSED);
    /* Physical removal means no interface remains to release; still report error. */
    assert(ft9366_device_claim(device, &error));
    release_ok = false; release_error = G_USB_DEVICE_ERROR_NO_DEVICE;
    assert(ft9366_device_close(device, &error));
    assert(close_error && ft9366_device_state(device) == FT9366_DEVICE_CLOSED);
    release_ok = true;
    assert(ft9366_device_claim(device, &error));
    free_in_callback = true;
    assert(ft9366_device_close(device, &error)); /* Callback destroys device. */
    assert(!error);
    /* Nested completion -> close -> owner destruction must not touch freed memory. */
    device = ft9366_device_new((FpDevice *)fp, notify, &closes);
    assert(ft9366_device_claim(device, &error));
    assert(ft9366_device_start(device, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_OK);
    assert(ft9366_device_close(device, &error));
    finish(NULL);
    /* Missing USB object is rejected without claiming or allocating a session. */
    GObject *saved_usb = fake_usb;
    fake_usb = NULL;
    assert(ft9366_device_new((FpDevice *)fp, notify, &closes) == NULL);
    fake_usb = saved_usb;
    assert(ft9366_device_cancel(NULL) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_device_start(NULL, FT9366_USB_READ, NULL, 4, 100) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_device_read(NULL, out, 4) == FT9366_USB_BAD_ARGUMENT);
    assert(ft9366_device_release_result(NULL) == FT9366_USB_BAD_ARGUMENT);
    g_object_unref(fp); g_object_unref(fake_usb);
    assert(weak_fp == NULL && weak_usb == NULL);
    assert(ft9366_device_free(NULL));
    puts("device lifecycle: all checks passed (simulated USB only)");
}
