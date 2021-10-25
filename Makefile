#!/usr/bin/make
BASE_DIR = $(shell pwd)
BUILD_DIR := $(BASE_DIR)/build
kernel := $(BUILD_DIR)/kernel.bin
LIBCAPSTONE := $(BUILD_DIR)/capstone/libcapstone.a

# ================================= glib =======================================
GLIB_LIB     = $(shell pkg-config --libs gthread-2.0) -DUSE_SYSTEM_GLIB
GLIB_INCLUDE = $(shell pkg-config --cflags glib-2.0)
# @todo libgcc is absolute path
# github action will not work with the absolute path
GLIBS= -lglib-2.0 -lrt -lm -lc /usr/lib/gcc/x86_64-linux-gnu/9/libgcc.a


$(info $(GLIB_LIB))
$(info $(GLIB_INCLUDE))


# ================================= glib =======================================

CFLAGS_HEADER=-I$(BASE_DIR)/capstone/include $(GLIB_INCLUDE)
CFLAGS := -Werror $(CFLAGS_HEADER) $(GLIB_LIB)


linker_script := src/linker.ld
# assembly_source_files := src/head.S
assembly_source_files :=

c_source_files += $(wildcard src/*.c)
c_source_files += $(wildcard src/tcg/*.c)
c_source_files += $(wildcard src/hw/*/*.c)
c_source_files += $(wildcard src/fpu/*.c)
c_source_files += $(wildcard src/qemu/*.c)
c_source_files += $(wildcard src/util/*.c)
c_source_files += $(wildcard src/unitest/*.c)
c_source_files += $(wildcard src/i386/*.c)

CONFIG_LATX=y
CONFIG_SOFTMMU=y
include ./src/i386/Makefile.objs
LATX_OBJ=$(addprefix ./src/i386/, $(obj-y))

# c_source_files += $(wildcard src/i386/LATX/*.c)
# c_source_files += $(wildcard src/i386/LATX/translator/*.c)
# c_source_files += $(wildcard src/i386/LATX/optimization/*.c)
# c_source_files += $(wildcard src/i386/LATX/ir1/*.c)
# c_source_files += $(wildcard src/i386/LATX/ir2/*.c)

assembly_object_files := $(assembly_source_files:%.S=$(BUILD_DIR)/%.o)
c_object_files := $(c_source_files:%.c=$(BUILD_DIR)/%.o) $(LATX_OBJ)

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
# $(info $(BASE_DIR))

all: check-and-reinit-submodules $(kernel)

# https://stackoverflow.com/questions/52337010/automatic-initialization-and-update-of-submodules-in-makefile
.PHONY: check-and-reinit-submodules
check-and-reinit-submodules:
	@if git submodule status | egrep -q '^[-]|^[+]' ; then \
            echo "INFO: Need to reinitialize git submodules"; \
            git submodule update --init; \
	fi

-include $(dependency_files)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(dependency_files)`.
$(BUILD_DIR)/%.o : %.c
	@mkdir -p $(@D)
	@$(GCC) $(CFLAGS) -MMD -c $< -o $@
	@echo "  CC      $<"

# compile assembly files
$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(@D)
	@$(GCC) $(CFLAGS) -MMD -D__ASSEMBLY__ -c $< -o $@
	@echo "  CC      $<"

# Actual target of the binary - depends on all .o files.
$(kernel) : $(obj_files) $(LIBCAPSTONE)
	@# Create build directories - same structure as sources.
	@mkdir -p $(@D)
	@# Just link all the object files.
	@# $(LD) $(CFLAGS) -n -T $(linker_script) -o $(kernel) $(obj_files)
	@$(LD) $(obj_files) $(LIBCAPSTONE)  -o $(kernel) $(GLIBS)
	@gcc $(obj_files) $(LIBCAPSTONE) $(GLIBS) -o $(kernel)
	@echo "BMBT is ready"

CAP_CFLAGS=$(CFLAGS_HEADER)
CAP_CFLAGS+=-DCAPSTONE_HAS_X86

$(LIBCAPSTONE) :
	@mkdir -p $(@D)
	@$(MAKE) -C ./capstone CAPSTONE_SHARED=no BUILDDIR="$(BUILD_DIR)/capstone" CC="$(CXX)" AR="$(AR)" LD="$(LD)" RANLIB="$(RANLIB)" CFLAGS="$(CAP_CFLAGS)" --no-print-directory --quiet BUILD_DIR=$(BUILD_DIR) $(LIBCAPSTONE)

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
