SHELL:=/bin/bash
BASE_DIR := $(shell pwd)
KERNEL_PATH ?= $(BASE_DIR)/../../../bmbt_linux
KERNEL_IMG=$(BASE_DIR)/linux.tar.xz

# https://unix.stackexchange.com/questions/11018/how-to-choose-directory-name-during-untarring

ifneq ($(shell uname -m), x86_64)
$(error mismatch architecture)
endif

kernel:
	# down load img
	if [[ ! -f $(KERNEL_IMG) ]]; then \
		curl -L https://mirrors.edge.kernel.org/pub/linux/kernel/v4.x/linux-4.4.142.tar.xz > linux.tar.xz; \
	fi
	# and extract
	if [[ ! -d $(KERNEL_PATH) ]]; then \
		mkdir -p $(KERNEL_PATH); \
		echo "$(KERNEL_PATH) not found, try to install one"; \
		tar -xvf linux.tar.xz -C $(KERNEL_PATH) --strip-components 1; \
	fi
	# copy modified kconfig
	if [[ ! -f $(KERNEL_PATH)/.config ]]; then \
		cp $(BASE_DIR)/kernel-config/tiny_kernel.config $(KERNEL_PATH)/.config; \
	fi
	# skip "set_video" because we didn't implemented the devices
	sed -i 's/set_video/\/\/ set_videio/' $(KERNEL_PATH)/arch/x86/boot/main.c
	make -C $(KERNEL_PATH)
	cp $(KERNEL_PATH)/arch/x86/boot/bzImage  $(BASE_DIR)/image/bzImage.bin
