// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Loongson Technology Corporation Limited
 */

#include <asm/cacheflush.h>
#include <asm/loongarchregs.h>
#include <asm/setup.h>
#include <larchintrin.h>
#include <linux/compiler_types.h>
#include <linux/sizes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void handle_ade(void);
extern void handle_ale(void);
extern void handle_sys(void);
extern void handle_sys_wrap(void);
extern void handle_bp(void);
extern void handle_ri(void);
extern void handle_fpu(void);
extern void handle_fpe(void);
extern void handle_lbt(void);
extern void handle_lsx(void);
extern void handle_lasx(void);
extern void handle_reserved(void);
extern void handle_watch(void);
extern void tlb_do_page_fault_protect(void);
extern void *vector_table[];

void *exception_table[EXCCODE_INT_START] = {
#ifdef TMP_TODO
    [0 ... EXCCODE_INT_START - 1] = handle_reserved,

    [EXCCODE_TLBL] = handle_tlb_load,
    [EXCCODE_TLBS] = handle_tlb_store,
    [EXCCODE_TLBI] = handle_tlb_load,
    [EXCCODE_TLBM] = handle_tlb_modify,
    [EXCCODE_TLBRI... EXCCODE_TLBPE] = tlb_do_page_fault_protect,
    [EXCCODE_ADE] = handle_ade,
    [EXCCODE_ALE] = handle_ale,
    [EXCCODE_SYS] = handle_sys_wrap,
    [EXCCODE_BP] = handle_bp,
    [EXCCODE_INE] = handle_ri,
    [EXCCODE_IPE] = handle_ri,
    [EXCCODE_FPDIS] = handle_fpu,
    [EXCCODE_LSXDIS] = handle_lsx,
    [EXCCODE_LASXDIS] = handle_lasx,
    [EXCCODE_FPE] = handle_fpe,
    [EXCCODE_WATCH] = handle_watch,
    [EXCCODE_BTDIS] = handle_lbt,
#endif
};

static inline void setup_vint_size(unsigned int size) {
  unsigned int vs;

  if (size == VECSIZE) {
    vs = 7;
  } else {
    abort();
  }
  csr_xchgl(vs << CSR_ECFG_VS_SHIFT, CSR_ECFG_VS, LOONGARCH_CSR_ECFG);
}

unsigned long eentry;
unsigned long tlbrentry;

static vi_handler_t ip_handlers[EXCCODE_INT_NUM];
void do_vi(int irq) {
  vi_handler_t action;

  action = ip_handlers[irq];
  if (action)
    action(irq);
  else {
    printf("vi handler[%d] is not installed\n", irq);
    abort();
  }
}

void set_vi_handler(int n, vi_handler_t addr) {
  if ((n < EXCCODE_INT_START) || (n >= EXCCODE_INT_END)) {
    printf("set invalid vector handler[%d] \n", n);
    abort();
    return;
  }

  ip_handlers[n - EXCCODE_INT_START] = addr;
}

extern void kernel_tlb_init(int cpu);

long exception_handlers[VECSIZE * 128 / sizeof(long)] __aligned(SZ_64K);

static void configure_exception_vector(void) {
  eentry = (unsigned long)exception_handlers;
  tlbrentry = (unsigned long)exception_handlers + 80 * VECSIZE;

  csr_writeq(eentry, LOONGARCH_CSR_EENTRY);
  csr_writeq(tlbrentry, LOONGARCH_CSR_TLBRENTRY);
  csr_writeq(eentry, LOONGARCH_CSR_MERRENTRY);
}

void per_cpu_trap_init(int cpu) {
  setup_vint_size(VECSIZE);
  configure_exception_vector();

  kernel_tlb_init(cpu);
  cpu_cache_init();
}

/* Install CPU exception handler */
void set_handler(unsigned long offset, void *addr, unsigned long size) {
  memcpy((void *)(eentry + offset), addr, size);
  local_flush_icache_range(eentry + offset, eentry + offset + size);
}

void set_tlb_handler(void) {
  int i;

  /* Initialise exception handlers */
  for (i = 0; i < 64; i++)
    set_handler(i * VECSIZE, handle_reserved, VECSIZE);

  for (i = EXCCODE_TLBL; i <= EXCCODE_TLBPE; i++)
    set_handler(i * VECSIZE, exception_table[i], VECSIZE);
}

void trap_init(void) {
  unsigned long i;
  void *vec_start;

  /* set interrupt vector handler */
  for (i = EXCCODE_INT_START; i < EXCCODE_INT_END; i++) {
    vec_start = vector_table[i - EXCCODE_INT_START];
    set_handler(i * VECSIZE, vec_start, VECSIZE);
  }

  for (i = EXCCODE_ADE; i <= EXCCODE_WATCH; i++)
    set_handler(i * VECSIZE, exception_table[i], VECSIZE);

  // TMP_TODO 暂时不去处理的函数，当
  // cache_error_setup();
}
