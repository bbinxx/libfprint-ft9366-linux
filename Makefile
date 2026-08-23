CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
BUILD_DIR ?= /tmp/libfprint-ft9366-arch-build
.PHONY: test
test:
	mkdir -p "$(BUILD_DIR)"
	$(CC) $(CFLAGS) -Idriver tests/test-frame.c driver/ft9366-frame.c -o "$(BUILD_DIR)/test-frame"
	"$(BUILD_DIR)/test-frame"
