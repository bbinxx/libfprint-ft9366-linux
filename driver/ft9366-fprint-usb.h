#ifndef FT9366_FPRINT_USB_H
#define FT9366_FPRINT_USB_H
#include <fp-device.h>
#include "ft9366-transport.h"
struct ft9366_fprint_usb;
typedef void (*ft9366_fprint_usb_done)(struct ft9366_fprint_usb *adapter, void *user_data);
/* Does not open, claim or communicate with the device. Holds a device reference. */
struct ft9366_fprint_usb *ft9366_fprint_usb_new(FpDevice *device,
                                               ft9366_fprint_usb_done done, void *user_data);
struct ft9366_transport *ft9366_fprint_usb_transport(struct ft9366_fprint_usb *adapter);
/* Returns false while a transfer is pending; caller must cancel and drain first.
 * done runs after transport completion and may free the adapter or start another
 * transfer. All access must occur on the same serialized event context. */
bool ft9366_fprint_usb_free(struct ft9366_fprint_usb *adapter);
#endif
