#include <asm/cpu-features.h>
#include <asm/loongarchregs.h>
#include <asm/ptrace.h>
#include <asm/time.h>
#include <assert.h>
#include <stdio.h>

u64 const_clock_freq;

static int constant_set_state_oneshot() {
  unsigned long timer_config;
  timer_config = csr_readq(LOONGARCH_CSR_TCFG);
  timer_config &= ~CSR_TCFG_EN;
  timer_config &= ~CSR_TCFG_PERIOD;
  csr_writeq(timer_config, LOONGARCH_CSR_TCFG);
  return 0;
}

int constant_timer_next_event(unsigned long delta) {
  unsigned long timer_config;
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

  if (!cpu_has_cpucfg)
    assert(0);
  else
    const_clock_freq = calc_const_freq();
  printf("Const clock freq=%ld\n", const_clock_freq);
}
