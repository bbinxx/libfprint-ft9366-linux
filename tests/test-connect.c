/* Real libusb declarations, simulated USB functions. No physical I/O. */
#include <libusb.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
int ft9366_probe_main(int argc, char **argv);
static int tag;
static int reads, releases, closes, opens, claims;
static int read_result, open_result, kernel_active;
static struct libusb_endpoint_descriptor endpoints[] = {
    {.bEndpointAddress = 0x01, .bmAttributes = LIBUSB_TRANSFER_TYPE_BULK, .wMaxPacketSize = 512},
    {.bEndpointAddress = 0x82, .bmAttributes = LIBUSB_TRANSFER_TYPE_BULK, .wMaxPacketSize = 512}
};
static struct libusb_interface_descriptor alt = {
    .bInterfaceNumber = 0, .bAlternateSetting = 0, .bInterfaceClass = 0xff,
    .bNumEndpoints = 2, .endpoint = endpoints
};
static struct libusb_interface interface = {.altsetting = &alt, .num_altsetting = 1};
static struct libusb_config_descriptor config = {.bNumInterfaces = 1, .interface = &interface};
int LIBUSB_CALL libusb_init_context(libusb_context **ctx, const struct libusb_init_option options[], int count)
{ (void)options; (void)count; *ctx = (libusb_context *)&tag; return 0; }
ssize_t LIBUSB_CALL libusb_get_device_list(libusb_context *ctx, libusb_device ***list)
{
    (void)ctx;
    static libusb_device *devices[2];
    devices[0] = (libusb_device *)&tag; devices[1] = NULL; *list = devices; return 1;
}
int LIBUSB_CALL libusb_get_device_descriptor(libusb_device *dev, struct libusb_device_descriptor *desc)
{ (void)dev; memset(desc, 0, sizeof *desc); desc->idVendor = 0x2808; desc->idProduct = 0xa658; return 0; }
int LIBUSB_CALL libusb_get_active_config_descriptor(libusb_device *dev, struct libusb_config_descriptor **out)
{ (void)dev; *out = &config; return 0; }
void LIBUSB_CALL libusb_free_config_descriptor(struct libusb_config_descriptor *desc) { (void)desc; }
uint8_t LIBUSB_CALL libusb_get_bus_number(libusb_device *dev) { (void)dev; return 3; }
uint8_t LIBUSB_CALL libusb_get_device_address(libusb_device *dev) { (void)dev; return 7; }
int LIBUSB_CALL libusb_open(libusb_device *dev, libusb_device_handle **handle)
{ (void)dev; opens++; if (open_result == 0) *handle = (libusb_device_handle *)&tag; return open_result; }
int LIBUSB_CALL libusb_kernel_driver_active(libusb_device_handle *handle, int iface)
{ (void)handle; assert(iface == 0); return kernel_active; }
int LIBUSB_CALL libusb_claim_interface(libusb_device_handle *handle, int iface)
{ (void)handle; assert(iface == 0); claims++; return 0; }
int LIBUSB_CALL libusb_bulk_transfer(libusb_device_handle *handle, unsigned char endpoint,
    unsigned char *data, int length, int *actual, unsigned int timeout)
{
    (void)handle;
    assert(endpoint == 0x82 && length == 512 && timeout == 1000);
    reads++; memset(data, 0xa5, (size_t)length); *actual = read_result == 0 ? length : 0;
    return read_result;
}
int LIBUSB_CALL libusb_release_interface(libusb_device_handle *handle, int iface)
{ (void)handle; assert(iface == 0); releases++; return 0; }
void LIBUSB_CALL libusb_close(libusb_device_handle *handle) { (void)handle; closes++; }
void LIBUSB_CALL libusb_free_device_list(libusb_device **list, int unref) { (void)list; (void)unref; }
void LIBUSB_CALL libusb_exit(libusb_context *ctx) { (void)ctx; }
int main(void)
{
    char *read_args[] = {"probe", "--read", NULL};
    read_result = LIBUSB_ERROR_TIMEOUT;
    assert(ft9366_probe_main(2, read_args) == 1);
    assert(reads == 1 && claims == 1 && releases == 1 && closes == 1);
    read_result = 0;
    assert(ft9366_probe_main(2, read_args) == 0);
    assert(reads == 2 && releases == 2 && closes == 2);
    read_result = LIBUSB_ERROR_OVERFLOW;
    assert(ft9366_probe_main(2, read_args) == 1);
    assert(reads == 3 && releases == 3 && closes == 3);
    kernel_active = 1;
    assert(ft9366_probe_main(2, read_args) == 1);
    assert(reads == 3 && claims == 3 && closes == 4);
    kernel_active = 0; open_result = LIBUSB_ERROR_ACCESS;
    assert(ft9366_probe_main(2, read_args) == 1);
    assert(reads == 3 && claims == 3 && closes == 4);
    open_result = 0;
    char *connect_args[] = {"probe", "--connect", NULL};
    assert(ft9366_probe_main(2, connect_args) == 0);
    assert(reads == 3 && releases == 4 && closes == 5);
    int before = opens;
    char *inspect_args[] = {"probe", NULL};
    assert(ft9366_probe_main(1, inspect_args) == 0);
    assert(opens == before);
    puts("connection probe: simulated read/cleanup checks passed");
}
