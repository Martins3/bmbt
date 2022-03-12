#!/usr/bin/make
include make/env.mk

bmbt := $(BUILD_DIR)/bmbt.bin
GCC_LFLAGS := $(GCOV_LFLAGS) $(GLIB_LIB) -lrt -lm
I386_FLAGS = -I./src/target/i386/LATX/include -I./src/target/i386 -I./src

# @todo it's so ugly to define two options out of the gcc option --coverage
# https://gcc.gnu.org/onlinedocs/gcc-9.3.0/gcc/Instrumentation-Options.html
# USE_LIBC=1 means use gcc
ifeq ($(USE_LIBC), 1)
  FS_SYSCALL_WRAP =-Wl,--wrap,fopen
  FS_SYSCALL_WRAP+=-Wl,--wrap,fclose
  FS_SYSCALL_WRAP+=-Wl,--wrap,fread
  FS_SYSCALL_WRAP+=-Wl,--wrap,ftell
  FS_SYSCALL_WRAP+=-Wl,--wrap,fseek
else
  FS_SYSCALL_WRAP =--wrap=fopen
  FS_SYSCALL_WRAP+=--wrap=fclose
  FS_SYSCALL_WRAP+=--wrap=fread
  FS_SYSCALL_WRAP+=--wrap=ftell
  FS_SYSCALL_WRAP+=--wrap=fseek
endif

