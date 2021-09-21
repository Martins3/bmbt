#include "../../include/sysemu/dma.h"
#include <string.h>

int dma_memory_set(AddressSpace *as, dma_addr_t addr, uint8_t c,
                   dma_addr_t len) {
  dma_barrier(as, DMA_DIRECTION_FROM_DEVICE);

#define FILLBUF_SIZE 512
  uint8_t fillbuf[FILLBUF_SIZE];
  int l;
  bool error = false;

  memset(fillbuf, c, FILLBUF_SIZE);
  while (len > 0) {
    l = len < FILLBUF_SIZE ? len : FILLBUF_SIZE;
    error |=
        address_space_rw(as, addr, MEMTXATTRS_UNSPECIFIED, fillbuf, l, true);
    len -= l;
    addr += l;
  }

  return error;
}
