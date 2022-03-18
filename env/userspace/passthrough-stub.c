#include <stdint.h>
#include <uglib.h>
void setup_serial_pass_through() { g_assert_not_reached(); }
void pci_pass_through_write(uint32_t addr, uint32_t val, int l) {
  g_assert_not_reached();
}
uint32_t pci_pass_through_read(uint32_t addr, int l) { g_assert_not_reached(); }
void pci_pass_through_init() { g_assert_not_reached(); }
