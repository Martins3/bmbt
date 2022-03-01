ifneq ($(shell uname -m), x86_64)
$(error mismatch architecture)
endif

all:
	mkdir -p image
	$(MAKE) -f make/kernel.mk
	$(MAKE) -C pc-bios
	$(MAKE) -C initrd
	$(MAKE) -f make/seabios.mk
