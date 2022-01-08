#!/usr/bin/make
include env.mk
C_SRC_FILES += capstone/cs.c
C_SRC_FILES += capstone/utils.c
C_SRC_FILES += capstone/SStream.c
C_SRC_FILES += capstone/MCInstrDesc.c
C_SRC_FILES += capstone/MCRegisterInfo.c
C_SRC_FILES += capstone/arch/X86/X86DisassemblerDecoder.c
C_SRC_FILES += capstone/arch/X86/X86Disassembler.c
C_SRC_FILES += capstone/arch/X86/X86IntelInstPrinter.c
C_SRC_FILES += capstone/arch/X86/X86ATTInstPrinter.c
C_SRC_FILES += capstone/arch/X86/X86Mapping.c
C_SRC_FILES += capstone/arch/X86/X86Module.c
C_SRC_FILES += capstone/MCInst.c

C_OBJ_FILES = $(C_SRC_FILES:%.c=$(BUILD_DIR)/%.o)
dependency_files = $(C_OBJ_FILES:%.o=%.d)

$(LIB_CAPSTONE): $(C_OBJ_FILES)
	@rm -f $@
	@ar rc $@ $^
	@ranlib $@
	@echo "AR      $@"

-include $(dependency_files)

$(BUILD_DIR)/%.o : %.c
	@mkdir -p $(@D)
	@gcc $(CFLAGS) -DCAPSTONE_HAS_X86 $(CAPSTONE_HEADER) -MMD -c $< -o $@
	@echo "CC      $<"
