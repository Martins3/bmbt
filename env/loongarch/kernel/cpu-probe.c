#include <asm/cpu-features.h>
#include <asm/cpu-info.h>
#include <asm/cpu.h>
#include <asm/setup.h>
#include <assert.h>
#include <autoconf.h>
#include <linux/bits.h>
#include <linux/smp.h>
#include <linux/threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <uapi/asm/hwcap.h>

unsigned int elf_hwcap;

static void set_isa(struct cpuinfo_loongarch *c, unsigned int isa) {
  switch (isa) {
  case LOONGARCH_CPU_ISA_LA64:
    c->isa_level |= LOONGARCH_CPU_ISA_LA64;
  case LOONGARCH_CPU_ISA_LA32S:
    c->isa_level |= LOONGARCH_CPU_ISA_LA32S;
  case LOONGARCH_CPU_ISA_LA32R:
    c->isa_level |= LOONGARCH_CPU_ISA_LA32R;
  }
}

static void cpu_probe_common(struct cpuinfo_loongarch *c) {
  unsigned int config;

  c->options = LOONGARCH_CPU_CPUCFG | LOONGARCH_CPU_CSR | LOONGARCH_CPU_TLB |
               LOONGARCH_CPU_VINT | LOONGARCH_CPU_WATCH;

  elf_hwcap |= HWCAP_LOONGARCH_CRC32;

  config = read_cpucfg(LOONGARCH_CPUCFG1);
  if (config & CPUCFG1_UAL) {
    c->options |= LOONGARCH_CPU_UAL;
    elf_hwcap |= HWCAP_LOONGARCH_UAL;
  }

  config = read_cpucfg(LOONGARCH_CPUCFG2);
  if (config & CPUCFG2_LAM) {
    c->options |= LOONGARCH_CPU_LAM;
    elf_hwcap |= HWCAP_LOONGARCH_LAM;
  }
  if (config & CPUCFG2_FP) {
    c->options |= LOONGARCH_CPU_FPU;
    elf_hwcap |= HWCAP_LOONGARCH_FPU;
  }
#ifdef CONFIG_CPU_HAS_LSX
  if (config & CPUCFG2_LSX) {
    c->options |= LOONGARCH_CPU_LSX;
    elf_hwcap |= HWCAP_LOONGARCH_LSX;
  }
#endif
#ifdef CONFIG_CPU_HAS_LASX
  if (config & CPUCFG2_LASX) {
    c->options |= LOONGARCH_CPU_LASX;
    elf_hwcap |= HWCAP_LOONGARCH_LASX;
  }
#endif
  if (config & CPUCFG2_COMPLEX) {
    c->options |= LOONGARCH_CPU_COMPLEX;
    elf_hwcap |= HWCAP_LOONGARCH_COMPLEX;
  }
  if (config & CPUCFG2_CRYPTO) {
    c->options |= LOONGARCH_CPU_CRYPTO;
    elf_hwcap |= HWCAP_LOONGARCH_CRYPTO;
  }
  if (config & CPUCFG2_LVZP) {
    c->options |= LOONGARCH_CPU_LVZ;
    elf_hwcap |= HWCAP_LOONGARCH_LVZ;
  }
#ifdef CONFIG_CPU_HAS_LBT
  if (config & CPUCFG2_X86BT) {
    c->options |= LOONGARCH_CPU_LBT_X86;
    elf_hwcap |= HWCAP_LOONGARCH_LBT_X86;
  }
  if (config & CPUCFG2_ARMBT) {
    c->options |= LOONGARCH_CPU_LBT_ARM;
    elf_hwcap |= HWCAP_LOONGARCH_LBT_ARM;
  }
  if (config & CPUCFG2_MIPSBT) {
    c->options |= LOONGARCH_CPU_LBT_MIPS;
    elf_hwcap |= HWCAP_LOONGARCH_LBT_MIPS;
  }
#endif

  config = read_cpucfg(LOONGARCH_CPUCFG6);
  if (config & CPUCFG6_PMP)
    c->options |= LOONGARCH_CPU_PMP;

  config = iocsr_readl(LOONGARCH_IOCSR_FEATURES);
  if (config & IOCSRF_CSRIPI)
    c->options |= LOONGARCH_CPU_CSRIPI;
  if (config & IOCSRF_EXTIOI)
    c->options |= LOONGARCH_CPU_EXTIOI;
  if (config & IOCSRF_FREQSCALE)
    c->options |= LOONGARCH_CPU_SCALEFREQ;
  if (config & IOCSRF_VM)
    c->options |= LOONGARCH_CPU_HYPERVISOR;
  if (config & IOCSRF_EXTIOI_DECODE)
    c->options |= LOONGARCH_CPU_EXTIOI_DECODE;
  if (config & IOCSRF_FLATMODE)
    c->options |= LOONGARCH_CPU_FLATMODE;

  config = read_csr_prcfg1();
  c->kscratch_mask = GENMASK((config & CSR_CONF1_KSNUM) - 1, 0);
  c->kscratch_mask &= ~(TLB_KSCRATCH_MASK | KVM_KSCRATCH_MASK);
  c->kscratch_mask &= ~PERCPU_BASE_MASK;

  config = read_csr_prcfg3();
  switch (config & CSR_CONF3_TLBTYPE) {
  case 0:
    c->tlbsizemtlb = 0;
    c->tlbsizestlbsets = 0;
    c->tlbsizestlbways = 0;
    c->tlbsize = 0;
    break;
  case 1:
    c->tlbsizemtlb =
        ((config & CSR_CONF3_MTLBSIZE) >> CSR_CONF3_MTLBSIZE_SHIFT) + 1;
    c->tlbsizestlbsets = 0;
    c->tlbsizestlbways = 0;
    c->tlbsize = c->tlbsizemtlb + c->tlbsizestlbsets * c->tlbsizestlbways;
    break;
  case 2:
    c->tlbsizemtlb =
        ((config & CSR_CONF3_MTLBSIZE) >> CSR_CONF3_MTLBSIZE_SHIFT) + 1;
    c->tlbsizestlbsets =
        1 << ((config & CSR_CONF3_STLBIDX) >> CSR_CONF3_STLBIDX_SHIFT);
    c->tlbsizestlbways =
        ((config & CSR_CONF3_STLBWAYS) >> CSR_CONF3_STLBWAYS_SHIFT) + 1;
    c->tlbsize = c->tlbsizemtlb + c->tlbsizestlbsets * c->tlbsizestlbways;
    break;
  default:
    printf("warning: unimplemented tlb type\n");
    abort();
  }

#ifdef BMBT
  loongarch_probe_watch_registers(c);
#endif
}

