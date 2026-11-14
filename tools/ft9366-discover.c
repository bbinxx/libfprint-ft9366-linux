/* libfprint discovery only: never opens the sensor or starts a biometric action. */
#include <fprint.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    g_autoptr(FpContext) context = fp_context_new();
    GPtrArray *devices = fp_context_get_devices(context);
    for (guint i = 0; i < devices->len; i++) {
        FpDevice *device = g_ptr_array_index(devices, i);
        if (strcmp(fp_device_get_driver(device), "ft9366") == 0) {
            printf("libfprint discovered: driver=%s; name=%s\n",
                   fp_device_get_driver(device), fp_device_get_name(device));
            puts("Discovery only: sensor not opened; no enrollment or verification performed.");
            return 0;
        }
    }
    fputs("No ft9366 device exposed by the loaded libfprint\n", stderr);
    return 1;
}
