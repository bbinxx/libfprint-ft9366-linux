# Device interface lifecycle

Original Track B helper: driver/ft9366-device.c. This is interface/session
management, not the registered libfprint driver or sensor initialization.
All verification is simulated; no real interface claim, release or reset ran.

## libfprint ownership evidence

At libfprint commit 80a4b5ec612892c5c056c48dddaa561452cf37ec,
fp-device.c fp_device_open opens the GUsbDevice before invoking the driver's open
vfunc. fpi-device.c fpi_device_close_complete closes it after driver cleanup.
The helper must not duplicate those handle operations. Its tests explicitly
reject USB open, close or reset calls.

## API and states

Construction retains the USB object and creates an adapter holding a FpDevice
reference; it performs no USB operation. Missing USB objects are rejected.
claim requests interface 0 with G_USB_DEVICE_CLAIM_INTERFACE_NONE. No kernel-driver
detachment, device reset, configuration change or sensor command is requested.
Failure leaves CLOSED; it never releases an interface it did not acquire.

States are CLOSED, CLAIMED, CLOSING and RELEASE_FAILED. CLAIMED means only that
interface ownership was acquired. It does not mean the sensor is initialized or
ready for acquisition. start/read/result access is restricted to CLAIMED.
Operations and callbacks must share one serialized event context.

close rejects duplicate close requests. If a transfer is active, close requests
cancellation and waits for its callback. If cancellation is already pending, close
waits without requesting it again. New transfers and destruction are blocked while
closing. When the callback drains, the helper discards retained transport data,
releases interface 0 and emits CLOSE_DONE instead of a normal transfer notification.
The callback may arrive synchronously when no drain is needed.

A non-disconnect release failure enters RELEASE_FAILED, preserving ownership and
blocking reopen/destruction. The caller can retry close and receives the release
error in the notification. If release reports NO_DEVICE, the physical device is
gone: no live claim remains, state becomes CLOSED, and the error is still reported.
The GError passed to the owner is borrowed only for that notification; copy it if
needed later. Neither error payloads nor biometric data are logged.

The owner may destroy the helper from a CLOSED notification, including the nested
transfer-completion -> close -> destroy path. Destruction refuses to hide a live
interface or pending callback. It does not close the USB handle. A future FpDevice
adapter must call libfprint completion hooks at the appropriate point and coordinate
release-failure recovery before allowing libfprint to close its handle.

## Verification

```sh
make test LIBFPRINT_SOURCE=/tmp/ft9366-libfprint-source
ASAN_OPTIONS=detect_leaks=0 make test LIBFPRINT_SOURCE=/tmp/ft9366-libfprint-source CC=clang BUILD_DIR=/tmp/ft9366-clang CFLAGS='-std=c11 -Wall -Wextra -Wpedantic -Werror -g -fsanitize=address,undefined'
```

Both commands passed the full suite. Lifecycle tests compile against real private
headers and GLib/libgusb types while replacing USB/FpDevice access with test doubles.
They verify claim failures, duplicate claims, transfer readback, cancel/drain close,
success/cancel races, duplicate close, release failure/retry, removal, missing USB
objects and callback-time destruction. Weak references verify device/USB reference
release. ASan/UBSan passed; leak detection remains excluded by sandbox restrictions.
This is not a complete libfprint build or a physical hardware test.

Next: register a FpDevice subclass and wire its completion/cancellation hooks,
without declaring sensor initialization successful before a verified sequence
exists. Sensor setup and actual transfers require their evidence and approval gates.
