#include "cpu.h"
#include "../../include/exec/cpu-ldst.h"
#include "../../include/exec/exec-all.h"
#include "../../include/fpu/softfloat-helper.h"
#include "../../include/fpu/softfloat.h"
#include "../../include/hw/i386/apic.h"
#include "../../include/qemu/bswap.h"
#include "../../include/qemu/log-for-trace.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/*
static int64_t x86_cpu_get_arch_id(CPUState *cs)
{
    X86CPU *cpu = X86_CPU(cs);

    return cpu->apic_id;
}
*/

static inline uint64_t x86_cpu_xsave_components(X86CPU *cpu) {
  return ((uint64_t)cpu->env.features[FEAT_XSAVE_COMP_HI]) << 32 |
         cpu->env.features[FEAT_XSAVE_COMP_LO];
}

typedef struct ExtSaveArea {
  uint32_t feature, bits;
  uint32_t offset, size;
} ExtSaveArea;

static const ExtSaveArea x86_ext_save_areas[] = {
    [XSTATE_FP_BIT] =
        {
            /* x87 FP state component is always enabled if XSAVE is supported */
            .feature = FEAT_1_ECX,
            .bits = CPUID_EXT_XSAVE,
            /* x87 state is in the legacy region of the XSAVE area */
            .offset = 0,
            .size = sizeof(X86LegacyXSaveArea) + sizeof(X86XSaveHeader),
        },
    [XSTATE_SSE_BIT] =
        {
            /* SSE state component is always enabled if XSAVE is supported */
            .feature = FEAT_1_ECX,
            .bits = CPUID_EXT_XSAVE,
            /* SSE state is in the legacy region of the XSAVE area */
            .offset = 0,
            .size = sizeof(X86LegacyXSaveArea) + sizeof(X86XSaveHeader),
        },
    [XSTATE_YMM_BIT] = {.feature = FEAT_1_ECX,
                        .bits = CPUID_EXT_AVX,
                        .offset = offsetof(X86XSaveArea, avx_state),
                        .size = sizeof(XSaveAVX)},
    [XSTATE_BNDREGS_BIT] = {.feature = FEAT_7_0_EBX,
                            .bits = CPUID_7_0_EBX_MPX,
                            .offset = offsetof(X86XSaveArea, bndreg_state),
                            .size = sizeof(XSaveBNDREG)},
    [XSTATE_BNDCSR_BIT] = {.feature = FEAT_7_0_EBX,
                           .bits = CPUID_7_0_EBX_MPX,
                           .offset = offsetof(X86XSaveArea, bndcsr_state),
                           .size = sizeof(XSaveBNDCSR)},
    [XSTATE_OPMASK_BIT] = {.feature = FEAT_7_0_EBX,
                           .bits = CPUID_7_0_EBX_AVX512F,
                           .offset = offsetof(X86XSaveArea, opmask_state),
                           .size = sizeof(XSaveOpmask)},
    [XSTATE_ZMM_Hi256_BIT] = {.feature = FEAT_7_0_EBX,
                              .bits = CPUID_7_0_EBX_AVX512F,
                              .offset = offsetof(X86XSaveArea, zmm_hi256_state),
                              .size = sizeof(XSaveZMM_Hi256)},
    [XSTATE_Hi16_ZMM_BIT] = {.feature = FEAT_7_0_EBX,
                             .bits = CPUID_7_0_EBX_AVX512F,
                             .offset = offsetof(X86XSaveArea, hi16_zmm_state),
                             .size = sizeof(XSaveHi16_ZMM)},
    [XSTATE_PKRU_BIT] = {.feature = FEAT_7_0_ECX,
                         .bits = CPUID_7_0_ECX_PKU,
                         .offset = offsetof(X86XSaveArea, pkru_state),
                         .size = sizeof(XSavePKRU)},
};

static uint32_t xsave_area_size(uint64_t mask) {
  int i;
  uint64_t ret = 0;

  for (i = 0; i < ARRAY_SIZE(x86_ext_save_areas); i++) {
    const ExtSaveArea *esa = &x86_ext_save_areas[i];
    if ((mask >> i) & 1) {
      ret = MAX(ret, esa->offset + esa->size);
    }
  }
  return ret;
}

