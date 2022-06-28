#ifndef DEVICE_H_V9602GXP
#define DEVICE_H_V9602GXP
#include <asm/addrspace.h>
#include <asm/mach-la64/loongson.h>

#define LS_ISA_SERIAL_IO_BASE ((void *)TO_UNCAC(LOONGSON_REG_BASE + 0x1e0))
#define LS_ISA_DEBUGCON_IO_BASE                                                \
  ((void *)TO_UNCAC(LOONGSON_REG_BASE + 0x1e0 + 0x100))
/* 0000000020000000-0000000027ffffff (prio 0, i/o): pcie-mmcfg-mmio */
#define QEMU_PCI_CONFIG_IO_BASE ((void *)TO_UNCAC(0x20000000))
/* arch/loongarch/include/asm/mach-la64/loongson-pch.h */
#define MCFG_EXT_PCICFG_BASE ((void *)TO_UNCAC(0xefe00000000UL))

/* seabios/src/config.h */
#define BUILD_PCIMEM_START 0xe0000000
#define BUILD_PCIMEM_END 0xfec00000 /* IOAPIC is mapped at */

/* seabios/src/fw/pciinit.c:pci_bios_init_root_regions_io */
#define X86_PCI_IO_START 0xc000
#define X86_PCI_IO_END 0xffff

/* arch/loongarch/include/asm/mach-la64/pci.h */
#define LOONGSON_PCI_MEM_START 0x40000000
#define LOONGSON_PCI_MEM_END 0x7fffffff
#define LOONGSON_PCI_IO_START 0x00004000

#define LOONGSON_X86_PCI_MEM_OFFSET                                            \
  (BUILD_PCIMEM_START - LOONGSON_PCI_MEM_START - 0x20000000)
#define LOONGSON_X86_PCI_IO_OFFSET (X86_PCI_IO_START - LOONGSON_PCI_IO_START)

// [interface 63]
#define X86_MSI_ADDR_BASE_LO 0xfee0100c
#define X86_MSI_ENTRY_DATA_FLAG 0x4100

// defined in arch/x86/boot/early_serial_console.c
#define DLAB 0x80

#define TXR 0 /*  Transmit register (WRITE) */
#define RXR 0 /*  Receive register  (READ)  */
#define IER 1 /*  Interrupt Enable          */
#define IIR 2 /*  Interrupt ID              */
#define FCR 2 /*  FIFO control              */
#define LCR 3 /*  Line control              */
#define MCR 4 /*  Modem control             */
#define LSR 5 /*  Line Status               */
#define MSR 6 /*  Modem Status              */
#define DLL 0 /*  Divisor Latch Low         */
#define DLH 1 /*  Divisor latch High        */

#endif /* end of include guard: DEVICE_H_V9602GXP */
