#!/usr/bin/make
SHELL:=/bin/bash

ARCH_FLAGS := -DBUILD_ON_LOONGSON
BASE_DIR := $(shell pwd)
BUILD_DIR := $(BASE_DIR)/build_loongson

UNAME := $(shell uname -a)
ifneq (,$(findstring x86_64, $(UNAME)))
	BUILD_DIR := /tmp/build_x86
	ARCH_FLAGS :=
endif
# QEMU=~/core/ld/qemu_bak/mybuild/loongson-softmmu/qemu-system-loongson

bmbt := $(BUILD_DIR)/bmbt.bin
LIBCAPSTONE := $(BUILD_DIR)/capstone/libcapstone.a

include ./kernel.mak
ifeq (, $(KERNEL_PATH))
$(error "kernel path not setup, create kernel.mak and set KERNEL_PATH in it")
endif

# ================================= glib =======================================
# lglib=1 to use local glib
ifeq ($(glib), 0)
GLIB_LIB     = $(shell pkg-config --libs gthread-2.0) -DUSE_SYSTEM_GLIB
$(info use system glib)
else
$(info use local glib)
endif
GLIB_INCLUDE = $(shell pkg-config --cflags glib-2.0)
# @todo libgcc is absolute path
# github action will not work with the absolute path
# -lc : add glibc
# -lm : add libmath
GLIBS= -lglib-2.0 -lrt -lm -lc

# ================================= glib =======================================

GCOV_CFLAGS=-fprofile-arcs -ftest-coverage
GCOV_LFLAGS=-lgcov --coverage


CFLAGS_HEADER=-I$(BASE_DIR)/capstone/include $(GLIB_INCLUDE) -I$(BASE_DIR)/include
CFLAGS := -g -Werror $(CFLAGS_HEADER) $(GLIB_LIB) $(GCOV_CFLAGS) $(ARCH_FLAGS)
LFLAGS := -g -Werror $(GCOV_LFLAGS) $(GLIB_LIB)


linker_script := src/linker.ld
# assembly_source_files := src/head.S
assembly_source_files :=

C_SRC_FILES += $(wildcard src/*.c)
C_SRC_FILES += $(wildcard src/tcg/*.c)
C_SRC_FILES += $(wildcard src/hw/*/*.c)
C_SRC_FILES += $(wildcard src/fpu/*.c)
C_SRC_FILES += $(wildcard src/qemu/*.c)
C_SRC_FILES += $(wildcard src/util/*.c)
C_SRC_FILES += $(wildcard src/test/*.c)
C_SRC_FILES += $(wildcard src/i386/*.c)

CONFIG_LATX=y
CONFIG_SOFTMMU=y
include ./src/i386/Makefile.objs
LATX_SRC=$(addprefix src/i386/, $(obj-y))
C_SRC_FILES += $(LATX_SRC)

ASSEMBLY_OBJ_FILES := $(assembly_source_files:%.S=$(BUILD_DIR)/%.o)
C_OBJ_FILES := $(C_SRC_FILES:%.c=$(BUILD_DIR)/%.o)

OBJ_FILES := $(ASSEMBLY_OBJ_FILES) $(C_OBJ_FILES)

CXX=/home/maritns3/core/iwyu/build/bin/include-what-you-use # 暂时不使用 iwyu
CXX=gcc

ARCH_PREFIX :=
GCC=$(ARCH_PREFIX)${CXX}
LD=$(ARCH_PREFIX)ld
AR=$(ARCH_PREFIX)ar
RANLIB=ranlib

DEF = ../../qemu_bak/vmlinux

dependency_files = $(OBJ_FILES:%.o=%.d)

# $(info GCC=$(GCC))
# $(info OBJ_FILES=$(OBJ_FILES))
# $(info dependency_files=$(dependency_files))
# $(info $(BASE_DIR))

all: check-and-reinit-submodules capstone $(bmbt)

# https://stackoverflow.com/questions/52337010/automatic-initialization-and-update-of-submodules-in-makefile
.PHONY: check-and-reinit-submodules capstone seabios
check-and-reinit-submodules:
	@if git submodule status | egrep -q '^[-]|^[+]' ; then \
            echo "INFO: Need to reinitialize git submodules"; \
            git submodule update --init; \
	fi

kernel:
	$(MAKE) -C $(KERNEL_PATH)
	mkdir -p image
	cp $(KERNEL_PATH)/arch/x86/boot/bzImage image/bzImage

CAP_CFLAGS=$(CFLAGS_HEADER)
CAP_CFLAGS+=-DCAPSTONE_HAS_X86
capstone:
	@mkdir -p $(@D)
	@$(MAKE) -C ./capstone CAPSTONE_SHARED=no BUILDDIR="$(BUILD_DIR)/capstone" CC="$(CXX)" AR="$(AR)" LD="$(LD)" RANLIB="$(RANLIB)" CFLAGS="$(CAP_CFLAGS)" --no-print-directory --quiet BUILD_DIR=$(BUILD_DIR) $(LIBCAPSTONE)

-include $(dependency_files)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(dependency_files)`.
$(BUILD_DIR)/%.o : %.c
	@mkdir -p $(@D)
	@$(GCC) $(CFLAGS) -MMD -c $< -o $@
	@# $(GCC) $(CFLAGS) -MMD -E $< -o $@.c
	@echo "  CC      $<"

# compile assembly files
$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(@D)
	$(GCC) $(CFLAGS) -MMD -D__ASSEMBLY__ -c $< -o $@
	@echo "  CC      $<"

# Actual target of the binary - depends on all .o files.
$(bmbt) : $(OBJ_FILES) capstone
		@mkdir -p $(@D)
		@echo "  Link    $@"
		@$(GCC) $(OBJ_FILES) $(LFLAGS) $(LIBCAPSTONE) $(GLIBS) -o $(bmbt)

# $(LD) $(CFLAGS) -n -T $(linker_script) -o $(bmbt) $(OBJ_FILES)


.PHONY: all clean gdb run gcov clear_gcda test

GCOV_OUT=$(BUILD_DIR)/gcov
GCOV_INFO=$(GCOV_OUT)/bmbt_coverage.info
GCOV_MERGE_INFO=$(GCOV_OUT)/bmbt_merge.info

gcov:
	@mkdir -p $(GCOV_OUT)
	lcov -capture --directory $(BUILD_DIR) --output-file $(GCOV_INFO)
	if [[ -e $(GCOV_MERGE_INFO) ]]; then\
		lcov -a $(GCOV_MERGE_INFO) -a $(GCOV_INFO) -d $(BUILD_DIR) -o $(gcov_merge_info); \
	else \
		lcov -a $(GCOV_INFO) -d $(BUILD_DIR) -o $(GCOV_MERGE_INFO); \
	fi
	genhtml $(GCOV_MERGE_INFO) --output-directory $(GCOV_OUT)
	# microsoft-edge $(GCOV_OUT)/index.html

clean:
	rm -r $(BUILD_DIR)

clear_gcda:
	@find $(BUILD_DIR) -name "*.gcda" -type f -delete

run: all clear_gcda
	@# $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -bmbt $(bmbt)
	@# only test work in process
	$(bmbt) -s wip

# use command "bear make compile -j10" to generate compile_comands.json
compile: all
	$(MAKE) -C seabios
	$(MAKE) -C pc-bios

test: all clear_gcda
	$(bmbt)


gdb: all
	@#gdb --args $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -bmbt $(DEF)
	gdb --args $(bmbt)

defrun: $(bmbt)
	 $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -bmbt $(bmbt) $(DEF)
