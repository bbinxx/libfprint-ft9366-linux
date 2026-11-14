/* Original experiment runner using externally supplied, attributed reference data.
 * This synchronous diagnostic is not the original libfprint driver. */
#include <glib.h>
#include <libusb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "ft9366-replay.h"
#include "ft9366_script.h"

static volatile sig_atomic_t interrupted;
static gint64 deadline;
static void on_signal(int signal_number) { (void)signal_number; interrupted=1; }
static void clear_bytes(void *buffer,size_t size)
{ volatile unsigned char *p=buffer; while(size--) *p++=0; }
struct response { unsigned frames; uint16_t flag; size_t last_length; };
struct script { const char *name; const guint8 *data; size_t size; };
static const struct script scripts[] = {
    {"init",ft9366_init_script,sizeof ft9366_init_script},
    {"idle",ft9366_idle_script,sizeof ft9366_idle_script},
    {"scan",ft9366_scan_script,sizeof ft9366_scan_script}
};
static bool replay(libusb_device_handle *handle,const struct script *script,struct response *response)
{
    uint8_t reply[10240]={0};
    struct ft9366_record record;
    size_t offset=0,index=0;
    *response=(struct response){0};
    while(ft9366_replay_next(script->data,script->size,&offset,&record)==1) {
        index++;
        gint64 remaining=deadline-g_get_monotonic_time();
        if(interrupted || remaining<=0) { fprintf(stderr,"Stopped: cancelled or total deadline reached\n");goto failed; }
        if(record.operation==2) {
            if((gint64)record.length*1000>remaining) goto failed;
            g_usleep((gulong)record.length*1000);
            continue;
        }
        unsigned timeout=(unsigned)MIN((gint64)3000,MAX((gint64)1,remaining/1000));
        int actual=0;
        if(record.operation==0) memcpy(reply,record.payload,record.length);
        int result=libusb_bulk_transfer(handle,record.operation==0?0x01:0x82,
            reply,record.length,&actual,timeout);
        if(result!=0 || actual!=record.length) {
            fprintf(stderr,"%s record %zu: %s; %d/%u bytes\n",script->name,index,
                libusb_error_name(result),actual,record.length);
            goto failed;
        }
        if(record.operation==1) {
            response->last_length=record.length;
            if(record.length==10240) response->frames++;
            if(record.length==2) response->flag=(uint16_t)((unsigned)reply[0]*256u+reply[1]);
        }
        clear_bytes(reply,sizeof reply);
    }
    return true;
failed:
    clear_bytes(reply,sizeof reply);
    return false;
}
static void manifest(void)
{
    puts("REFERENCE EXPERIMENT ONLY: reset once, claim interface 0, init, idle poll, two scans, release/close.");
    puts("Poll cap: 40 idle cycles / 10 seconds; replay deadline: 60 seconds; transfer timeout: <=3000 ms.");
    puts("All received payloads are discarded. No images, templates or payload hashes are saved.");
    for(size_t i=0;i<G_N_ELEMENTS(scripts);i++) {
        size_t offset=0,index=0; struct ft9366_record record;
        printf("\nSCRIPT %s\n",scripts[i].name);
        while(ft9366_replay_next(scripts[i].data,scripts[i].size,&offset,&record)==1) {
            printf("%zu %s %u",++index,record.operation==0?"OUT01":record.operation==1?"IN82":"DELAY_MS",record.length);
            if(record.operation==0) for(size_t j=0;j<record.length;j++) printf(" %02x",record.payload[j]);
            putchar('\n');
        }
    }
}
static bool expected_layout(libusb_device *device)
{
    struct libusb_config_descriptor *config=NULL;
    if(libusb_get_active_config_descriptor(device,&config)!=0) return false;
    bool valid=false;
    for(unsigned i=0;i<config->bNumInterfaces;i++) {
        const struct libusb_interface *interface=&config->interface[i];
        for(int j=0;j<interface->num_altsetting;j++) {
            const struct libusb_interface_descriptor *alt=&interface->altsetting[j];
            if(alt->bInterfaceNumber!=0 || alt->bAlternateSetting!=0 ||
               alt->bInterfaceClass!=0xff || alt->bNumEndpoints!=2) continue;
            unsigned endpoints=0;
            for(unsigned k=0;k<alt->bNumEndpoints;k++) {
                const struct libusb_endpoint_descriptor *ep=&alt->endpoint[k];
                if((ep->bmAttributes&LIBUSB_TRANSFER_TYPE_MASK)!=LIBUSB_TRANSFER_TYPE_BULK ||
                    ep->wMaxPacketSize!=512) continue;
                if(ep->bEndpointAddress==0x01) endpoints|=1;
                if(ep->bEndpointAddress==0x82) endpoints|=2;
            }
            valid|=endpoints==3;
        }
    }
    libusb_free_config_descriptor(config);
    return valid;
}
int main(int argc,char **argv)
{
    (void)setvbuf(stdout,NULL,_IOLBF,0);
    for(size_t i=0;i<G_N_ELEMENTS(scripts);i++)
        if(!ft9366_replay_validate(scripts[i].data,scripts[i].size)) {
            fprintf(stderr,"Refusing malformed reference sequence\n");return 2;
        }
    if(argc==1 || (argc==2 && strcmp(argv[1],"--plan")==0)) { manifest();return 0; }
    if(argc!=2 || strcmp(argv[1],"--capture")!=0) {
        fprintf(stderr,"Usage: ft9366-capture-reference [--plan|--capture]\n");return 2;
    }
    puts("REFERENCE-BASED CAPTURE: requires explicit approval of --plan. Keep finger OFF during initialization.");
    puts("Press Enter to initialize, or Ctrl-C to stop.");
    if(getchar()!='\n') return 2;
    signal(SIGINT,on_signal); signal(SIGTERM,on_signal);
    libusb_context *context=NULL; libusb_device **devices=NULL;
    libusb_device *target=NULL; libusb_device_handle *handle=NULL;
    bool claimed=false; int status=1;
    if(libusb_init_context(&context,NULL,0)!=0) { fprintf(stderr,"libusb initialization failed\n");return 1; }
    ssize_t count=libusb_get_device_list(context,&devices);
    unsigned matches=0;
    for(ssize_t i=0;i<count;i++) {
        struct libusb_device_descriptor descriptor;
        if(libusb_get_device_descriptor(devices[i],&descriptor)==0 && descriptor.idVendor==0x2808 && descriptor.idProduct==0xa658) {
            target=devices[i];matches++;
        }
    }
    if(matches!=1) { fprintf(stderr,"Expected one 2808:a658 device; found %u\n",matches);goto cleanup; }
    if(!expected_layout(target)) { fprintf(stderr,"Unexpected interface layout; refusing reset/writes\n");goto cleanup; }
    int result=libusb_open(target,&handle);
    if(result!=0) { fprintf(stderr,"Open: %s\n",libusb_error_name(result));goto cleanup; }
    result=libusb_kernel_driver_active(handle,0);
    if(result!=0 && result!=LIBUSB_ERROR_NOT_SUPPORTED) {
        fprintf(stderr,"Refusing active kernel driver or unknown interface ownership\n");goto cleanup;
    }
    if(interrupted) goto cleanup;
    result=libusb_reset_device(handle);
    if(result!=0) { fprintf(stderr,"Reset: %s\n",libusb_error_name(result));goto cleanup; }
    result=libusb_claim_interface(handle,0);
    if(result!=0) { fprintf(stderr,"Claim: %s\n",libusb_error_name(result));goto cleanup; }
    claimed=true;
    deadline=g_get_monotonic_time()+60*G_USEC_PER_SEC;
    g_usleep(500000);
    struct response response;
    if(!replay(handle,&scripts[0],&response) || response.frames!=4) {
        fprintf(stderr,"Initialization did not complete with four reference-sized frames\n");goto cleanup;
    }
    puts("Reference initialization transfers completed; four calibration-sized frames discarded.");
    puts("PLACE AND HOLD your finger flat on the sensor now (up to 10 seconds).");
    gint64 touch_deadline=g_get_monotonic_time()+10*G_USEC_PER_SEC;
    gint64 capture_deadline=deadline;
    deadline=MIN(deadline,touch_deadline);
    bool finger=false;
    for(unsigned poll=0;poll<40 && g_get_monotonic_time()<touch_deadline;poll++) {
        if(!replay(handle,&scripts[1],&response) || response.last_length!=2) goto cleanup;
        if(response.flag&0x0002) { finger=true;break; }
        g_usleep(150000);
    }
    deadline=capture_deadline;
    if(!finger) { fprintf(stderr,"No reference finger-present flag observed\n");goto cleanup; }
    puts("Reference finger-present flag observed; requesting frames.");
    if(!replay(handle,&scripts[2],&response) || response.frames!=1) goto cleanup;
    puts("First touch frame received and discarded, following reference settling behavior.");
    if(!replay(handle,&scripts[1],&response) || response.last_length!=2 || !(response.flag&0x0002)) {
        fprintf(stderr,"Finger removed before second frame\n");goto cleanup;
    }
    g_usleep(60000);
    if(!replay(handle,&scripts[2],&response) || response.frames!=1) goto cleanup;
    puts("Finger-triggered 10240-byte sensor frame received in memory and cleared.");
    puts("This confirms frame transfer only; fingerprint ridge quality, enrollment and matching are unverified.");
    status=0;
cleanup:
    if(claimed) {
        int release=libusb_release_interface(handle,0);
        if(release!=0) { fprintf(stderr,"Release: %s\n",libusb_error_name(release));status=1; }
    }
    if(handle) libusb_close(handle);
    if(devices) libusb_free_device_list(devices,1);
    libusb_exit(context);
    puts("USB resources closed; no biometric payload saved.");
    return status;
}
