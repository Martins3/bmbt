#ifndef RAM_ADDR_H_ACNMERX5
#define RAM_ADDR_H_ACNMERX5
#include "cpu-common.h"

// FIXME this file, along with memory.h ram_addr.h
// will be redesinged
typedef struct RAMBlock {

} RAMBlock;

// FIXME defined in exec.c
static inline void cpu_physical_memory_set_dirty_range(ram_addr_t start,
                                                       ram_addr_t length,
                                                       uint8_t mask);

/* Note: start and end must be within the same ram block.  */
bool cpu_physical_memory_test_and_clear_dirty(ram_addr_t start,
                                              ram_addr_t length,
                                              unsigned client);

static inline void cpu_physical_memory_set_dirty_flag(ram_addr_t addr,
                                                      unsigned client);

static inline bool cpu_physical_memory_get_dirty_flag(ram_addr_t addr,
                                                      unsigned client);


static inline void cpu_physical_memory_set_dirty_range(ram_addr_t start,
                                                       ram_addr_t length,
                                                       uint8_t mask);

#endif /* end of include guard: RAM_ADDR_H_ACNMERX5 */
