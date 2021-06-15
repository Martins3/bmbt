#ifndef CPU_COMMON_H_OF49R186
#define CPU_COMMON_H_OF49R186

#include "../qemu/config-host.h"
#include "../types.h"


// FIXME what's the purpose of this header

// FIXME CONFIG_XEN_BACKEND why affect ram_addr_t ?
/* address in the RAM (different from a physical address) */
#if defined(CONFIG_XEN_BACKEND)
typedef uint64_t ram_addr_t;
#  define RAM_ADDR_MAX UINT64_MAX
#  define RAM_ADDR_FMT "%" PRIx64
#else
typedef uintptr_t ram_addr_t;
#  define RAM_ADDR_MAX UINTPTR_MAX
#  define RAM_ADDR_FMT "%" PRIxPTR
#endif



#endif /* end of include guard: CPU_COMMON_H_OF49R186 */
