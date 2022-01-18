#ifndef CPU_INFO_H_VPSRUY4J
#define CPU_INFO_H_VPSRUY4J

#include <asm/loongarchregs.h>
#include <linux/cache.h>
#include <linux/smp.h>
/*
 * Descriptor for a cache
 */
struct cache_desc {
  unsigned int waysize; /* Bytes per way */
  unsigned short sets;  /* Number of lines per set */
  unsigned char ways;   /* Number of ways */
  unsigned char linesz; /* Size of line in bytes */
  unsigned char waybit; /* Bits to select in a cache set */
  unsigned char flags;  /* Flags describing cache properties */
};

struct guest_info {
  unsigned long ases;
  unsigned long ases_dyn;
  unsigned long long options;
  unsigned long long options_dyn;
  int tlbsize;
  u8 conf;
  unsigned int kscratch_mask;
};

struct cpuinfo_loongarch {
  u64 asid_cache;
  unsigned long asid_mask;

  /*
   * Capability and feature descriptor structure for LoongArch CPU
   */
  unsigned long ases;
  unsigned long long options;
  unsigned int udelay_val;
  unsigned int processor_id;
  unsigned int fpu_vers;
  unsigned int fpu_csr0;
  unsigned int fpu_mask;
  unsigned int cputype;
  int isa_level;
  int tlbsize;
  int tlbsizemtlb;
  int tlbsizestlbsets;
  int tlbsizestlbways;
  struct cache_desc icache; /* Primary I-cache */
  struct cache_desc dcache; /* Primary D or combined I/D cache */
  struct cache_desc vcache; /* Victim cache, between pcache and scache */
  struct cache_desc scache; /* Secondary cache */
  struct cache_desc tcache; /* Tertiary/split secondary cache */
  int package;              /* physical package number */
  unsigned int globalnumber;
  int vabits;                     /* Virtual Address size in bits */
  int pabits;                     /* Physical Address size in bits */
  void *data;                     /* Additional data */
  unsigned int watch_dreg_count;  /* Number data breakpoints */
  unsigned int watch_ireg_count;  /* Number instruction breakpoints */
  unsigned int watch_reg_use_cnt; /* min(NUM_WATCH_REGS, watch_dreg_count +
                                     watch_ireg_count), Usable by ptrace */
  unsigned int kscratch_mask;     /* Usable KScratch mask. */

  /* VZ & Guest features */
  struct guest_info guest;
  unsigned int guestid_mask;
  unsigned int guestid_cache;
  unsigned long guest_cfg;
} __attribute__((aligned(SMP_CACHE_BYTES)));

extern struct cpuinfo_loongarch cpu_data[];
#define boot_cpu_data cpu_data[0]
#define current_cpu_data cpu_data[smp_processor_id()]
#define raw_current_cpu_data cpu_data[raw_smp_processor_id()]

extern void cpu_probe(void);

extern const char *__cpu_family[];
extern const char *__cpu_full_name[];
#define cpu_family_string() __cpu_family[raw_smp_processor_id()]
#define cpu_full_name_string() __cpu_full_name[raw_smp_processor_id()]

static inline unsigned long cpu_asid_mask(struct cpuinfo_loongarch *cpuinfo) {
  return cpuinfo->asid_mask;
}

static inline void set_cpu_asid_mask(struct cpuinfo_loongarch *cpuinfo,
                                     unsigned long asid_mask) {
  cpuinfo->asid_mask = asid_mask;
}

#endif /* end of include guard: CPU_INFO_H_VPSRUY4J */
