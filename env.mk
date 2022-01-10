#!/usr/bin/make
SHELL:=/bin/bash
BASE_DIR := $(shell pwd)
ARCH_APPENDIX :=loongson
CFLAGS := -g -Wall -O0 -Werror

USE_LIBC ?= 0
USE_GLIB ?= 0
ENV_KERNEL ?= 1
USE_ULIBC_FILE ?= 1

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

# gcov use file related syscall
ifeq ($(USE_ULIBC_FILE), 1)
	ENV_SRC_FILES += file/ulibc-file.c
  GCOV_CFLAGS=
  GCOV_LFLAGS=
	CFLAGS += -DUSE_ULIBC_FILE
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
  LDFLAGS = -T env/kernel/linker.ld
  ENV_SRC_FILES += $(wildcard env/kernel/main.c)
  ENV_SRC_FILES += $(wildcard env/kernel/interrupt-timer.c)
  ASM_SRC_FILES += $(wildcard env/kernel/*.S)
	CFLAGS += -DENV_KERNEL
else
  ENV_APPENDIX=userspace
  ENV_SRC_FILES += $(wildcard env/userspace/*.c)
endif

UNAME := $(shell uname -a)
ifneq (,$(findstring x86_64, $(UNAME)))
	ARCH_APPENDIX := x86
endif

BUILD_DIR := $(BASE_DIR)/build_$(ARCH_APPENDIX)_$(ENV_APPENDIX)_$(LIBC_APPENDIX)_$(GLIB_APPENDIX)
CAPSTONE_HEADER=-I$(BASE_DIR)/capstone/include
LIB_CAPSTONE=$(BUILD_DIR)/capstone/libcapstone.a

CFLAGS += $(LIBC_CFLAGS) $(LIBC_HEADER)
ifeq ($(DEBUG), 1)
  $(info LIBC_HEADER=$(LIBC_HEADER))
  $(info GLIB_HEADER=$(GLIB_HEADER))
  $(info CAPSTONE_HEADER=$(CAPSTONE_HEADER))
  $(info LIBC_CFLAGS=$(LIBC_CFLAGS))
  $(info GCOV_CFLAGS=$(GCOV_CFLAGS))
endif

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(@D)
	@gcc $(CFLAGS) -MMD -D__ASSEMBLY__ -c $< -o $@
	@echo "CC      $<"
