kernel := kernel.bin

linker_script := linker.ld
assembly_source_files := head.S
assembly_object_files := $(patsubst %.S, %.o, $(assembly_source_files))

ARCH_PREFIX=~/arch/LARCH_toolchain_root_newabi/bin/loongarch64-linux-gnu
QEMU=~/core/ld/qemu_bak/mybuild/loongson-softmmu/qemu-system-loongson

GCC=$(ARCH_PREFIX)-gcc
LD=$(ARCH_PREFIX)-ld

.PHONY: all clean

all: $(kernel)

clean:
	rm -f $(assembly_object_files)
	rm -f $(kernel)

$(kernel): $(assembly_object_files) $(linker_script)
	$(LD) -n -T $(linker_script) -o $(kernel) $(assembly_object_files)

gdb: $(kernel)
	 gdb --args $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel)

run: $(kernel)
	 $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel)

# compile assembly files
%.o: %.S
	$(GCC) -c $< -o $@
