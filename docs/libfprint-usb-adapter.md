# libfprint USB adapter

Original Track B adapter: driver/ft9366-fprint-usb.c. Targets libfprint v1.94.100,
commit 80a4b5ec612892c5c056c48dddaa561452cf37ec. It creates FpiUsbTransfer bulk
requests for the existing transport core; it does not open or claim an interface,
register a device, initialize the sensor or supply protocol command payloads.

## API and lifetime

Create the adapter with a valid FpDevice and an optional completion notification.
Creation retains a device reference and performs no USB operation. Use the returned
transport through its normal start/cancel/read/release API. A transfer receives a
fresh GCancellable; cancellation is reported only when completion drains.
Timeout milliseconds are forwarded to fpi_usb_transfer_submit.

fill_bulk_full borrows the core buffer with no destroy callback. libfprint consumes
the transfer reference on submit and unrefs it after the adapter callback returns.
The callback maps G_IO_ERROR and G_USB_DEVICE_ERROR cancellation/timeouts, maps
NO_DEVICE to disconnection, frees the transferred GError, completes the core and
then notifies the owner. Negative actual lengths fail length validation unless an
error already supplies a failure outcome. No raw error text or payload is logged.

The owner notification is the callback's final adapter access, allowing the owner
to free the adapter or start another request there. All calls must be serialized
on the device event context. Destroy refuses while a callback is outstanding;
cancel and drain first. This is not a substitute for device-close state handling.
Action-level cancellation still needs wiring by the later FpDevice integration.

## Reproducible offline verification

```sh
make test LIBFPRINT_SOURCE=/tmp/ft9366-libfprint-source
ASAN_OPTIONS=detect_leaks=0 make test LIBFPRINT_SOURCE=/tmp/ft9366-libfprint-source CC=clang BUILD_DIR=/tmp/ft9366-clang CFLAGS='-std=c11 -Wall -Wextra -Wpedantic -Werror -g -fsanitize=address,undefined'
```

Supply a clean checkout at the pinned revision. The helper checks HEAD and tracked
libfprint changes, generates the enum declarations required for header compilation
using glib-mkenums and compiles against the actual private headers. Its generated
enum header is a compile-only subset for fpi-print.h, not a replacement production
libfprint build artifact. Production Meson generates the full normal enum headers.

Requirements already present at initial verification: C compiler, Python, Git,
pkg-config, glib-mkenums, GLib/GIO/libgusb and installed libfprint public headers.
The helper does not download or execute upstream build scripts. Test transfer
functions simulate USB completion using the real FpiUsbTransfer structure and
GLib types. A plain GObject stands in for the device reference; real FpDevice
methods and USB I/O are not exercised. The executable links GLib/libgusb, not a
modified libfprint shared library. This verifies adapter compilation and behavior
under simulation, not a full libfprint link or hardware readiness.

Tests cover reads/writes, borrowed buffers, timeout forwarding, cancel drain,
success/cancel races, USB and GIO error mapping, negative/short/oversized lengths,
fresh cancellables and callback-time destruction. Full core and Python suites
also passed with GCC and Clang ASan/UBSan. Leak checking remains excluded because
of the sandbox restriction; no system security setting was changed.

## Remaining gates

Build the adapter inside libfprint's Meson tree after approved dependency setup;
implement lifecycle and class registration on their own feature branches. Review
initialization evidence and exact USB operations before any hardware test.
Keep FP_DEBUG_TRANSFER unset during biometric use: libfprint's transfer helper can
dump payloads when transfer debugging is enabled, even though this adapter does not.
Do not store resulting biometric logs or captures in the repository.
