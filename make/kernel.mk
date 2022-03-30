SHELL:=/bin/bash
BASE_DIR := $(shell pwd)
KERNEL_PATH ?= $(BASE_DIR)/../../../bmbt_linux
KERNEL_IMG=$(BASE_DIR)/linux.tar.xz

# https://unix.stackexchange.com/questions/11018/how-to-choose-directory-name-during-untarring

ifneq ($(shell uname -m), x86_64)
$(error mismatch architecture)
endif

setup_src:
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
	# skip "set_video" because we didn't implemented the devices
	sed -i 's/set_video/\/\/ skip this /' $(KERNEL_PATH)/arch/x86/boot/main.c

minimal_config:
	# copy modified kconfig
	if [[ ! -f $(KERNEL_PATH)/.config ]]; then \
		cp $(BASE_DIR)/kernel-config/tiny_kernel.config $(KERNEL_PATH)/.config; \
	fi

def_config:
	if [[ ! -f $(KERNEL_PATH)/.config ]]; then \
		make -C $(KERNEL_PATH) i386_defconfig; \
                # find a better way to add debug info config to guest kernel \
	        sed -i 's/# CONFIG_DEBUG_INFO is not set/CONFIG_DEBUG_INFO=y\n# CONFIG_DEBUG_INFO_REDUCED is not set\n# CONFIG_DEBUG_INFO_SPLIT is not set\n# CONFIG_DEBUG_INFO_DWARF4 is not set\n# CONFIG_GDB_SCRIPTS is not set/' $(KERNEL_PATH)/.config; \
	fi

compile:
	make -C $(KERNEL_PATH)

minimal: setup_src minimal_config compile
	@echo "minimal kernel installed"

copy:
	cp $(KERNEL_PATH)/arch/x86/boot/bzImage  $(BASE_DIR)/image/bzImage.bin

def: setup_src def_config compile copy
	@echo "minimal kernel installed"
