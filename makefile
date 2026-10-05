SHELL := /bin/bash

SDK_ENV := /opt/fsl-imx-x11/4.1.15-2.1.0/environment-setup-cortexa7hf-neon-poky-linux-gnueabi
TARGET := main
OUTPUT := output
CFLAGS ?= -std=c11 -O2 -g -Wall -Wextra -Wpedantic
SOURCES := main.c $(wildcard Hardware/*/*.c System/*/*.c)
HEADERS := $(wildcard *.h Hardware/*/*.h System/*/*.h)
OBJECTS := $(patsubst %.c,$(OUTPUT)/%.o,$(SOURCES))

.PHONY: all clean
.DELETE_ON_ERROR:
all: $(TARGET)

$(TARGET): $(OBJECTS) makefile $(SDK_ENV)
	source "$(SDK_ENV)" && $$CC $(OBJECTS) -o "$@"

$(OUTPUT)/%.o: %.c $(HEADERS) makefile $(SDK_ENV)
	mkdir -p "$(@D)"
	source "$(SDK_ENV)" && $$CC $(CPPFLAGS) $(CFLAGS) -c "$<" -o "$@"

clean:
	rm -f -- "$(TARGET)"
	rm -rf -- "$(OUTPUT)"
