#include <assert.h>
#include <env/device.h>
#include <env/memory.h>
#include <sys/mman.h>
#include <uglib.h>

void setup_serial_pass_through() { g_assert_not_reached(); }
void pci_pass_through_write(uint32_t addr, uint32_t val, int l) {
  g_assert_not_reached();
}
uint32_t pci_pass_through_read(uint32_t addr, int l) { g_assert_not_reached(); }
void pci_pass_through_init() { g_assert_not_reached(); }

void *alloc_ram(hwaddr size) {
  // (qemu) qemu_ram_mmap size=0x180200000 flags=0x22 guardfd=-1
  void *host = mmap(0, size, PROT_EXEC | PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(host != (void *)-1);
  return host;
}

void cpu_wait(void) {}
