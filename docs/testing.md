# Testing

Run `make test`. The suite uses synthetic bytes to check big-endian decoding,
first/last samples, untouched output on errors, exact frame size, null pointers,
output capacity and a trailing output canary. No USB access occurs.

Additional check:

```sh
ASAN_OPTIONS=detect_leaks=0 make test CC=clang BUILD_DIR=/tmp/ft9366-clang CFLAGS='-std=c11 -Wall -Wextra -Wpedantic -Werror -g -fsanitize=address,undefined'
```

On November 14, 2026 GCC tests passed. Clang AddressSanitizer/UndefinedBehaviorSanitizer
passed with leak detection disabled. Default LeakSanitizer failed due to the sandbox's
ptrace restrictions; leak checking was not completed. No system security setting changed.

Hardware gates: reviewed initialization, discovery, enrollment, repeated genuine
verification, different-finger rejection, cancel, disconnect, timeout and suspend/resume.
Record counts and failures without biometric payloads. Detection and compilation do
not establish functionality or authentication security. Hardware tests remain pending.

Capture tests additionally exercise every two-fragment split of a 10240-byte
synthetic frame, 512-byte fragments, partial-frame rejection, repeated starts,
oversized lengths including SIZE_MAX, cancellation/timeout/disconnect, late feeds,
ready-frame cancellation, and clearing internal buffers. Hardware transfer
boundaries are not inferred from these synthetic fragment patterns.

`make test` also runs Python unittest cases for the offline reference-container
audit: record counts, delay parsing, invalid operations and truncated payloads.

Transport tests use a simulated backend to test endpoint selection, timeout
forwarding, write ownership, read bounds, short/oversized completions, submission
failure, cancellation races, stale callbacks and token exhaustion. The full suite
is three C test executables plus four Python unittest cases. No real USB transport,
libfprint adapter or hardware timeout behavior has yet been tested.
