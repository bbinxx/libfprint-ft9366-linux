/* Original connection diagnostic: no reset, detach or endpoint transfer. */
#include <libusb.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool expected_interface(libusb_device *device)
{
    struct libusb_config_descriptor *config = NULL;
    int result = libusb_get_active_config_descriptor(device, &config);
    if (result != LIBUSB_SUCCESS) {
        fprintf(stderr, "Read configuration: %s\n", libusb_error_name(result));
        return false;
    }
    bool found = false;
    for (unsigned i = 0; i < config->bNumInterfaces; i++) {
        const struct libusb_interface *interface = &config->interface[i];
        for (int j = 0; j < interface->num_altsetting; j++) {
            const struct libusb_interface_descriptor *alt = &interface->altsetting[j];
            if (alt->bInterfaceNumber != 0 || alt->bAlternateSetting != 0 ||
                alt->bInterfaceClass != 0xff || alt->bNumEndpoints != 2)
                continue;
            bool input = false, output = false;
            for (unsigned k = 0; k < alt->bNumEndpoints; k++) {
                const struct libusb_endpoint_descriptor *ep = &alt->endpoint[k];
                if ((ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) != LIBUSB_TRANSFER_TYPE_BULK ||
                    ep->wMaxPacketSize != 512)
                    continue;
                input |= ep->bEndpointAddress == 0x82;
                output |= ep->bEndpointAddress == 0x01;
            }
            found = input && output;
        }
    }
    libusb_free_config_descriptor(config);
    if (!found)
        fprintf(stderr, "Refusing connection: interface layout differs from observed hardware\n");
    return found;
}

int main(int argc, char **argv)
{
    (void)setvbuf(stdout, NULL, _IOLBF, 0);
    bool connect = argc == 2 && strcmp(argv[1], "--connect") == 0;
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        puts("Usage: ft9366-connect [--connect]\n"
             "Default: descriptor inspection only. --connect: open, claim/release interface 0, close.\n"
             "Never resets, detaches a kernel driver or sends endpoint/vendor transfers.");
        return 0;
    }
    if (argc != 1 && !connect) {
        fprintf(stderr, "Usage: ft9366-connect [--connect]\n");
        return 2;
    }
    libusb_context *context = NULL;
    libusb_device **devices = NULL;
    libusb_device *target = NULL;
    libusb_device_handle *handle = NULL;
    int result = libusb_init_context(&context, NULL, 0);
    if (result != LIBUSB_SUCCESS) {
        fprintf(stderr, "USB initialization: %s\n", libusb_error_name(result));
        return 1;
    }
    int exit_status = 1;
    ssize_t count = libusb_get_device_list(context, &devices);
    if (count < 0) {
        fprintf(stderr, "USB enumeration: %s\n", libusb_error_name((int)count));
        goto cleanup;
    }
    unsigned matches = 0;
    for (ssize_t i = 0; i < count; i++) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(devices[i], &desc) == LIBUSB_SUCCESS &&
            desc.idVendor == 0x2808 && desc.idProduct == 0xa658) {
            target = devices[i]; matches++;
        }
    }
    if (matches != 1) {
        fprintf(stderr, "Expected exactly one 2808:a658 device, found %u\n", matches);
        goto cleanup;
    }
    printf("Found 2808:a658 on bus %03u device %03u\n",
           libusb_get_bus_number(target), libusb_get_device_address(target));
    if (!expected_interface(target))
        goto cleanup;
    puts("Interface 0: bulk OUT 0x01 / IN 0x82, 512-byte packets verified");
    if (!connect) {
        puts("Descriptor inspection complete; no handle opened");
        exit_status = 0;
        goto cleanup;
    }
    result = libusb_open(target, &handle);
    if (result != LIBUSB_SUCCESS) {
        fprintf(stderr, "Open: %s\n", libusb_error_name(result));
        goto cleanup;
    }
    puts("USB handle opened");
    result = libusb_kernel_driver_active(handle, 0);
    if (result == 1) {
        fprintf(stderr, "Interface 0 has a kernel driver; refusing to detach it\n");
        goto cleanup;
    }
    if (result < 0 && result != LIBUSB_ERROR_NOT_SUPPORTED) {
        fprintf(stderr, "Kernel-driver check: %s\n", libusb_error_name(result));
        goto cleanup;
    }
    result = libusb_claim_interface(handle, 0);
    if (result != LIBUSB_SUCCESS) {
        fprintf(stderr, "Claim interface 0: %s\n", libusb_error_name(result));
        goto cleanup;
    }
    puts("Interface 0 claimed successfully");
    result = libusb_release_interface(handle, 0);
    if (result != LIBUSB_SUCCESS) {
        fprintf(stderr, "Release interface 0: %s\n", libusb_error_name(result));
        goto cleanup;
    }
    puts("Interface 0 released successfully");
    exit_status = 0;
cleanup:
    if (handle != NULL) {
        libusb_close(handle);
        puts("USB handle closed");
    }
    if (devices != NULL)
        libusb_free_device_list(devices, 1);
    libusb_exit(context);
    return exit_status;
}
