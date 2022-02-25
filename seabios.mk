all:
	$(MAKE) -C seabios
	cp seabios/out/bios.bin image/bios.bin
