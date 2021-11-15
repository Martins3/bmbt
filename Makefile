#!/usr/bin/make
SHELL:=/bin/bash

ARCH_BUILD := build_loongson
ARCH_PREFIX :=
ARCH_FLAGS := -DBUILD_ON_LOONGSON
UNAME := $(shell uname -a)
ifneq (,$(findstring x86_64, $(UNAME)))
	ARCH_BUILD := build_x86
	ARCH_FLAGS :=
endif
# QEMU=~/core/ld/qemu_bak/mybuild/loongson-softmmu/qemu-system-loongson

BASE_DIR := $(shell pwd)
BUILD_DIR := $(BASE_DIR)/$(ARCH_BUILD)
kernel := $(BUILD_DIR)/kernel.bin
LIBCAPSTONE := $(BUILD_DIR)/capstone/libcapstone.a

# ================================= glib =======================================
GLIB_LIB     = $(shell pkg-config --libs gthread-2.0) -DUSE_SYSTEM_GLIB
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

c_source_files += $(wildcard src/*.c)
c_source_files += $(wildcard src/tcg/*.c)
c_source_files += $(wildcard src/hw/*/*.c)
c_source_files += $(wildcard src/fpu/*.c)
c_source_files += $(wildcard src/qemu/*.c)
c_source_files += $(wildcard src/util/*.c)
c_source_files += $(wildcard src/test/*.c)
c_source_files += $(wildcard src/i386/*.c)

CONFIG_LATX=y
CONFIG_SOFTMMU=y
include ./src/i386/Makefile.objs
LATX_SRC=$(addprefix src/i386/, $(obj-y))
c_source_files += $(LATX_SRC)

assembly_object_files := $(assembly_source_files:%.S=$(BUILD_DIR)/%.o)
c_object_files := $(c_source_files:%.c=$(BUILD_DIR)/%.o)

obj_files := $(assembly_object_files) $(c_object_files)

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

all: check-and-reinit-submodules capstone $(kernel)

# https://stackoverflow.com/questions/52337010/automatic-initialization-and-update-of-submodules-in-makefile
.PHONY: check-and-reinit-submodules capstone seabios
check-and-reinit-submodules:
	@if git submodule status | egrep -q '^[-]|^[+]' ; then \
            echo "INFO: Need to reinitialize git submodules"; \
            git submodule update --init; \
	fi


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
$(kernel) : $(obj_files) capstone
		@mkdir -p $(@D)
		@echo "  Link    $@"
		@$(GCC) $(obj_files) $(LFLAGS) $(LIBCAPSTONE) $(GLIBS) -o $(kernel)

# $(LD) $(CFLAGS) -n -T $(linker_script) -o $(kernel) $(obj_files)


.PHONY: all clean gdb run gcov clear_gcda test

gcov_out=$(ARCH_BUILD)/gcov
gcov_info=$(gcov_out)/bmbt_coverage.info
gcov_merge_info=$(gcov_out)/bmbt_merge.info

gcov:
	@mkdir -p $(gcov_out)
	lcov -capture --directory $(ARCH_BUILD) --output-file $(gcov_info)
	if [[ -e $(gcov_merge_info) ]]; then\
		lcov -a $(gcov_merge_info) -a $(gcov_info) -d $(ARCH_BUILD) -o $(gcov_merge_info); \
	else \
		lcov -a $(gcov_info) -d $(ARCH_BUILD) -o $(gcov_merge_info); \
	fi
	genhtml $(gcov_merge_info) --output-directory $(gcov_out)
	# microsoft-edge $(gcov_out)/index.html

clean:
	rm -r $(BUILD_DIR)

clear_gcda:
	@find $(BUILD_DIR) -name "*.gcda" -type f -delete

run: all clear_gcda
	@# $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel)
	@# only test work in process
	$(kernel) -s wip

compile: all
	$(MAKE) -C seabios

test: all clear_gcda
	$(kernel)


gdb: all
	@#gdb --args $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(DEF)
	gdb --args $(kernel)

defrun: $(kernel)
	 $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -kernel $(kernel) $(DEF)
