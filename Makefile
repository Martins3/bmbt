#!/usr/bin/make
include env.mk

bmbt := $(BUILD_DIR)/bmbt.bin
LFLAGS := $(GCOV_LFLAGS) $(GLIB_LIB) -lrt -lm

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

# ASM_SRC_FILES := src/head.S

C_SRC_FILES = $(ENV_SRC_FILES)
C_SRC_FILES += $(wildcard src/*.c)
C_SRC_FILES += $(wildcard src/tcg/*.c)
C_SRC_FILES += $(wildcard src/hw/*/*.c)
C_SRC_FILES += $(wildcard src/fpu/*.c)
C_SRC_FILES += $(wildcard src/qemu/*.c)
C_SRC_FILES += $(wildcard src/util/*.c)
C_SRC_FILES += $(wildcard src/test/*.c)
C_SRC_FILES += $(wildcard src/i386/*.c)
C_SRC_FILES += $(wildcard glib/*.c)

BIN_FILES = $(wildcard image/*.bin)

CONFIG_LATX=y
CONFIG_SOFTMMU=y
include ./src/i386/Makefile.objs
LATX_SRC=$(addprefix src/i386/, $(obj-y))
C_SRC_FILES += $(LATX_SRC)

OBJ_FILES := $(C_SRC_FILES:%.c=$(BUILD_DIR)/%.o)
OBJ_FILES += $(ASM_SRC_FILES:%.S=$(BUILD_DIR)/%.o)
dependency_files = $(OBJ_FILES:%.o=%.d)

ifeq ($(USE_ULIBC_FILE), 1)
  OBJ_FILES += $(BIN_FILES:%.bin=$(BUILD_DIR)/%.o)
endif

all: check-and-reinit-submodules check-pre-commit $(bmbt)

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
		echo "see ./install-contri-check.sh"; \
		# exit 1; \
	fi

CFLAGS += $(GLIB_HEADER) $(CAPSTONE_HEADER) $(GCOV_CFLAGS) -I./include
$(BUILD_DIR)/%.o : %.c
	@mkdir -p $(@D)
	@gcc $(CFLAGS) -MMD -c $< -o $@
	@echo "CC      $<"

$(BUILD_DIR)/image/%.o : image/%.bin
	@mkdir -p $(@D)
	ld -r -b binary -o $@ $<

$(bmbt) : $(OBJ_FILES)
		@$(MAKE) -f capstone.mk
		@if [ $(USE_LIBC) != 1 ]; then \
			$(MAKE) -f libc/libc.mk; \
			ld $(FS_SYSCALL_WRAP) -T src/linker.ld -o $(bmbt) $(OBJ_FILES) $(LIB_CAPSTONE) $(LIB_C) /usr/lib/gcc/loongarch64-linux-gnu/8/libgcc.a; \
		else \
			gcc $(FS_SYSCALL_WRAP) $(OBJ_FILES) $(LIB_CAPSTONE) -o $(bmbt) $(LFLAGS) ;\
		fi
		@echo "Link      $@"

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


QEMU=~/core/ld/qemu_bak/mybuild/loongson-softmmu/qemu-system-loongson
DEF = ../../qemu_bak/vmlinux

run: clear_gcda all
	@# $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -bmbt $(bmbt)
	@# only test work in process
	$(bmbt) -s wip

test: all clear_gcda
	$(bmbt)

gdb: all
	@#gdb --args $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -bmbt $(DEF)
	gdb --args $(bmbt)

defrun: $(bmbt)
	 $(QEMU) -m 1024 -M ls3a5k -d in_asm,out_asm -D log.txt -monitor stdio -bmbt $(bmbt) $(DEF)
