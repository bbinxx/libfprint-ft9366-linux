CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
BUILD_DIR ?= /tmp/libfprint-ft9366-linux-build
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

.PHONY: connection-probe
connection-probe:
	mkdir -p "$(BUILD_DIR)"
	$(CC) $(CFLAGS) $$(pkg-config --cflags libusb-1.0) tools/ft9366-connect.c $$(pkg-config --libs libusb-1.0) -o "$(BUILD_DIR)/ft9366-connect"

.PHONY: connection-probe-test
connection-probe-test:
	mkdir -p "$(BUILD_DIR)"
	$(CC) $(CFLAGS) $$(pkg-config --cflags libusb-1.0) -Dmain=ft9366_probe_main -c tools/ft9366-connect.c -o "$(BUILD_DIR)/probe-test.o"
	$(CC) $(CFLAGS) $$(pkg-config --cflags libusb-1.0) tests/test-connect.c "$(BUILD_DIR)/probe-test.o" $$(pkg-config --libs libusb-1.0) -o "$(BUILD_DIR)/test-connect"
	"$(BUILD_DIR)/test-connect"

test: connection-probe-test

.PHONY: replay-test reference-capture-probe reference-capture-test
replay-test:
	mkdir -p "$(BUILD_DIR)"
	$(CC) $(CFLAGS) -Itools tests/test-replay.c tools/ft9366-replay.c -o "$(BUILD_DIR)/test-replay"
	"$(BUILD_DIR)/test-replay"

test: replay-test

reference-capture-probe:
	python3 scripts/build_reference_capture.py --reference-source "$(REFERENCE_SOURCE)" --cc "$(CC)" --cflags "$(CFLAGS)"

reference-capture-test:
	python3 scripts/build_reference_capture.py --reference-source "$(REFERENCE_SOURCE)" --cc "$(CC)" --cflags "$(CFLAGS)" --simulate
