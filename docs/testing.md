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
