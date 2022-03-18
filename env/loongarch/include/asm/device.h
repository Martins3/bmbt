#ifndef DEVICE_H_V9602GXP
#define DEVICE_H_V9602GXP
#include <asm/addrspace.h>
#include <asm/mach-la64/loongson.h>

#define LS_ISA_SERIAL_IO_BASE ((void *)TO_UNCAC(LOONGSON_REG_BASE + 0x1e0))
#define LS_ISA_DEBUGCON_IO_BASE                                                \
  ((void *)TO_UNCAC(LOONGSON_REG_BASE + 0x1e0 + 0x100))
/* 0000000020000000-0000000027ffffff (prio 0, i/o): pcie-mmcfg-mmio */
#define LS_PCI_CONFIG_IO_BASE ((void *)TO_UNCAC(0x20000000))

/* macros in seabios/src/config.h start */
#define BUILD_PCIMEM_START 0xe0000000
#define BUILD_PCIMEM_END 0xfec00000 /* IOAPIC is mapped at */
/* macros in seabios/src/config.h end */
#define X86_PCI_IO_START 0xc000
#define X86_PCI_IO_END 0xffff

/* define arch/loongarch/include/asm/mach-la64/pci.h start */
#define LOONGSON_PCI_MEM_START 0x40000000UL
#define LOONGSON_PCI_MEM_END 0x7fffffffUL
#define LOONGSON_PCI_IO_START 0x00004000UL
/* define arch/loongarch/include/asm/mach-la64/pci.h end */

#define LOONGSON_X86_PCI_MEM_OFFSET                                            \
  (BUILD_PCIMEM_START - LOONGSON_PCI_MEM_START)
#define LOONGSON_X86_PCI_IO_OFFSET (X86_PCI_IO_START - LOONGSON_PCI_IO_START)

#endif /* end of include guard: DEVICE_H_V9602GXP */