#define L1_DTLB_2M_ASSOC 1
#define L1_DTLB_2M_ENTRIES 255
#define L1_DTLB_4K_ASSOC 1
#define L1_DTLB_4K_ENTRIES 255

#define L1_ITLB_2M_ASSOC 1
#define L1_ITLB_2M_ENTRIES 255
#define L1_ITLB_4K_ASSOC 1
#define L1_ITLB_4K_ENTRIES 255

#define L2_DTLB_2M_ASSOC 0   /* disabled */
#define L2_DTLB_2M_ENTRIES 0 /* disabled */
#define L2_DTLB_4K_ASSOC 4
#define L2_DTLB_4K_ENTRIES 512

#define L2_ITLB_2M_ASSOC 0   /* disabled */
#define L2_ITLB_2M_ENTRIES 0 /* disabled */
#define L2_ITLB_4K_ASSOC 4
#define L2_ITLB_4K_ENTRIES 512

#define ASSOC_FULL 0xFF
/* AMD associativity encoding used on CPUID Leaf 0x80000006: */
#define AMD_ENC_ASSOC(a)                                                           \
  (a <= 1                                                                          \
       ? a                                                                         \
       : a == 2                                                                    \
             ? 0x2                                                                 \
             : a == 4                                                              \
                   ? 0x4                                                           \
                   : a == 8                                                        \
                         ? 0x6                                                     \
                         : a == 16                                                 \
                               ? 0x8                                               \
                               : a == 32                                           \
                                     ? 0xA                                         \
                                     : a == 48                                     \
                                           ? 0xB                                   \
                                           : a == 64                               \
                                                 ? 0xC                             \
                                                 : a == 96 ? 0xD                   \
                                                           : a == 128              \
                                                                 ? 0xE             \
                                                                 : a == ASSOC_FULL \
                                                                       ? 0xF       \
                                                                       : 0 /* invalid value */)

/* Encode cache info for CPUID[0x80000005].ECX or CPUID[0x80000005].EDX */
static uint32_t encode_cache_cpuid80000005(CPUCacheInfo *cache) {
  assert(cache->size % 1024 == 0);
  assert(cache->lines_per_tag > 0);
  assert(cache->associativity > 0);
  assert(cache->line_size > 0);
  return ((cache->size / 1024) << 24) | (cache->associativity << 16) |
         (cache->lines_per_tag << 8) | (cache->line_size);
}

/*
 * Encode cache info for CPUID[0x80000006].ECX and CPUID[0x80000006].EDX
 * @l3 can be NULL.
 */
static void encode_cache_cpuid80000006(CPUCacheInfo *l2, CPUCacheInfo *l3,
                                       uint32_t *ecx, uint32_t *edx) {
  assert(l2->size % 1024 == 0);
  assert(l2->associativity > 0);
  assert(l2->lines_per_tag > 0);
  assert(l2->line_size > 0);
  *ecx = ((l2->size / 1024) << 16) | (AMD_ENC_ASSOC(l2->associativity) << 12) |
         (l2->lines_per_tag << 8) | (l2->line_size);

  if (l3) {
    assert(l3->size % (512 * 1024) == 0);
    assert(l3->associativity > 0);
    assert(l3->lines_per_tag > 0);
    assert(l3->line_size > 0);
    *edx = ((l3->size / (512 * 1024)) << 18) |
           (AMD_ENC_ASSOC(l3->associativity) << 12) | (l3->lines_per_tag << 8) |
           (l3->line_size);
  } else {
    *edx = 0;
  }
}

static bool x86_cpu_get_paging_enabled(const CPUState *cs) {
  X86CPU *cpu = X86_CPU(cs);

  return cpu->env.cr[0] & CR0_PG_MASK;
}

static void x86_cpu_set_pc(CPUState *cs, vaddr value) {
  X86CPU *cpu = X86_CPU(cs);

  cpu->env.eip = value;
}

void x86_cpu_synchronize_from_tb(CPUState *cs, TranslationBlock *tb) {
  X86CPU *cpu = X86_CPU(cs);

  cpu->env.eip = tb->pc - tb->cs_base;
}

