#!/usr/bin/make
include make/env.mk
C_SRC_FILES += $(CAPSTONE_DIR)/cs.c
C_SRC_FILES += $(CAPSTONE_DIR)/utils.c
C_SRC_FILES += $(CAPSTONE_DIR)/SStream.c
C_SRC_FILES += $(CAPSTONE_DIR)/MCInst.c
C_SRC_FILES += $(CAPSTONE_DIR)/MCInstrDesc.c
C_SRC_FILES += $(CAPSTONE_DIR)/MCRegisterInfo.c
C_SRC_FILES += $(CAPSTONE_DIR)/arch/X86/X86DisassemblerDecoder.c
C_SRC_FILES += $(CAPSTONE_DIR)/arch/X86/X86Disassembler.c
C_SRC_FILES += $(CAPSTONE_DIR)/arch/X86/X86IntelInstPrinter.c
C_SRC_FILES += $(CAPSTONE_DIR)/arch/X86/X86ATTInstPrinter.c
C_SRC_FILES += $(CAPSTONE_DIR)/arch/X86/X86Mapping.c
C_SRC_FILES += $(CAPSTONE_DIR)/arch/X86/X86Module.c

C_OBJ_FILES = $(C_SRC_FILES:%.c=$(BUILD_DIR)/%.o)
dependency_files = $(C_OBJ_FILES:%.o=%.d)

# latx use auto generated capStone-defs.h with -include
# simplified them with -D
# https://stackoverflow.com/questions/13639928/is-there-any-way-to-include-a-header-file-in-all-c-or-cpp-file-automatically
CFLAGS += -DCAPSTONE_USE_SYS_DYN_MEM -DCONFIG_LATX_DEBUG -DCAPSTONE_HAS_X86

$(LIB_CAPSTONE): $(C_OBJ_FILES)
	@rm -f $@
	@ar rc $@ $^
	@ranlib $@
	@echo "AR      $@"

-include $(dependency_files)

$(BUILD_DIR)/%.o : %.c
	@mkdir -p $(@D)
	@gcc $(CFLAGS) $(CAPSTONE_HEADER) -MMD -c $< -o $@
	@echo "CC      $<"
