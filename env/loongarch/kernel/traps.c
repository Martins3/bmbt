// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Loongson Technology Corporation Limited
 */

#include <asm/cacheflush.h>
#include <asm/loongarchregs.h>
#include <asm/ptrace.h>
#include <asm/setup.h>
#include <linux/compiler_types.h>
#include <linux/sizes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAMT
#include <asm/tlb.h>
extern void tlb_do_page_fault_protect(void);
#endif

extern void handle_reserved(void);
extern void *vector_table[];

void *exception_table[EXCCODE_INT_START] = {
    [0 ... EXCCODE_INT_START - 1] = handle_reserved,
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
unsigned long tlbrentry_end;

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

  printf("vi handler[%d] installed\n", n - EXCCODE_INT_START);

  ip_handlers[n - EXCCODE_INT_START] = addr;
}

extern void kernel_tlb_init(int cpu);
extern void cache_error_setup(void);

long exception_handlers[VECSIZE * 128 / sizeof(long)] __aligned(SZ_64K);

static void configure_exception_vector(void) {
  eentry = (unsigned long)exception_handlers;
  tlbrentry = (unsigned long)exception_handlers + 80 * VECSIZE;
  tlbrentry_end =
      (unsigned long)exception_handlers + sizeof(exception_handlers);

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

/*
 * Install uncached CPU exception handler.
 * This is suitable only for the cache error exception which is the only
 * exception handler that is being run uncached.
 */
void set_merr_handler(unsigned long offset, void *addr, unsigned long size) {
  unsigned long uncached_eentry = TO_UNCAC(TO_PHYS(eentry));

  if (!addr)
    abort();

  memcpy((void *)(uncached_eentry + offset), addr, size);
}

void set_tlb_handler(void) {
  int i;

  /* Initialise exception handlers */
  for (i = 0; i < 64; i++)
    set_handler(i * VECSIZE, handle_reserved, VECSIZE);

#ifndef HAMT
  for (i = EXCCODE_TLBL; i <= EXCCODE_TLBPE; i++)
    set_handler(i * VECSIZE, exception_table[i], VECSIZE);
#else
  set_handler(EXCCODE_TLBL * VECSIZE, handle_tlb_load, VECSIZE);
  set_handler(EXCCODE_TLBS * VECSIZE, handle_tlb_store, VECSIZE);
  set_handler(EXCCODE_TLBI * VECSIZE, handle_tlb_load, VECSIZE);
  set_handler(EXCCODE_TLBM * VECSIZE, handle_tlb_modify, VECSIZE);
  // set_handler(EXCCODE_TLBRI * VECSIZE, tlb_do_page_fault_protect, VECSIZE);
  // set_handler(EXCCODE_TLBXI * VECSIZE, tlb_do_page_fault_protect, VECSIZE);
  // set_handler(EXCCODE_TLBPE * VECSIZE, tlb_do_page_fault_protect, VECSIZE);
#endif
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

  cache_error_setup();
}

static void show_regs(struct pt_regs *regs, long is_tlbr) {
  const int field = 2 * sizeof(unsigned long);
  unsigned int excsubcode;
  unsigned int exccode;
  int i;

  if (is_tlbr) {
    printf("TLB refill exception\n");
  } else {
    exccode = ((regs->csr_estat) & CSR_ESTAT_EXC) >> CSR_ESTAT_EXC_SHIFT;
    excsubcode =
        ((regs->csr_estat) & CSR_ESTAT_ESUBCODE) >> CSR_ESTAT_ESUBCODE_SHIFT;
    printf("ExcCode : %x (SubCode %x)\n", exccode, excsubcode);

    if (exccode >= EXCCODE_TLBL && exccode <= EXCCODE_ALE)
      printf("BadVA : %0*lx\n", field, regs->csr_badv);
  }

  /*
   * Saved main processor registers
   */
  for (i = 0; i < 32;) {
    if ((i % 4) == 0)
      printf("$%2d   :", i);
    printf(" %0*lx", field, regs->regs[i]);

    i++;
    if ((i % 4) == 0)
      printf("\n");
  }

  printf("ra    : %0*lx %pS\n", field, regs->regs[1], (void *)regs->regs[1]);
  printf("CSR crmd: %08lx ", regs->csr_crmd);
  if (is_tlbr) {
    printf("CSR tlbprmd: %08lx  ", regs->csr_tlbrprmd);
  } else {
    printf("CSR prmd: %08lx ", regs->csr_prmd);
  }
  printf("CSR ecfg: %08lx ", regs->csr_ecfg);
  printf("CSR estat: %08lx  ", regs->csr_estat);
  printf("CSR euen: %08lx ", regs->csr_euen);
  printf("CSR badi: %08lx ", csr_readq(LOONGARCH_CSR_BADI));
  printf("CSR badv: %016lx ", csr_readq(LOONGARCH_CSR_BADV));
  printf("\n");
}

void do_reserved(struct pt_regs *regs, long is_tlbr) {
  /*
   * Game over - no way to handle this if it ever occurs.  Most probably
   * caused by a new unknown cpu type or after another deadly
   * hard/software error.
   */
  show_regs(regs, is_tlbr);
  abort();
}
