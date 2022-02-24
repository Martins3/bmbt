#ifndef UUID_H_ZBJY6R7X
#define UUID_H_ZBJY6R7X

#include <stdint.h>

/* Version 4 UUID (pseudo random numbers), RFC4122 4.4. */

typedef struct {
  union {
    unsigned char data[16];
    struct {
      /* Generated in BE endian, can be swapped with qemu_uuid_bswap. */
      uint32_t time_low;
      uint16_t time_mid;
      uint16_t time_high_and_version;
      uint8_t clock_seq_and_reserved;
      uint8_t clock_seq_low;
      uint8_t node[6];
    } fields;
  };
} QemuUUID;
#endif /* end of include guard: UUID_H_ZBJY6R7X */
