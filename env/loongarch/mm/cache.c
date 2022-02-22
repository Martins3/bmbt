#include <asm/cpu-info.h>
#include <asm/cpu.h>
#include <asm/loongarchregs.h>
#include <asm/setup.h>
#include <linux/compiler_types.h>
#include <stdio.h>
#include <stdlib.h>

// [interface 56]
/* Cache operations. */
void local_flush_icache_range(unsigned long start, unsigned long end) {
  asm volatile("\tibar 0\n" ::);
}

void cache_error_setup(void) {
  extern void except_vec_cex(void);
  set_merr_handler(0x0, (void *)except_vec_cex, 0x80);
}

static unsigned long icache_size;
static unsigned long dcache_size;
static unsigned long vcache_size;
static unsigned long scache_size;

static char *way_string[] = {
    NULL,     "direct mapped", "2-way",  "3-way",  "4-way",  "5-way",
    "6-way",  "7-way",         "8-way",  "9-way",  "10-way", "11-way",
    "12-way", "13-way",        "14-way", "15-way", "16-way",
};

static void probe_pcache(void) {
  struct cpuinfo_loongarch *c = &current_cpu_data;
  unsigned int lsize, sets, ways;
  unsigned int config;

  config = read_cpucfg(LOONGARCH_CPUCFG17);
  lsize = 1 << ((config & CPUCFG17_L1I_SIZE_M) >> CPUCFG17_L1I_SIZE);
  sets = 1 << ((config & CPUCFG17_L1I_SETS_M) >> CPUCFG17_L1I_SETS);
  ways = ((config & CPUCFG17_L1I_WAYS_M) >> CPUCFG17_L1I_WAYS) + 1;

  c->icache.linesz = lsize;
  c->icache.sets = sets;
  c->icache.ways = ways;
  icache_size = sets * ways * lsize;
  c->icache.waysize = icache_size / c->icache.ways;

  config = read_cpucfg(LOONGARCH_CPUCFG18);
  lsize = 1 << ((config & CPUCFG18_L1D_SIZE_M) >> CPUCFG18_L1D_SIZE);
  sets = 1 << ((config & CPUCFG18_L1D_SETS_M) >> CPUCFG18_L1D_SETS);
  ways = ((config & CPUCFG18_L1D_WAYS_M) >> CPUCFG18_L1D_WAYS) + 1;

  c->dcache.linesz = lsize;
  c->dcache.sets = sets;
  c->dcache.ways = ways;
  dcache_size = sets * ways * lsize;
  c->dcache.waysize = dcache_size / c->dcache.ways;

  c->options |= LOONGARCH_CPU_PREFETCH;

  printf("Primary instruction cache %ldkB, %s, %s, linesize %d bytes.\n",
         icache_size >> 10, way_string[c->icache.ways], "VIPT",
         c->icache.linesz);

  printf("Primary data cache %ldkB, %s, %s, %s, linesize %d bytes\n",
         dcache_size >> 10, way_string[c->dcache.ways], "VIPT", "no aliases",
         c->dcache.linesz);

  if (c->dcache.linesz != (1 << CONFIG_L1_CACHE_SHIFT) ||
      c->icache.linesz != (1 << CONFIG_L1_CACHE_SHIFT)) {
    printf("Kconfig isn't matched with machine\n");
    abort();
  }
}

static void probe_vcache(void) {
  struct cpuinfo_loongarch *c = &current_cpu_data;
  unsigned int lsize, sets, ways;
  unsigned int config;

  config = read_cpucfg(LOONGARCH_CPUCFG19);
  lsize = 1 << ((config & CPUCFG19_L2_SIZE_M) >> CPUCFG19_L2_SIZE);
  sets = 1 << ((config & CPUCFG19_L2_SETS_M) >> CPUCFG19_L2_SETS);
  ways = ((config & CPUCFG19_L2_WAYS_M) >> CPUCFG19_L2_WAYS) + 1;

  c->vcache.linesz = lsize;
  c->vcache.sets = sets;
  c->vcache.ways = ways;
  vcache_size = lsize * sets * ways;
  c->vcache.waysize = vcache_size / c->vcache.ways;

  printf("Unified victim cache %ldkB %s, linesize %d bytes.\n",
         vcache_size >> 10, way_string[c->vcache.ways], c->vcache.linesz);
}

static void probe_scache(void) {
  struct cpuinfo_loongarch *c = &current_cpu_data;
  unsigned int lsize, sets, ways;
  unsigned int config;

  config = read_cpucfg(LOONGARCH_CPUCFG20);
  lsize = 1 << ((config & CPUCFG20_L3_SIZE_M) >> CPUCFG20_L3_SIZE);
  sets = 1 << ((config & CPUCFG20_L3_SETS_M) >> CPUCFG20_L3_SETS);
  ways = ((config & CPUCFG20_L3_WAYS_M) >> CPUCFG20_L3_WAYS) + 1;

  c->scache.linesz = lsize;
  c->scache.sets = sets;
  c->scache.ways = ways;
  /* 4 cores. scaches are shared */
  scache_size = lsize * sets * ways;
  c->scache.waysize = scache_size / c->scache.ways;

  printf("Unified secondary cache %ldkB %s, linesize %d bytes.\n",
         scache_size >> 10, way_string[c->scache.ways], c->scache.linesz);
}

void cpu_cache_init(void) {
  probe_pcache();
  probe_vcache();
  probe_scache();

#ifdef BMBT
  shm_align_mask = PAGE_SIZE - 1;
#endif
}
