BASE_DIR = $(shell pwd)
BUILD_DIR := $(BASE_DIR)/build
kernel := $(BUILD_DIR)/kernel.bin
LIBCAPSTONE := $(BUILD_DIR)/capstone/libcapstone.a

CFLAGS_HEADER=-I$(BASE_DIR)/capstone/include
CFLAGS := -Werror $(CFLAGS_HEADER)

linker_script := src/linker.ld
# assembly_source_files := src/head.S
assembly_source_files :=

c_source_files += $(wildcard src/*.c)
c_source_files := $(wildcard src/tcg/*.c)
c_source_files += $(wildcard src/i386/*.c)
c_source_files += $(wildcard src/hw/core/*.c)
c_source_files += $(wildcard src/hw/qemu/*.c)
c_source_files += $(wildcard src/fpu/*.c)
c_source_files += $(wildcard src/qemu/*.c)
c_source_files += $(wildcard src/util/*.c)

c_source_files += $(wildcard src/i386/LATX/*.c)
c_source_files += $(wildcard src/i386/LATX/translator/*.c)
c_source_files += $(wildcard src/i386/LATX/optimization/*.c)


assembly_object_files := $(assembly_source_files:%.S=$(BUILD_DIR)/%.o)
c_object_files := $(c_source_files:%.c=$(BUILD_DIR)/%.o)

obj_files := $(assembly_object_files) $(c_object_files)

# UNAME := $(shell uname -a)
# ifeq (,$(findstring loongson, $(UNAME)))
# ARCH_PREFIX=~/arch/LARCH_toolchain_root_newabi/bin/loongarch64-linux-gnu-
# QEMU=~/core/ld/qemu_bak/mybuild/loongson-softmmu/qemu-system-loongson
# endif


CXX=/home/maritns3/core/iwyu/build/bin/include-what-you-use # 暂时不使用 iwyu
CXX=gcc

GCC=$(ARCH_PREFIX)${CXX}
LD=$(ARCH_PREFIX)ld
AR=$(ARCH_PREFIX)ar
RANLIB=ranlib

DEF = ../../qemu_bak/vmlinux

dependency_files = $(obj_files:%.o=%.d)

# $(info GCC=$(GCC))
# $(info obj_files=$(obj_files))
# $(info dependency_files=$(dependency_files))
$(info $(BASE_DIR))

all: $(kernel) $(LIBCAPSTONE)

format:

-include $(dependency_files)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(dependency_files)`.
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
	echo "happy"
	# $(LD) $(CFLAGS) -n -T $(linker_script) -o $(kernel) $(obj_files)

CAP_CFLAGS=$(CFLAGS_HEADER)
CAP_CFLAGS+=-DCAPSTONE_HAS_X86

$(LIBCAPSTONE) :
	mkdir -p $(@D)
	$(MAKE) -C ./capstone CAPSTONE_SHARED=no BUILDDIR="$(BUILD_DIR)/capstone" CC="$(CXX)" AR="$(AR)" LD="$(LD)" RANLIB="$(RANLIB)" CFLAGS="$(CAP_CFLAGS)" --no-print-directory --quiet BUILD_DIR=$(BUILD_DIR) $(LIBCAPSTONE)

.PHONY: all clean

clean:
	rm -r $(BUILD_DIR)

gdb: $(kernel)
	 gdb --args $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel)

run: $(kernel)
	 $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel)

defgdb: $(kernel)
	 gdb --args $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(DEF)

defrun: $(kernel)
	 $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel) $(DEF)
