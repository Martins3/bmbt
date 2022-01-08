#include <assert.h>
#include <exec/hwaddr.h>
#include <sys/mman.h>
#include <unistd.h>

void *alloc_ram(hwaddr size) {
  // (qemu) qemu_ram_mmap size=0x180200000 flags=0x22 guardfd=-1
  void *host = mmap(0, size, PROT_EXEC | PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(host != (void *)-1);
  return host;
}
