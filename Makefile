CFLAGS :=
BUILD_DIR := build
kernel := $(BUILD_DIR)/kernel.bin
linker_script := src/linker.ld
assembly_source_files := src/head.S
# c_source_files := $(wildcard src/*.c)
c_source_files := src/tcg/tcg.c

assembly_object_files := $(assembly_source_files:%.S=$(BUILD_DIR)/%.o)
c_object_files := $(c_source_files:%.c=$(BUILD_DIR)/%.o)

obj_files := $(assembly_object_files) $(c_object_files)

UNAME := $(shell uname -a)
# ifeq (,$(findstring loongson, $(UNAME)))
# ARCH_PREFIX=~/arch/LARCH_toolchain_root_newabi/bin/loongarch64-linux-gnu-
# QEMU=~/core/ld/qemu_bak/mybuild/loongson-softmmu/qemu-system-loongson
# endif


GCC=$(ARCH_PREFIX)gcc
LD=$(ARCH_PREFIX)ld

DEF = ../../qemu_bak/vmlinux

DEP = $(obj_files:%.o=%.d)

$(info GCC=$(GCC))
$(info obj_files=$(obj_files))
$(info DEP=$(DEP))

-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/%.o : %.c
	mkdir -p $(@D)
	$(GCC) $(CFLAGS) -MMD -c $< -o $@

# compile assembly files
$(BUILD_DIR)/%.o: %.S
	mkdir -p $(@D)
	$(GCC) $(CFLAGS) -MMD -D__ASSEMBLY__ -c $< -o $@

# Actual target of the binary - depends on all .o files.
$(kernel) : $(obj_files)
	# Create build directories - same structure as sources.
	mkdir -p $(@D)
	# Just link all the object files.
	$(LD) $(CFLAGS) -n -T $(linker_script) -o $(kernel) $(obj_files)

.PHONY: all clean

all: $(kernel)

clean:
	rm -f $(assembly_object_files)
	rm -f $(c_object_files)
	rm -f $(kernel)

gdb: $(kernel)
	 gdb --args $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel)

run: $(kernel)
	 $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel)

defgdb: $(kernel)
	 gdb --args $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(DEF)

defrun: $(kernel)
	 $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel) $(DEF)
