#include "../../../include/hw/i386/pc.h"

void pc_system_firmware_init(PCMachineState *pcms, MemoryRegion *rom_memory) {
  /* Machine property pflash0 not set, use ROM mode */
  x86_bios_rom_init(rom_memory, false);
}