C_SRC_FILES = $(ENV_SRC_FILES)
C_SRC_FILES += $(wildcard src/*.c)
C_SRC_FILES += $(wildcard src/tcg/*.c)
C_SRC_FILES += $(wildcard src/accel/tcg/*.c)
C_SRC_FILES += $(wildcard src/tcg/loongarch/*.c)
C_SRC_FILES += $(wildcard src/hw/*/*.c)
C_SRC_FILES += $(wildcard src/fpu/*.c)
C_SRC_FILES += $(wildcard src/qemu/*.c)
C_SRC_FILES += $(wildcard src/util/*.c)
C_SRC_FILES += $(wildcard src/test/*.c)
C_SRC_FILES += $(wildcard src/target/i386/*.c)
C_SRC_FILES += $(wildcard src/crypto/*.c)
C_SRC_FILES += $(wildcard glib/*.c)

BIN_FILES = $(wildcard image/*.bin)

# kernel files need extra flags "-O2" to eliminate the error
# "invalid argument to built-in function"
ifeq ($(ENV_KERNEL), 1)
  KERNEL_SRC_FILES += $(wildcard env/loongarch/*/*.c)
  KERNEL_SRC_FILES += $(wildcard env/loongarch/drivers/*/*.c)
  KERNEL_OBJS_FILES=$(KERNEL_SRC_FILES:%.c=$(BUILD_DIR)/%.o)
  $(KERNEL_OBJS_FILES): EXTRA_FLAGS = -O2
endif

CONFIG_LATX=y
CONFIG_SOFTMMU=y
include ./src/target/i386/Makefile.objs
LATX_SRC=$(addprefix src/target/i386/, $(obj-y))
C_SRC_FILES += $(LATX_SRC)

OBJ_FILES := $(C_SRC_FILES:%.c=$(BUILD_DIR)/%.o)
OBJ_FILES += $(ASM_SRC_FILES:%.S=$(BUILD_DIR)/%.o)
OBJ_FILES += $(KERNEL_OBJS_FILES)
dependency_files = $(OBJ_FILES:%.o=%.d)

ifeq ($(USE_ULIBC_FILE), 1)
  OBJ_FILES += $(BIN_FILES:%.bin=$(BUILD_DIR)/%.o)
endif

all: check-and-reinit-submodules check-pre-commit $(bmbt)

-include $(dependency_files)

# https://stackoverflow.com/questions/52337010/automatic-initialization-and-update-of-submodules-in-makefile
.PHONY: check-and-reinit-submodules check-pre-commit
check-and-reinit-submodules:
	@if git submodule status | egrep -q '^[-]|^[+]' ; then \
            echo "INFO: Need to reinitialize git submodules"; \
            git submodule update --init; \
	fi

check-pre-commit:
	@if [ ! -d $(BASE_DIR)/.husky/_ ];then \
		echo "please setup up precomit and husky"; \
		echo "see ./script/install-contri-check.sh"; \
		# exit 1; \
	fi

capstone:
	$(MAKE) -f make/capstone.mk

libc:
	@if [ $(USE_LIBC) != 1 ]; then \
		$(MAKE) -f libc/libc.mk; \
	fi

# Can't find a method to generate asm-offset.h automatically. Fortunately, there
# is only one rule which is based on headers included asm-offset.c
env/loongarch/include/generated/asm-offset.h: env/loongarch/asm-offset.c env/loongarch/include/asm/ptrace.h
	@@echo "GENERATE asm-offset.h"
	@mkdir -p env/loongarch/include/generated
	@mkdir -p $(BUILD_DIR)
	@gcc -Ienv/loongarch/include env/loongarch/asm-offset.c -o $(BUILD_DIR)/asm-offset.out
	@$(BUILD_DIR)/asm-offset.out

asm-offset: env/loongarch/include/generated/asm-offset.h

CFLAGS += $(GLIB_HEADER) $(CAPSTONE_HEADER) $(GCOV_CFLAGS) -I$(BASE_DIR)/include $(I386_FLAGS)
$(BUILD_DIR)/%.o : %.c
	@mkdir -p $(@D)
	@gcc $(CFLAGS) $(EXTRA_FLAGS) -MMD -c $< -o $@
	@echo "CC      $<"

$(BUILD_DIR)/image/%.o : image/%.bin
	@mkdir -p $(@D)
	ld -r -b binary -o $@ $<

$(bmbt) : asm-offset $(OBJ_FILES) libc capstone
		@if [ $(USE_LIBC) != 1 ]; then \
			ld $(LDFLAGS) $(FS_SYSCALL_WRAP) -o $(bmbt) $(OBJ_FILES) $(LIB_CAPSTONE) $(LIB_C) /usr/lib/gcc/loongarch64-linux-gnu/8/libgcc.a; \
		else \
			gcc $(FS_SYSCALL_WRAP) $(OBJ_FILES) $(LIB_CAPSTONE) $(GCC_LFLAGS) -o $(bmbt);\
		fi
		@echo "Link      $@"

.PHONY: all clean gdb run gcov clear_gcda test libc capstone asm-offset

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
	rm -rf $(BUILD_DIR)
	rm -rf env/loongarch/include/generated

clear_gcda:
	@find $(BUILD_DIR) -name "*.gcda" -type f -delete

QEMU_DIR=/home/loongson/core/centos-qemu
LA_BIOS=$(QEMU_DIR)/pc-bios/loongarch_bios.bin
LA_QEMU=$(QEMU_DIR)/build/loongarch64-softmmu/qemu-system-loongarch64
RUN_IN_QEMU=$(LA_QEMU) -m 8G -cpu Loongson-3A5000 -serial stdio -bios $(LA_BIOS) --enable-kvm -M loongson7a_v1.0,accel=kvm -kernel $(bmbt)

run: all clear_gcda
	if [[ $(ENV_KERNEL) == 1 ]];then \
		$(RUN_IN_QEMU); \
	else \
		$(bmbt); \
	fi

qemu:
	if [[ $(ENV_KERNEL) == 1 ]];then \
		# in cpus.c:qemu_cpu_kick_thread, SIG_IPI (which is SIGUSR1) will cause \
		# the gdb stopped unexpectedly \
		gdb -ex "handle SIGUSR1 nostop noprint" --args $(RUN_IN_QEMU); \
	else \
		gdb -ex "handle SIGUSR1 nostop noprint" --args $(bmbt); \
	fi

test: all clear_gcda
	$(bmbt)

gdb: all
	if [[ $(ENV_KERNEL) == 1 ]];then \
		gdb $(bmbt) -ex "target remote :1234"; \
	else \
		gdb -ex "handle SIG127 nostop noprint" -ex "run" --args $(bmbt); \
	fi

s: all
		$(RUN_IN_QEMU) -S -s;
