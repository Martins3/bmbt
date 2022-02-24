#ifndef TIME_H_WL4XY2K7
#define TIME_H_WL4XY2K7
#include <asm/loongarchregs.h>

static inline unsigned int calc_const_freq(void) {
  unsigned int res;
  unsigned int base_freq;
  unsigned int cfm, cfd;

  res = read_cpucfg(LOONGARCH_CPUCFG2);
  if (!(res & CPUCFG2_LLFTP))
    return 0;

  base_freq = read_cpucfg(LOONGARCH_CPUCFG4);
  res = read_cpucfg(LOONGARCH_CPUCFG5);
  cfm = res & 0xffff;
  cfd = (res >> 16) & 0xffff;

  if (!base_freq || !cfm || !cfd)
    return 0;
  else
    return (base_freq * cfm / cfd);
}

void time_init(void);
int constant_timer_next_event(unsigned long delta);
#endif /* end of include guard: TIME_H_WL4XY2K7 */
