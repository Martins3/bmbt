#!/usr/bin/make
SHELL:=/bin/bash
BASE_DIR := $(shell pwd)
ARCH_APPENDIX :=loongson
CFLAGS := -g -Wall -Werror -O3

ENV_KERNEL ?= 1
HAMT ?= 1

USE_LIBC ?= 0
USE_GLIB ?= 0
USE_ULIBC_FILE ?= 1

CAPSTONE_DIR=src/target/i386/LATX/capStone

ifeq ($(USE_LIBC), 1)
  LIBC_APPENDIX=libc
  LIBC_CFLAGS=-DUSE_SYSTEM_LIBC
  # gcov only avaliable in system libc
  GCOV_CFLAGS=-fprofile-arcs -ftest-coverage
  GCOV_LFLAGS=-lgcov --coverage
else
  LIBC_CFLAGS=-nostdinc -nostdlib
  LIBC_HEADER=-I$(BASE_DIR)/libc/include
  LIB_C=$(BUILD_DIR)/libc/libc.a
  ifeq ($(USE_ULIBC_FILE), 0)
	  $(error ulibc doesn't implement fread,fwrite,etc. )
  endif
endif

ifeq ($(HAMT), 1)
	CFLAGS += -DHAMT
endif

ifeq ($(USE_ULIBC_FILE), 1)
  # gcov use file related syscall, but ulibc file implementation rewrite them
  GCOV_CFLAGS=
  GCOV_LFLAGS=
  CFLAGS += -DUSE_ULIBC_FILE
  ENV_SRC_FILES += file/ulibc-file.c
else
  ENV_SRC_FILES += file/glibc-file.c
endif

ifeq ($(USE_GLIB), 1)
  GLIB_LIB     = $(shell pkg-config --libs gthread-2.0) -DUSE_SYSTEM_GLIB
  GLIB_HEADER  = $(shell pkg-config --cflags glib-2.0)
  GLIB_APPENDIX=glib
  ifneq ($(USE_LIBC), 1)
  	$(error glib depends on libc)
  endif
endif

ifeq ($(ENV_KERNEL), 1)
  ENV_APPENDIX=kernel
  ifeq ($(USE_LIBC), 1)
  	$(error there's no glibc in baremetal)
  endif
  LDFLAGS = -T env/loongarch/linker.ld
  ASM_SRC_FILES += $(wildcard env/loongarch/*/*.S)
  ENV_CFLAGS += -DENV_KERNEL -I$(BASE_DIR)/env/loongarch/include/
  ENV_CFLAGS += -isystem /usr/lib/gcc/loongarch64-linux-gnu/8/include
  ENV_CFLAGS += -fno-omit-frame-pointer
else
  ENV_CFLAGS += -DENV_USERSPACE
  ENV_APPENDIX=userspace
  ENV_SRC_FILES += $(wildcard env/userspace/*.c)
endif

UNAME := $(shell uname -a)
ifneq (,$(findstring x86_64, $(UNAME)))
  ARCH_APPENDIX := x86
  ifeq ($(ENV_KERNEL), 1)
  	$(error "x86 doesn't support kernel mode")
  endif

  ifeq ($(USE_LIBC), 0)
  	$(error "x86 need libc to compile")
  endif
endif

BUILD_DIR := $(BASE_DIR)/build_$(ARCH_APPENDIX)_$(ENV_APPENDIX)_$(LIBC_APPENDIX)_$(GLIB_APPENDIX)
CAPSTONE_HEADER=-I$(BASE_DIR)/$(CAPSTONE_DIR)/include
LIB_CAPSTONE=$(BUILD_DIR)/$(CAPSTONE_DIR)/libcapstone.a

CFLAGS += $(LIBC_CFLAGS) $(LIBC_HEADER) $(ENV_CFLAGS)
ifeq ($(DEBUG), 1)
  $(info LIBC_HEADER=$(LIBC_HEADER))
  $(info GLIB_HEADER=$(GLIB_HEADER))
  $(info CAPSTONE_HEADER=$(CAPSTONE_HEADER))
  $(info LIBC_CFLAGS=$(LIBC_CFLAGS))
  $(info GCOV_CFLAGS=$(GCOV_CFLAGS))
  $(info GCOV_LFLAGS=$(GCOV_LFLAGS))
endif

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(@D)
	@gcc $(CFLAGS) -MMD -D__ASSEMBLY__ -c $< -o $@
	@echo "CC      $<"