#define MAX_NAME_LEN 32
#define VENDOR_OFFSET 0
#define CPUNAME_OFFSET 9

static char cpu_full_name[MAX_NAME_LEN] = "        -        ";

static inline void cpu_probe_loongson(struct cpuinfo_loongarch *c,
                                      unsigned int cpu) {
  uint64_t *vendor = (void *)(&cpu_full_name[VENDOR_OFFSET]);
  uint64_t *cpuname = (void *)(&cpu_full_name[CPUNAME_OFFSET]);

  __cpu_full_name[cpu] = cpu_full_name;
  *vendor = iocsr_readq(LOONGARCH_IOCSR_VENDOR);
  *cpuname = iocsr_readq(LOONGARCH_IOCSR_CPUNAME);

  switch (c->processor_id & PRID_IMP_MASK) {
  case PRID_IMP_LOONGSON_32:
    c->cputype = CPU_LOONGSON32;
    set_isa(c, LOONGARCH_CPU_ISA_LA32S);
    __cpu_family[cpu] = "Loongson-32bit";
    printf("Standard 32-bit Loongson Processor probed\n");
    break;
  case PRID_IMP_LOONGSON_64R:
    c->cputype = CPU_LOONGSON64;
    set_isa(c, LOONGARCH_CPU_ISA_LA64);
    __cpu_family[cpu] = "Loongson-64bit";
    printf("Reduced 64-bit Loongson Processor probed\n");
    break;
  case PRID_IMP_LOONGSON_64C:
    c->cputype = CPU_LOONGSON64;
    set_isa(c, LOONGARCH_CPU_ISA_LA64);
    __cpu_family[cpu] = "Loongson-64bit";
    printf("Classic 64-bit Loongson Processor probed\n");
    break;
  case PRID_IMP_LOONGSON_64G:
    c->cputype = CPU_LOONGSON64;
    set_isa(c, LOONGARCH_CPU_ISA_LA64);
    __cpu_family[cpu] = "Loongson-64bit";
    printf("Generic 64-bit Loongson Processor probed\n");
    break;
  default: /* Default to 64 bit */
    c->cputype = CPU_LOONGSON64;
    set_isa(c, LOONGARCH_CPU_ISA_LA64);
    __cpu_family[cpu] = "Loongson-64bit";
    printf("Unknown 64-bit Loongson Processor probed\n");
  }
}

const char *__cpu_family[NR_CPUS];
const char *__cpu_full_name[NR_CPUS];
const char *__elf_platform;

static void cpu_report(void) {
  struct cpuinfo_loongarch *c = &current_cpu_data;

  printf("CPU%d revision is: %08x (%s)\n", smp_processor_id(), c->processor_id,
         cpu_family_string());
  if (c->options & LOONGARCH_CPU_FPU)
    printf("FPU%d revision is: %08x\n", smp_processor_id(), c->fpu_vers);
}

void cpu_probe(void) {
  unsigned int config;
  unsigned long asid_mask;
  unsigned int cpu = smp_processor_id();
  struct cpuinfo_loongarch *c = &current_cpu_data;

#ifdef BMBT
  /*
   * Set a default elf platform, cpu probe may later
   * overwrite it with a more precise value
   */
  set_elf_platform(cpu, "loongarch");
#endif

  c->cputype = CPU_UNKNOWN;
  c->processor_id = read_cpucfg(LOONGARCH_CPUCFG0);
  c->fpu_vers = (read_cpucfg(LOONGARCH_CPUCFG2) >> 3) & 0x3;

  c->fpu_csr0 = FPU_CSR_RN;
  c->fpu_mask = FPU_CSR_RSVD;

  cpu_probe_common(c);

  config = csr_readq(LOONGARCH_CSR_ASID);
  config = (config & CSR_ASID_BIT) >> CSR_ASID_BIT_SHIFT;
  asid_mask = GENMASK(config - 1, 0);

  set_cpu_asid_mask(c, asid_mask);

  per_cpu_trap_init(cpu);

  switch (c->processor_id & PRID_COMP_MASK) {
  case PRID_COMP_LOONGSON:
    cpu_probe_loongson(c, cpu);
    break;
  }

  assert(__cpu_family[cpu]);
  assert(c->cputype != CPU_UNKNOWN);

#ifdef BMBT
  cpu_probe_addrbits(c);

  if (cpu == 0)
    __ua_limit = ~((1ull << cpu_vabits) - 1);
  if (cpu_has_lvz)
    cpu_probe_lvz(c);
#endif

  cpu_report();
  assert(cpu_has_vint);
}
