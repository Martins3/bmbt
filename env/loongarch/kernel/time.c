#include <asm/cpu-features.h>
#include <asm/loongarchregs.h>
#include <asm/ptrace.h>
#include <assert.h>
#include <stdio.h>

static int constant_set_state_oneshot() {
  unsigned long timer_config;
  timer_config = csr_readq(LOONGARCH_CSR_TCFG);
  timer_config &= ~CSR_TCFG_EN;
  timer_config &= ~CSR_TCFG_PERIOD;
  csr_writeq(timer_config, LOONGARCH_CSR_TCFG);
  return 0;
}

int constant_timer_next_event(unsigned long delta) {
  // printf("huxueshi:%s %lx\n", __FUNCTION__, delta);
  // backtrace(NULL);
  // assert(0);
  unsigned long timer_config;
  if (delta % 4 != 0 || delta == 0) {
    assert(0);
    backtrace(NULL);
  }

  delta &= CSR_TCFG_VAL;
  timer_config = delta | CSR_TCFG_EN;
  timer_config &= ~CSR_TCFG_PERIOD;
  // timer_config |= CSR_TCFG_PERIOD;
  csr_writeq(timer_config, LOONGARCH_CSR_TCFG);

  return 0;
}

void time_init(void) {
  write_csr_tintclear(CSR_TINTCLR_TI);
  constant_set_state_oneshot();

  // 下面应该是用于实现 clocktime 的部分吧
#ifdef TMP_TODO
  if (!cpu_has_cpucfg)
    const_clock_freq = cpu_clock_freq;
  else
    const_clock_freq = calc_const_freq();

  init_offset = -(drdtime() - csr_readq(LOONGARCH_CSR_CNTC));
#endif

#ifdef BMBT
  constant_clockevent_init();
  constant_clocksource_init();

  pv_time_init();
#endif
}
