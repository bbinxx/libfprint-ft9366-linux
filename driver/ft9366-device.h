#ifndef FT9366_DEVICE_H
#define FT9366_DEVICE_H
#include "ft9366-fprint-usb.h"
struct ft9366_device;
enum ft9366_device_state {
    FT9366_DEVICE_CLOSED, FT9366_DEVICE_CLAIMED,
    FT9366_DEVICE_CLOSING, FT9366_DEVICE_RELEASE_FAILED
};
enum ft9366_device_event { FT9366_DEVICE_TRANSFER_DONE, FT9366_DEVICE_CLOSE_DONE };
/* error is borrowed for the duration of notify. Callback may destroy a CLOSED
 * device. Calls and notifications must be serialized on the device event context. */
typedef void (*ft9366_device_notify)(struct ft9366_device *device,
                                     enum ft9366_device_event event,
                                     const GError *error, void *user_data);
/* Requires a valid FpDevice whose USB handle libfprint manages. No USB operation. */
struct ft9366_device *ft9366_device_new(FpDevice *device, ft9366_device_notify notify, void *user_data);
/* Claims interface 0 without reset or kernel-driver detachment. Does not init sensor. */
bool ft9366_device_claim(struct ft9366_device *device, GError **error);
/* Returns accepted/rejected. On acceptance CLOSE_DONE may fire before return or
 * after cancellation drains. Check callback error and state for release failure.
 * It never closes the USB handle; libfprint owns that operation. */
bool ft9366_device_close(struct ft9366_device *device, GError **error);
enum ft9366_device_state ft9366_device_state(const struct ft9366_device *device);
enum ft9366_transport_result ft9366_device_start(struct ft9366_device *device,
    enum ft9366_direction direction, const uint8_t *data, size_t length, uint32_t timeout_ms);
enum ft9366_transport_result ft9366_device_cancel(struct ft9366_device *device);
enum ft9366_transport_state ft9366_device_result(const struct ft9366_device *device);
enum ft9366_transport_result ft9366_device_read(struct ft9366_device *device,
    uint8_t *destination, size_t capacity);
enum ft9366_transport_result ft9366_device_release_result(struct ft9366_device *device);
/* Refuses while interface is claimed or callbacks are pending. Close and drain first. */
bool ft9366_device_free(struct ft9366_device *device);
#endif