/*
FIXME, only these are necessary, so I only copy them

FIXME cpuid get the information form env, I don't know how to init them

0
1
6
7
0xd
0x80000000
0x80000001
0x80000002
0x80000003
0x80000004
0x80000005
0x80000006
0x80000007
0x80000008
0x8000000a
0x40000000
*/
void cpu_x86_cpuid(CPUX86State *env, uint32_t index, uint32_t count,
                   uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {

  X86CPU *cpu = env_archcpu(env);
  CPUState *cs = env_cpu(env);
  uint32_t die_offset;
  uint32_t limit;
  uint32_t signature[3];

  /* Calculate & apply limits for different index ranges */
  if (index >= 0xC0000000) {
    limit = env->cpuid_xlevel2;
  } else if (index >= 0x80000000) {
    limit = env->cpuid_xlevel;
  } else if (index >= 0x40000000) {
    limit = 0x40000001;
  } else {
    limit = env->cpuid_level;
  }

  if (index > limit) {
    /* Intel documentation states that invalid EAX input will
     * return the same information as EAX=cpuid_level
     * (Intel SDM Vol. 2A - Instruction Set Reference - CPUID)
     */
    index = env->cpuid_level;
  }

  switch (index) {
  case 0:
    *eax = env->cpuid_level;
    *ebx = env->cpuid_vendor1;
    *edx = env->cpuid_vendor2;
    *ecx = env->cpuid_vendor3;
    break;
  case 1:
    *eax = env->cpuid_version;
    *ebx = (cpu->apic_id << 24) |
           8 << 8; /* CLFLUSH size in quad words, Linux wants it. */
    *ecx = env->features[FEAT_1_ECX];
    if ((*ecx & CPUID_EXT_XSAVE) && (env->cr[4] & CR4_OSXSAVE_MASK)) {
      *ecx |= CPUID_EXT_OSXSAVE;
    }
    *edx = env->features[FEAT_1_EDX];
    if (cs->nr_cores * cs->nr_threads > 1) {
      *ebx |= (cs->nr_cores * cs->nr_threads) << 16;
      *edx |= CPUID_HT;
    }
    break;
  case 2:
  case 4:
  case 5:
  case 9:
  case 0xA:
  case 0xB:
  case 0x1F:
  case 0x14:
    bmbt_assert();
  case 6:
    /* Thermal and Power Leaf */
    *eax = env->features[FEAT_6_EAX];
    *ebx = 0;
    *ecx = 0;
    *edx = 0;
    break;
  case 7:
    /* Structured Extended Feature Flags Enumeration Leaf */
    if (count == 0) {
      /* Maximum ECX value for sub-leaves */
      *eax = env->cpuid_level_func7;
      *ebx = env->features[FEAT_7_0_EBX]; /* Feature flags */
      *ecx = env->features[FEAT_7_0_ECX]; /* Feature flags */
      if ((*ecx & CPUID_7_0_ECX_PKU) && env->cr[4] & CR4_PKE_MASK) {
        *ecx |= CPUID_7_0_ECX_OSPKE;
      }
      *edx = env->features[FEAT_7_0_EDX]; /* Feature flags */
    } else if (count == 1) {
      *eax = env->features[FEAT_7_1_EAX];
      *ebx = 0;
      *ecx = 0;
      *edx = 0;
    } else {
      *eax = 0;
      *ebx = 0;
      *ecx = 0;
      *edx = 0;
    }
    break;
  case 0xD: {
    /* Processor Extended State */
    *eax = 0;
    *ebx = 0;
    *ecx = 0;
    *edx = 0;
    if (!(env->features[FEAT_1_ECX] & CPUID_EXT_XSAVE)) {
      break;
    }

    if (count == 0) {
      *ecx = xsave_area_size(x86_cpu_xsave_components(cpu));
      *eax = env->features[FEAT_XSAVE_COMP_LO];
      *edx = env->features[FEAT_XSAVE_COMP_HI];
      /*
       * The initial value of xcr0 and ebx == 0, On host without kvm
       * commit 412a3c41(e.g., CentOS 6), the ebx's value always == 0
       * even through guest update xcr0, this will crash some legacy guest
       * (e.g., CentOS 6), So set ebx == ecx to workaroud it.
       */

      // *ebx = kvm_enabled() ? *ecx : xsave_area_size(env->xcr0);
      *ebx = xsave_area_size(env->xcr0);
    } else if (count == 1) {
      *eax = env->features[FEAT_XSAVE];
    } else if (count < ARRAY_SIZE(x86_ext_save_areas)) {
      if ((x86_cpu_xsave_components(cpu) >> count) & 1) {
        const ExtSaveArea *esa = &x86_ext_save_areas[count];
        *eax = esa->size;
        *ebx = esa->offset;
      }
    }
    break;
  }
  case 0x40000000:
    /*
     * CPUID code in kvm_arch_init_vcpu() ignores stuff
     * set here, but we restrict to TCG none the less.
     */
    if (tcg_enabled() && cpu->expose_tcg) {
      memcpy(signature, "TCGTCGTCGTCG", 12);
      *eax = 0x40000001;
      *ebx = signature[0];
      *ecx = signature[1];
      *edx = signature[2];
    } else {
      *eax = 0;
      *ebx = 0;
      *ecx = 0;
      *edx = 0;
    }
    break;
  case 0x40000001:
    *eax = 0;
    *ebx = 0;
    *ecx = 0;
    *edx = 0;
    break;
  case 0x80000000:
    *eax = env->cpuid_xlevel;
    *ebx = env->cpuid_vendor1;
    *edx = env->cpuid_vendor2;
    *ecx = env->cpuid_vendor3;
    break;
  case 0x80000001:
    *eax = env->cpuid_version;
    *ebx = 0;
    *ecx = env->features[FEAT_8000_0001_ECX];
    *edx = env->features[FEAT_8000_0001_EDX];

    /* The Linux kernel checks for the CMPLegacy bit and
     * discards multiple thread information if it is set.
     * So don't set it here for Intel to make Linux guests happy.
     */
    if (cs->nr_cores * cs->nr_threads > 1) {
      if (env->cpuid_vendor1 != CPUID_VENDOR_INTEL_1 ||
          env->cpuid_vendor2 != CPUID_VENDOR_INTEL_2 ||
          env->cpuid_vendor3 != CPUID_VENDOR_INTEL_3) {
        *ecx |= 1 << 1; /* CmpLegacy bit */
      }
    }
    break;
  case 0x80000002:
  case 0x80000003:
  case 0x80000004:
    *eax = env->cpuid_model[(index - 0x80000002) * 4 + 0];
    *ebx = env->cpuid_model[(index - 0x80000002) * 4 + 1];
    *ecx = env->cpuid_model[(index - 0x80000002) * 4 + 2];
    *edx = env->cpuid_model[(index - 0x80000002) * 4 + 3];
    break;
  case 0x80000005:
    /* cache info (L1 cache) */
    // FIXME I guess it's useless, verify it later
    // if (cpu->cache_info_passthrough) {
    // host_cpuid(index, 0, eax, ebx, ecx, edx);
    // break;
    // }
    *eax = (L1_DTLB_2M_ASSOC << 24) | (L1_DTLB_2M_ENTRIES << 16) |
           (L1_ITLB_2M_ASSOC << 8) | (L1_ITLB_2M_ENTRIES);
    *ebx = (L1_DTLB_4K_ASSOC << 24) | (L1_DTLB_4K_ENTRIES << 16) |
           (L1_ITLB_4K_ASSOC << 8) | (L1_ITLB_4K_ENTRIES);
    *ecx = encode_cache_cpuid80000005(env->cache_info_amd.l1d_cache);
    *edx = encode_cache_cpuid80000005(env->cache_info_amd.l1i_cache);
    break;
  case 0x80000006:
    // FIXME similar problem
    /* cache info (L2 cache) */
    // if (cpu->cache_info_passthrough) {
    // host_cpuid(index, 0, eax, ebx, ecx, edx);
    // break;
    // }
    *eax = (AMD_ENC_ASSOC(L2_DTLB_2M_ASSOC) << 28) |
           (L2_DTLB_2M_ENTRIES << 16) |
           (AMD_ENC_ASSOC(L2_ITLB_2M_ASSOC) << 12) | (L2_ITLB_2M_ENTRIES);
    *ebx = (AMD_ENC_ASSOC(L2_DTLB_4K_ASSOC) << 28) |
           (L2_DTLB_4K_ENTRIES << 16) |
           (AMD_ENC_ASSOC(L2_ITLB_4K_ASSOC) << 12) | (L2_ITLB_4K_ENTRIES);
    encode_cache_cpuid80000006(
        env->cache_info_amd.l2_cache,
        cpu->enable_l3_cache ? env->cache_info_amd.l3_cache : NULL, ecx, edx);
    break;
  case 0x80000007:
    *eax = 0;
    *ebx = 0;
    *ecx = 0;
    *edx = env->features[FEAT_8000_0007_EDX];
    break;
  case 0x80000008:
    /* virtual & phys address size in low 2 bytes. */
    if (env->features[FEAT_8000_0001_EDX] & CPUID_EXT2_LM) {
      /* 64 bit processor */
      *eax = cpu->phys_bits; /* configurable physical bits */
      if (env->features[FEAT_7_0_ECX] & CPUID_7_0_ECX_LA57) {
        *eax |= 0x00003900; /* 57 bits virtual */
      } else {
        *eax |= 0x00003000; /* 48 bits virtual */
      }
    } else {
      *eax = cpu->phys_bits;
    }
    *ebx = env->features[FEAT_8000_0008_EBX];
    *ecx = 0;
    *edx = 0;
    if (cs->nr_cores * cs->nr_threads > 1) {
      *ecx |= (cs->nr_cores * cs->nr_threads) - 1;
    }
    break;
  case 0x8000000A:
    if (env->features[FEAT_8000_0001_ECX] & CPUID_EXT3_SVM) {
      *eax = 0x00000001; /* SVM Revision */
      *ebx = 0x00000010; /* nr of ASIDs */
      *ecx = 0;
      *edx = env->features[FEAT_SVM]; /* optional features */
    } else {
      *eax = 0;
      *ebx = 0;
      *ecx = 0;
      *edx = 0;
    }
    break;
  case 0x8000001D:
  case 0x8000001E:
  case 0xC0000000:
  case 0xC0000001:
  case 0xC0000002:
  case 0xC0000003:
  case 0xC0000004:
  case 0x8000001F:
    bmbt_assert();
  default:
    /* reserved values: zero */
    *eax = 0;
    *ebx = 0;
    *ecx = 0;
    *edx = 0;
    break;
  }
}

/* CPUClass::reset() */
static void x86_cpu_reset(CPUState *s) {
  X86CPU *cpu = X86_CPU(s);
  X86CPUClass *xcc = X86_CPU_GET_CLASS(cpu);
  CPUX86State *env = &cpu->env;
  target_ulong cr4;
  uint64_t xcr0;
  int i;

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
  xtm_pf_inc_tbf_cpu_reset();
#endif

  xcc->parent_reset(s);

  memset(env, 0, offsetof(CPUX86State, end_reset_fields));

  env->old_exception = -1;

  /* init to reset state */

  env->hflags2 |= HF2_GIF_MASK;

  cpu_x86_update_cr0(env, 0x60000010);
  env->a20_mask = ~0x0;
  env->smbase = 0x30000;
  env->msr_smi_count = 0;

  env->idt.limit = 0xffff;
  env->gdt.limit = 0xffff;
  env->ldt.limit = 0xffff;
  env->ldt.flags = DESC_P_MASK | (2 << DESC_TYPE_SHIFT);
  env->tr.limit = 0xffff;
  env->tr.flags = DESC_P_MASK | (11 << DESC_TYPE_SHIFT);

  cpu_x86_load_seg_cache(env, R_CS, 0xf000, 0xffff0000, 0xffff,
                         DESC_P_MASK | DESC_S_MASK | DESC_CS_MASK |
                             DESC_R_MASK | DESC_A_MASK);
  cpu_x86_load_seg_cache(env, R_DS, 0, 0, 0xffff,
                         DESC_P_MASK | DESC_S_MASK | DESC_W_MASK | DESC_A_MASK);
  cpu_x86_load_seg_cache(env, R_ES, 0, 0, 0xffff,
                         DESC_P_MASK | DESC_S_MASK | DESC_W_MASK | DESC_A_MASK);
  cpu_x86_load_seg_cache(env, R_SS, 0, 0, 0xffff,
                         DESC_P_MASK | DESC_S_MASK | DESC_W_MASK | DESC_A_MASK);
  cpu_x86_load_seg_cache(env, R_FS, 0, 0, 0xffff,
                         DESC_P_MASK | DESC_S_MASK | DESC_W_MASK | DESC_A_MASK);
  cpu_x86_load_seg_cache(env, R_GS, 0, 0, 0xffff,
                         DESC_P_MASK | DESC_S_MASK | DESC_W_MASK | DESC_A_MASK);

  env->eip = 0xfff0;
  env->regs[R_EDX] = env->cpuid_version;

  env->eflags = 0x2;

  /* FPU init */
  for (i = 0; i < 8; i++) {
    env->fptags[i] = 1;
  }
  cpu_set_fpuc(env, 0x37f);

  env->mxcsr = 0x1f80;
  /* All units are in INIT state.  */
  env->xstate_bv = 0;

  env->pat = 0x0007040600070406ULL;
  env->msr_ia32_misc_enable = MSR_IA32_MISC_ENABLE_DEFAULT;
  if (env->features[FEAT_1_ECX] & CPUID_EXT_MONITOR) {
    env->msr_ia32_misc_enable |= MSR_IA32_MISC_ENABLE_MWAIT;
  }

  memset(env->dr, 0, sizeof(env->dr));
  env->dr[6] = DR6_FIXED_1;
  env->dr[7] = DR7_FIXED_1;
  cpu_breakpoint_remove_all(s, BP_CPU);
  cpu_watchpoint_remove_all(s, BP_CPU);

  cr4 = 0;
  xcr0 = XSTATE_FP_MASK;

#ifdef CONFIG_USER_ONLY
  /* Enable all the features for user-mode.  */
  if (env->features[FEAT_1_EDX] & CPUID_SSE) {
    xcr0 |= XSTATE_SSE_MASK;
  }
  for (i = 2; i < ARRAY_SIZE(x86_ext_save_areas); i++) {
    const ExtSaveArea *esa = &x86_ext_save_areas[i];
    if (env->features[esa->feature] & esa->bits) {
      xcr0 |= 1ull << i;
    }
  }

  if (env->features[FEAT_1_ECX] & CPUID_EXT_XSAVE) {
    cr4 |= CR4_OSFXSR_MASK | CR4_OSXSAVE_MASK;
  }
  if (env->features[FEAT_7_0_EBX] & CPUID_7_0_EBX_FSGSBASE) {
    cr4 |= CR4_FSGSBASE_MASK;
  }
#endif

  env->xcr0 = xcr0;
  cpu_x86_update_cr4(env, cr4);

  /*
   * SDM 11.11.5 requires:
   *  - IA32_MTRR_DEF_TYPE MSR.E = 0
   *  - IA32_MTRR_PHYSMASKn.V = 0
   * All other bits are undefined.  For simplification, zero it all.
   */
  env->mtrr_deftype = 0;
  memset(env->mtrr_var, 0, sizeof(env->mtrr_var));
  memset(env->mtrr_fixed, 0, sizeof(env->mtrr_fixed));

  // FIXME comments in QEMU say this is related with kvm, verify it later
  // env->interrupt_injected = -1;
  // env->exception_nr = -1;
  // env->exception_pending = 0;
  // env->exception_injected = 0;
  // env->exception_has_payload = false;
  // env->exception_payload = 0;
  // env->nmi_injected = false;
#if !defined(CONFIG_USER_ONLY)
  /* We hard-wire the BSP to the first CPU. */
  apic_designate_bsp(cpu->apic_state, s->cpu_index == 0);

  s->halted = !cpu_is_bsp(cpu);
#endif
}

static void mce_init(X86CPU *cpu) {
  CPUX86State *cenv = &cpu->env;
  unsigned int bank;

  if (((cenv->cpuid_version >> 8) & 0xf) >= 6 &&
      (cenv->features[FEAT_1_EDX] & (CPUID_MCE | CPUID_MCA)) ==
          (CPUID_MCE | CPUID_MCA)) {
    cenv->mcg_cap =
        MCE_CAP_DEF | MCE_BANKS_DEF | (cpu->enable_lmce ? MCG_LMCE_P : 0);
    cenv->mcg_ctl = ~(uint64_t)0;
    for (bank = 0; bank < MCE_BANKS_DEF; bank++) {
      cenv->mce_banks[bank * 4] = ~(uint64_t)0;
    }
  }
}

int x86_cpu_pending_interrupt(CPUState *cs, int interrupt_request) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;

#if !defined(CONFIG_USER_ONLY)
  if (interrupt_request & CPU_INTERRUPT_POLL) {
    return CPU_INTERRUPT_POLL;
  }
#endif
  if (interrupt_request & CPU_INTERRUPT_SIPI) {
    return CPU_INTERRUPT_SIPI;
  }

  if (env->hflags2 & HF2_GIF_MASK) {
    if ((interrupt_request & CPU_INTERRUPT_SMI) &&
        !(env->hflags & HF_SMM_MASK)) {
      return CPU_INTERRUPT_SMI;
    } else if ((interrupt_request & CPU_INTERRUPT_NMI) &&
               !(env->hflags2 & HF2_NMI_MASK)) {
      return CPU_INTERRUPT_NMI;
    } else if (interrupt_request & CPU_INTERRUPT_MCE) {
      return CPU_INTERRUPT_MCE;
    } else if ((interrupt_request & CPU_INTERRUPT_HARD) &&
               (((env->hflags2 & HF2_VINTR_MASK) &&
                 (env->hflags2 & HF2_HIF_MASK)) ||
                (!(env->hflags2 & HF2_VINTR_MASK) &&
                 (env->eflags & IF_MASK &&
                  !(env->hflags & HF_INHIBIT_IRQ_MASK))))) {
      return CPU_INTERRUPT_HARD;
#if !defined(CONFIG_USER_ONLY)
    } else if ((interrupt_request & CPU_INTERRUPT_VIRQ) &&
               (env->eflags & IF_MASK) &&
               !(env->hflags & HF_INHIBIT_IRQ_MASK)) {
      return CPU_INTERRUPT_VIRQ;
#endif
    }
  }

  return 0;
}

