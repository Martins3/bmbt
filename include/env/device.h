#ifndef PASSTHROUGH_H_F8VJ4FXW
#define PASSTHROUGH_H_F8VJ4FXW

#include <stdint.h>

void setup_serial_pass_through();
void pci_pass_through_write(uint32_t addr, uint32_t val, int l);
uint32_t pci_pass_through_read(uint32_t addr, int l);
void pci_pass_through_init();
void loongarch_pci_init();
#endif /* end of include guard: PASSTHROUGH_H_F8VJ4FXW */
