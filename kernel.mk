SHELL:=/bin/bash
BASE_DIR := $(shell pwd)
KERNEL_PATH ?= $(BASE_DIR)/../../../bmbt_linux

# https://unix.stackexchange.com/questions/11018/how-to-choose-directory-name-during-untarring

ifneq ($(shell uname -m), x86_64)
$(error mismatch architecture)
endif

kernel:
	if [[ ! -d $(KERNEL_PATH) ]]; then \
		mkdir -p $(KERNEL_PATH); \
		echo "$(KERNEL_PATH) not found, try to install one"; \
		curl -L https://mirrors.edge.kernel.org/pub/linux/kernel/v4.x/linux-4.4.142.tar.xz > linux.tar.xz; \
		tar -xvf linux.tar.xz -C $(KERNEL_PATH) --strip-components 1; \
		make tinyconfig; \
	fi
	cd $(KERNEL_PATH) || exit 1
	make -j
	cd $(BASE_DIR)
	cp $(KERNEL_PATH)/arch/x86/boot/bzImage  $(BASE_DIR)/image/bzImage.bin
