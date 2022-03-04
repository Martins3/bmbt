SHELL:=/bin/bash
BASE_DIR := $(shell pwd)
clean=$(shell git -C seabios diff --stat)

ifneq ($(shell uname -m), x86_64)
$(error mismatch architecture)
endif
all:
	if [[ "$(clean)" != "" ]]; then \
		echo 'dirty, skip the patch'; \
	else \
		echo 'clean, apply the patch'; \
		git -C seabios apply $(BASE_DIR)/patch/seabios.patch; \
	fi
	$(MAKE) -C seabios
	cp seabios/out/bios.bin image/bios.bin
