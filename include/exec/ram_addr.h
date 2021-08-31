#ifndef RAM_ADDR_H_ACNMERX5
#define RAM_ADDR_H_ACNMERX5
#include "cpu-common.h"
#include "ramlist.h"

typedef struct RAMBlock {
  uint8_t *host;

} RAMBlock;

/* Note: start and end must be within the same ram block.  */
static inline void cpu_physical_memory_test_and_clear_dirty(ram_addr_t start,
                                                            ram_addr_t length,
                                                            unsigned client) {
  // FIXME
}

static inline void cpu_physical_memory_set_dirty_flag(ram_addr_t addr,
                                                      unsigned client) {
  // FIXME
}

static inline bool cpu_physical_memory_get_dirty_flag(ram_addr_t addr,
                                                      unsigned client) {
  // FIXME
  return false;
}

void cpu_physical_memory_set_dirty_range(ram_addr_t start, ram_addr_t length,
                                         uint8_t mask) {
  // FIXME
}

static inline bool cpu_physical_memory_is_clean(ram_addr_t addr) {
  bool vga = cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_VGA);
  bool code = cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_CODE);
  bool migration =
      cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_MIGRATION);
  return !(vga && code && migration);
}

#endif /* end of include guard: RAM_ADDR_H_ACNMERX5 */
