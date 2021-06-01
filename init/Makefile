kernel := kernel.bin

CFLAGS := -static
linker_script := linker.ld
assembly_source_files := head.S
c_source_files := $(wildcard *.c)

assembly_object_files := $(patsubst %.S, %.o, $(assembly_source_files))
c_object_files := $(patsubst %.c, %.o, $(c_source_files))

UNAME := $(shell uname -a)
ifeq (,$(findstring loongson, $(UNAME)))
ARCH_PREFIX=~/arch/LARCH_toolchain_root_newabi/bin/loongarch64-linux-gnu-
QEMU=~/core/ld/qemu_bak/mybuild/loongson-softmmu/qemu-system-loongson
endif

GCC=$(ARCH_PREFIX)gcc
LD=$(ARCH_PREFIX)ld

DEF = ../../qemu_bak/vmlinux

.PHONY: all clean

all: $(kernel)

clean:
	rm -f $(assembly_object_files)
	rm -f $(c_object_files)
	rm -f $(kernel)

$(kernel): $(assembly_object_files) $(c_object_files) $(linker_script)
	$(LD) $(CFLAGS) -n -T $(linker_script) -o $(kernel) $(assembly_object_files) $(c_object_files)

gdb: $(kernel)
	 gdb --args $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel)

run: $(kernel)
	 $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel)

defgdb: $(kernel)
	 gdb --args $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(DEF)

defrun: $(kernel)
	 $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel) $(DEF)

# compile assembly files
%.o: %.S
	$(GCC) $(CFLAGS) -D__ASSEMBLY__ -c $< -o $@

%.o: %.c
	$(GCC) $(CFLAGS) -c $< -o $@
