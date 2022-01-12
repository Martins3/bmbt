#!/usr/bin/make
include env.mk

C_SRC_FILES+=$(wildcard libc/src/*/*.c)
C_SRC_FILES+=$(wildcard libc/src/loongarch.c)
C_SRC_FILES+=$(wildcard libc/src/malloc/mallocng/*.c)
ASM_SRC_FILES := $(wildcard libc/src/bits/loongarch/*.S)
ASM_SRC_FILES += $(wildcard libc/src/signal/loongarch/restore.S)

C_OBJ_FILES = $(C_SRC_FILES:%.c=$(BUILD_DIR)/%.o)
C_OBJ_FILES += $(ASM_SRC_FILES:%.S=$(BUILD_DIR)/%.o)

dependency_files = $(C_OBJ_FILES:%.o=%.d)

$(LIB_C): $(C_OBJ_FILES)
	@rm -f $@
	@ar rc $@ $^
	@ranlib $@
	@echo "AR      $@"

-include $(dependency_files)

LIBC_INTERNAL_HEADER= -I$(BASE_DIR)/libc/src/include -I$(BASE_DIR)/libc/src/internal
FLAGS= $(LIBC_INTERNAL_HEADER) $(CFLAGS)

EXPAND_MACAO=0
$(BUILD_DIR)/%.o : %.c
	@mkdir -p $(@D)
	@if [[ $(EXPAND_MACAO) == 1 ]];then \
		echo "expand"; \
		gcc $(FLAGS) -MMD -c $< -E -o /tmp/macro.c; \
		exit 1; \
	else \
		gcc $(FLAGS) -MMD -c $< -o $@; \
	fi
	@echo "CC      $<"

# ------------------------------- Test -----------------------------------------
# ------------------------------------------------------------------------------
TEST_C_SRC=$(wildcard libc/test/*.c)
TEST_OBJ_FILES=$(TEST_C_SRC:%.c=$(BUILD_DIR)/%.o)
deps = $(TEST_OBJ_FILES:%.o=%.d)
-include $(deps)

test_bin=$(BUILD_DIR)/libc/test.out

check_env:
	@if [[ $(ENV_KERNEL) == 1 ]]; then \
		echo "please test libc in userspace"; \
		exit 1;\
	fi

run: build
	$(test_bin)

gdb: build
	gdb $(test_bin) -ex 'run'

build: check_env $(LIB_C) $(TEST_OBJ_FILES)
	@ld -o $(test_bin) $(TEST_OBJ_FILES) $(LIB_C) /usr/lib/gcc/loongarch64-linux-gnu/8/libgcc.a;
	@echo "Link      $<"

$(BUILD_DIR)/libc/test/%.o: libc/test/%.c
	@mkdir -p $(@D)
	@gcc -I$(BASE_DIR)/include $(FLAGS) -MMD -c $< -o $@
	@echo "CC      $<"
