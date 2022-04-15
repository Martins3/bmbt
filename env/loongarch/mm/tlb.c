#include <asm/cacheflush.h>
#include <asm/loongarchregs.h>
#include <asm/page.h>
#include <asm/setup.h>
#include <asm/tlb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void local_flush_tlb_all(void) { invtlb_all_atomic(INVTLB_CURRENT_ALL, 0, 0); }

static void build_tlb_handler(int cpu) {

  if (cpu == 0) {
    long len = tlbrentry_end - tlbrentry;
    memcpy((void *)tlbrentry, handle_tlb_refill, len);
    local_flush_icache_range(tlbrentry, tlbrentry + len);
    set_tlb_handler();

#ifdef BMBT
    /* initial swapper_pg_dir so that refill can work */
    pagetable_init();
#endif
  }

#ifdef BMBT
  setup_pw();
  csr_writeq((long)swapper_pg_dir, LOONGARCH_CSR_PGDH);
  csr_writeq((long)invalid_pgd, LOONGARCH_CSR_PGDL);
  csr_writeq((long)smp_processor_id(), LOONGARCH_CSR_TMID);
#endif
}

void kernel_tlb_init(int cpu) {
  write_csr_pagesize(PS_DEFAULT_SIZE);
  // write_csr_stlbpgsize(PS_DEFAULT_SIZE);
  write_csr_stlbpgsize(PS_4K);
  write_csr_tlbrefill_pagesize(PS_DEFAULT_SIZE);

  if (read_csr_pagesize() != PS_DEFAULT_SIZE) {
    printf("MMU doesn't support PAGE_SIZE=0x%lx", PAGE_SIZE);
    abort();
  }

  build_tlb_handler(cpu);
  local_flush_tlb_all();
}
