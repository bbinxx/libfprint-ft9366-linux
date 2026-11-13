# First hardware connection probe

Original diagnostic: tools/ft9366-connect.c. It matches exactly one 2808:a658,
checks interface 0/alternate 0 with bulk endpoints 0x01 and 0x82 (512-byte packets),
and defaults to descriptor inspection. --connect opens the device, checks for a
kernel driver, claims interface 0, releases it, and closes the handle.
It refuses to detach a kernel driver and never requests a USB reset or sends
bulk/interrupt/vendor protocol commands. Standard USB enumeration/access may wake
the device; no persistent power or permission settings are modified.

## Build and run

```sh
make connection-probe BUILD_DIR=/tmp
/tmp/ft9366-connect --help
/tmp/ft9366-connect --connect
```

If open returns LIBUSB_ERROR_ACCESS, the owner can run the reviewed executable
from their own terminal:

```sh
sudo /tmp/ft9366-connect --connect
```

Never share a sudo password in chat. Read output before proceeding to sensor
commands. No fprintd stop or system package replacement is required for this probe;
if the interface is busy, stop and investigate rather than detach another owner.

## Actual observations: November 14, 2026

- Compiled with the installed compiler and libusb 1.0.30 under strict warnings.
- Outside-sandbox run found 2808:a658 at bus 003 device 007 and confirmed endpoints.
- libusb_open returned LIBUSB_ERROR_ACCESS for the unprivileged user.
- Privileged retry using sudo -n stopped because sudo requires a password.
- The owner subsequently ran sudo /tmp/ft9366-connect --connect and supplied the
  output below. It reports successful open, claim, release and close. This is
  owner-reported hardware evidence; the agent did not execute the privileged run.
  The numeric exit status was not included in the supplied transcript.

```text
Found 2808:a658 on bus 003 device 007
Interface 0: bulk OUT 0x01 / IN 0x82, 512-byte packets verified
USB handle opened
Interface 0 claimed successfully
Interface 0 released successfully
USB handle closed
```

Success for this milestone is explicit open, claim, release and close output with
exit status 0. It establishes USB access only, not sensor initialization, capture,
enrollment or matching. This standalone libusb probe does not validate the separate
libfprint adapter or device class. No image or template data is read or stored.
