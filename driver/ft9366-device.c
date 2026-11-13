#include "ft9366-device.h"
#include "fpi-device.h"

struct ft9366_device {
    GUsbDevice *usb;
    struct ft9366_fprint_usb *adapter;
    enum ft9366_device_state state;
    ft9366_device_notify notify;
    void *user_data;
};

static void finish_close(struct ft9366_device *device)
{
    struct ft9366_transport *transport = ft9366_fprint_usb_transport(device->adapter);
    if (transport->state != FT9366_USB_IDLE)
        (void)ft9366_transport_release(transport);
    g_autoptr(GError) error = NULL;
    bool released = g_usb_device_release_interface(device->usb, 0,
        G_USB_DEVICE_CLAIM_INTERFACE_NONE, &error);
    /* NO_DEVICE means physical removal, so no live claim can remain. Other
     * release failures retain ownership and must be retried before reuse/free. */
    bool removed = g_error_matches(error, G_USB_DEVICE_ERROR, G_USB_DEVICE_ERROR_NO_DEVICE);
    device->state = released || removed ? FT9366_DEVICE_CLOSED : FT9366_DEVICE_RELEASE_FAILED;
    /* Final device access: owner may destroy the CLOSED object in notify. */
    device->notify(device, FT9366_DEVICE_CLOSE_DONE, error, device->user_data);
}

static void transfer_done(struct ft9366_fprint_usb *adapter, void *user_data)
{
    struct ft9366_device *device = user_data;
    (void)adapter;
    if (device->state == FT9366_DEVICE_CLOSING) {
        finish_close(device);
        return;
    }
    device->notify(device, FT9366_DEVICE_TRANSFER_DONE, NULL, device->user_data);
}

struct ft9366_device *ft9366_device_new(FpDevice *fp_device,
    ft9366_device_notify notify, void *user_data)
{
    if (fp_device == NULL || notify == NULL)
        return NULL;
    GUsbDevice *usb = fpi_device_get_usb_device(fp_device);
    if (usb == NULL)
        return NULL;
    struct ft9366_device *device = g_new0(struct ft9366_device, 1);
    device->usb = g_object_ref(usb);
    device->notify = notify;
    device->user_data = user_data;
    device->state = FT9366_DEVICE_CLOSED;
    device->adapter = ft9366_fprint_usb_new(fp_device, transfer_done, device);
    return device;
}

bool ft9366_device_claim(struct ft9366_device *device, GError **error)
{
    if (device == NULL || device->state != FT9366_DEVICE_CLOSED) {
        g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_BUSY, "FT9366 interface is not closed");
        return false;
    }
    if (!g_usb_device_claim_interface(device->usb, 0,
        G_USB_DEVICE_CLAIM_INTERFACE_NONE, error))
        return false;
    device->state = FT9366_DEVICE_CLAIMED;
    return true;
}

bool ft9366_device_close(struct ft9366_device *device, GError **error)
{
    if (device == NULL || (device->state != FT9366_DEVICE_CLAIMED &&
                          device->state != FT9366_DEVICE_RELEASE_FAILED)) {
        g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_BUSY, "FT9366 interface cannot close in this state");
        return false;
    }
    device->state = FT9366_DEVICE_CLOSING;
    struct ft9366_transport *transport = ft9366_fprint_usb_transport(device->adapter);
    if (transport->state == FT9366_USB_ACTIVE) {
        (void)ft9366_transport_cancel(transport, FT9366_USB_CANCELLED);
        return true;
    }
    if (transport->state == FT9366_USB_CANCELLING)
        return true;
    finish_close(device);
    return true;
}

enum ft9366_device_state ft9366_device_state(const struct ft9366_device *device)
{
    return device == NULL ? FT9366_DEVICE_CLOSED : device->state;
}

enum ft9366_transport_result ft9366_device_start(struct ft9366_device *device,
    enum ft9366_direction direction, const uint8_t *data, size_t length, uint32_t timeout_ms)
{
    if (device == NULL)
        return FT9366_USB_BAD_ARGUMENT;
    if (device->state != FT9366_DEVICE_CLAIMED)
        return FT9366_USB_BAD_STATE;
    return ft9366_transport_start(ft9366_fprint_usb_transport(device->adapter),
        direction, data, length, timeout_ms);
}

enum ft9366_transport_result ft9366_device_cancel(struct ft9366_device *device)
{
    if (device == NULL)
        return FT9366_USB_BAD_ARGUMENT;
    if (device->state != FT9366_DEVICE_CLAIMED)
        return FT9366_USB_BAD_STATE;
    return ft9366_transport_cancel(ft9366_fprint_usb_transport(device->adapter), FT9366_USB_CANCELLED);
}

enum ft9366_transport_state ft9366_device_result(const struct ft9366_device *device)
{
    if (device == NULL || device->state != FT9366_DEVICE_CLAIMED)
        return FT9366_USB_IO_ERROR;
    return ft9366_fprint_usb_transport(device->adapter)->state;
}

enum ft9366_transport_result ft9366_device_read(struct ft9366_device *device,
    uint8_t *destination, size_t capacity)
{
    if (device == NULL)
        return FT9366_USB_BAD_ARGUMENT;
    if (device->state != FT9366_DEVICE_CLAIMED)
        return FT9366_USB_BAD_STATE;
    return ft9366_transport_read(ft9366_fprint_usb_transport(device->adapter), destination, capacity);
}

enum ft9366_transport_result ft9366_device_release_result(struct ft9366_device *device)
{
    if (device == NULL)
        return FT9366_USB_BAD_ARGUMENT;
    if (device->state != FT9366_DEVICE_CLAIMED)
        return FT9366_USB_BAD_STATE;
    return ft9366_transport_release(ft9366_fprint_usb_transport(device->adapter));
}

bool ft9366_device_free(struct ft9366_device *device)
{
    if (device == NULL)
        return true;
    if (device->state != FT9366_DEVICE_CLOSED || !ft9366_fprint_usb_free(device->adapter))
        return false;
    g_clear_object(&device->usb);
    g_free(device);
    return true;
}
