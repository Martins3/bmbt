#ifndef DEVICE_H_V9602GXP
#define DEVICE_H_V9602GXP
#include <asm/addrspace.h>
#include <asm/mach-la64/loongson.h>

#define LS_ISA_SERIAL_IO_BASE ((void *)TO_UNCAC(LOONGSON_REG_BASE + 0x1e0))
#define LS_ISA_DEBUGCON_IO_BASE                                                \
  ((void *)TO_UNCAC(LOONGSON_REG_BASE + 0x1e0 + 0x100))
/* 0000000020000000-0000000027ffffff (prio 0, i/o): pcie-mmcfg-mmio */
#define LS_PCI_CONFIG_IO_BASE ((void *)TO_UNCAC(0x20000000))

#endif /* end of include guard: DEVICE_H_V9602GXP */
