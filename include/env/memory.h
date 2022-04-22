#ifndef MEM_H_Q2JOQR86
#define MEM_H_Q2JOQR86
#include <exec/hwaddr.h>
#include <inttypes.h>

#define MAX_PC_RAM_NUM 10
typedef struct {
  uint64_t start;
  uint64_t end;
} RamRange;
int get_guest_ram_num();
RamRange guest_ram(int idx);
uint64_t get_guest_total_ram();
uint64_t get_host_offset();
#endif /* end of include guard: MEM_H_Q2JOQR86 */