static void x86_cpu_common_class_init(CPUState *cpu) {
  CPUClass *cc = CPU_GET_CLASS(cpu);

  // device_class_set_parent_realize(dc, x86_cpu_realizefn,
  // &xcc->parent_realize);
  // device_class_set_parent_unrealize(dc, x86_cpu_unrealizefn,
  // &xcc->parent_unrealize);
  // dc->props = x86_cpu_properties;

  // xcc->parent_reset = cc->reset;
  cc->reset = x86_cpu_reset;
  cc->reset_dump_flags = CPU_DUMP_FPU | CPU_DUMP_CCOP;

#ifdef CONFIG_TCG
  cc->cpu_exec_interrupt = x86_cpu_exec_interrupt;
#endif
  cc->set_pc = x86_cpu_set_pc;
  cc->synchronize_from_tb = x86_cpu_synchronize_from_tb;
  cc->get_paging_enabled = x86_cpu_get_paging_enabled;
#ifndef CONFIG_USER_ONLY

#endif

#ifdef TARGET_X86_64
  cc->gdb_core_xml_file = "i386-64bit.xml";
  cc->gdb_num_core_regs = 66;
#else
#endif
#if defined(CONFIG_TCG) && !defined(CONFIG_USER_ONLY)
  cc->debug_excp_handler = breakpoint_handler;
#endif
  cc->cpu_exec_enter = x86_cpu_exec_enter;
  cc->cpu_exec_exit = x86_cpu_exec_exit;
#ifdef CONFIG_TCG
  cc->tlb_fill = x86_cpu_tlb_fill;
#endif
}
