#ifndef CPU_COMMON_H_OF49R186
#define CPU_COMMON_H_OF49R186
#include "../types.h"

enum device_endian {
  DEVICE_NATIVE_ENDIAN,
  DEVICE_BIG_ENDIAN,
  DEVICE_LITTLE_ENDIAN,
};

/* address in the RAM (different from a physical address) */
#if defined(CONFIG_XEN_BACKEND)
typedef uint64_t ram_addr_t;
#define RAM_ADDR_MAX UINT64_MAX
#define RAM_ADDR_FMT "%" PRIx64
#else
typedef uintptr_t ram_addr_t;
#define RAM_ADDR_MAX UINTPTR_MAX
#define RAM_ADDR_FMT "%" PRIxPTR
#endif

#if defined(HOST_WORDS_BIGENDIAN)
#define DEVICE_HOST_ENDIAN DEVICE_BIG_ENDIAN
#else
#define DEVICE_HOST_ENDIAN DEVICE_LITTLE_ENDIAN
#endif

ram_addr_t qemu_ram_addr_from_host(void *ptr);

extern ram_addr_t ram_size;

#endif /* end of include guard: CPU_COMMON_H_OF49R186 */
