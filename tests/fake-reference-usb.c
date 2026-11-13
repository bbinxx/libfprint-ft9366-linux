/* Simulation-only symbols; never linked into the hardware executable. */
#include <glib.h>
#include <libusb.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int tag;
static bool opened,claimed;
static unsigned transfers;
static gint64 clock_us;
static struct libusb_endpoint_descriptor endpoints[]={
    {.bEndpointAddress=0x01,.bmAttributes=2,.wMaxPacketSize=512},
    {.bEndpointAddress=0x82,.bmAttributes=2,.wMaxPacketSize=512}
};
static struct libusb_interface_descriptor alt={.bInterfaceNumber=0,.bInterfaceClass=0xff,.bNumEndpoints=2,.endpoint=endpoints};
static struct libusb_interface interface={.altsetting=&alt,.num_altsetting=1};
static struct libusb_config_descriptor config={.bNumInterfaces=1,.interface=&interface};
static bool scenario(const char *name)
{ const char *value=getenv("FT9366_SIMULATION_CASE");return value && strcmp(value,name)==0; }
gint64 g_get_monotonic_time(void) { clock_us+=1000;return clock_us; }
void g_usleep(gulong duration) { clock_us+=(gint64)duration; }
int LIBUSB_CALL libusb_init_context(libusb_context **ctx,const struct libusb_init_option options[],int count)
{ (void)options;(void)count;puts("SIMULATED USB BACKEND -- NO HARDWARE");*ctx=(libusb_context *)&tag;return 0; }
ssize_t LIBUSB_CALL libusb_get_device_list(libusb_context *ctx,libusb_device ***out)
{ (void)ctx;static libusb_device *list[2];list[0]=(libusb_device *)&tag;*out=list;return 1; }
int LIBUSB_CALL libusb_get_device_descriptor(libusb_device *dev,struct libusb_device_descriptor *desc)
{ (void)dev;memset(desc,0,sizeof *desc);desc->idVendor=0x2808;desc->idProduct=0xa658;return 0; }
int LIBUSB_CALL libusb_get_active_config_descriptor(libusb_device *dev,struct libusb_config_descriptor **out)
{ (void)dev;*out=&config;return 0; }
void LIBUSB_CALL libusb_free_config_descriptor(struct libusb_config_descriptor *desc) { (void)desc; }
int LIBUSB_CALL libusb_open(libusb_device *dev,libusb_device_handle **out)
{ (void)dev;opened=true;*out=(libusb_device_handle *)&tag;return 0; }
int LIBUSB_CALL libusb_kernel_driver_active(libusb_device_handle *handle,int iface)
{ (void)handle;assert(iface==0);return 0; }
int LIBUSB_CALL libusb_reset_device(libusb_device_handle *handle)
{ (void)handle;assert(opened && !claimed);return 0; }
int LIBUSB_CALL libusb_claim_interface(libusb_device_handle *handle,int iface)
{ (void)handle;assert(iface==0 && opened && !claimed);claimed=true;return 0; }
int LIBUSB_CALL libusb_bulk_transfer(libusb_device_handle *handle,unsigned char ep,
    unsigned char *data,int length,int *actual,unsigned timeout)
{
    (void)handle;assert(claimed && length>0 && length<=10240 && timeout>0 && timeout<=3000);
    assert(ep==0x01 || ep==0x82);transfers++;
    if(scenario("disconnect") && transfers==2) { *actual=0;return LIBUSB_ERROR_NO_DEVICE; }
    *actual=length;
    if(ep==0x82) {
        memset(data,0,(size_t)length);
        if(length==2 && !scenario("no-finger")) data[1]=0x02;
        if(scenario("short") && transfers==2) *actual=length-1;
    }
    return 0;
}
int LIBUSB_CALL libusb_release_interface(libusb_device_handle *handle,int iface)
{ (void)handle;assert(iface==0 && claimed);claimed=false;return 0; }
void LIBUSB_CALL libusb_close(libusb_device_handle *handle)
{ (void)handle;assert(opened && !claimed);opened=false; }
void LIBUSB_CALL libusb_free_device_list(libusb_device **list,int unref) { (void)list;(void)unref; }
void LIBUSB_CALL libusb_exit(libusb_context *ctx) { (void)ctx;assert(!opened && !claimed); }
