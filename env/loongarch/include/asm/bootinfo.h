#ifndef BOOTINFO_H_TYIJUDWF
#define BOOTINFO_H_TYIJUDWF

extern void platform_init(void);
/*
 * Registers a0, a1, a3 and a4 as passed to the kernel entry by firmware
 */
extern unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;

#endif /* end of include guard: BOOTINFO_H_TYIJUDWF */
