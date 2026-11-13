CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
BUILD_DIR ?= /tmp/libfprint-ft9366-arch-build
.PHONY: test
test:
	mkdir -p "$(BUILD_DIR)"
	$(CC) $(CFLAGS) -Idriver tests/test-frame.c driver/ft9366-frame.c -o "$(BUILD_DIR)/test-frame"
	"$(BUILD_DIR)/test-frame"
	$(CC) $(CFLAGS) -Idriver tests/test-capture.c driver/ft9366-capture.c driver/ft9366-frame.c -o "$(BUILD_DIR)/test-capture"
	"$(BUILD_DIR)/test-capture"
	$(CC) $(CFLAGS) -Idriver tests/test-transport.c driver/ft9366-transport.c -o "$(BUILD_DIR)/test-transport"
	"$(BUILD_DIR)/test-transport"
	PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s tests -p 'test_*.py'

.PHONY: adapter-test
adapter-test:
	python3 scripts/build_adapter_test.py --libfprint-source "$(LIBFPRINT_SOURCE)" --build-dir "$(BUILD_DIR)/adapter" --cc "$(CC)" --cflags "$(CFLAGS)"

ifneq ($(strip $(LIBFPRINT_SOURCE)),)
test: adapter-test
endif
