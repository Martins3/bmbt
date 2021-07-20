#include "../../include/exec/cpu-ldst.h"
#include "../../include/exec/exec-all.h"
#include "../../include/exec/memory.h"
#include "../../include/fpu/softfloat-helpers.h"
#include "../../include/fpu/softfloat.h"
#include "../../include/hw/i386/apic.h"
#include "../../include/qemu/bswap.h"
#include "../../include/qemu/log.h"
#include "../../include/qemu/log-for-trace.h"
#include "LATX/x86tomips-config.h"
#include "cpu.h"
#include "seg_helper.h"
#include "svm.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// notes: dump and mce related code not copied
// it seems they are useless, but further investigation is needed

void cpu_sync_bndcs_hflags(CPUX86State *env) {
  uint32_t hflags = env->hflags;
  uint32_t hflags2 = env->hflags2;
  uint32_t bndcsr;

  if ((hflags & HF_CPL_MASK) == 3) {
    bndcsr = env->bndcs_regs.cfgu;
  } else {
    bndcsr = env->msr_bndcfgs;
  }

  if ((env->cr[4] & CR4_OSXSAVE_MASK) && (env->xcr0 & XSTATE_BNDCSR_MASK) &&
      (bndcsr & BNDCFG_ENABLE)) {
    hflags |= HF_MPX_EN_MASK;
  } else {
    hflags &= ~HF_MPX_EN_MASK;
  }

  if (bndcsr & BNDCFG_BNDPRESERVE) {
    hflags2 |= HF2_MPX_PR_MASK;
  } else {
    hflags2 &= ~HF2_MPX_PR_MASK;
  }

  env->hflags = hflags;
  env->hflags2 = hflags2;
}

void x86_cpu_set_a20(X86CPU *cpu, int a20_state) {
  CPUX86State *env = &cpu->env;

  a20_state = (a20_state != 0);
  if (a20_state != ((env->a20_mask >> 20) & 1)) {
    CPUState *cs = CPU(cpu);

    qemu_log_mask(CPU_LOG_MMU, "A20 update: a20=%d\n", a20_state);
    /* if the cpu is currently executing code, we must unlink it and
       all the potentially executing TB */
    cpu_interrupt(cs, CPU_INTERRUPT_EXITTB);

    /* when a20 is changed, all the MMU mappings are invalid, so
       we must flush everything */
    tlb_flush(cs);
    env->a20_mask = ~(1 << 20) | (a20_state << 20);
  }
}

void cpu_x86_update_cr0(CPUX86State *env, uint32_t new_cr0) {
  X86CPU *cpu = env_archcpu(env);
  int pe_state;

  qemu_log_mask(CPU_LOG_MMU, "CR0 update: CR0=0x%08x\n", new_cr0);
  if ((new_cr0 & (CR0_PG_MASK | CR0_WP_MASK | CR0_PE_MASK)) !=
      (env->cr[0] & (CR0_PG_MASK | CR0_WP_MASK | CR0_PE_MASK))) {
    tlb_flush(CPU(cpu));
  }

#ifdef TARGET_X86_64
  if (!(env->cr[0] & CR0_PG_MASK) && (new_cr0 & CR0_PG_MASK) &&
      (env->efer & MSR_EFER_LME)) {
    /* enter in long mode */
    /* XXX: generate an exception */
    if (!(env->cr[4] & CR4_PAE_MASK))
      return;
    env->efer |= MSR_EFER_LMA;
    env->hflags |= HF_LMA_MASK;
  } else if ((env->cr[0] & CR0_PG_MASK) && !(new_cr0 & CR0_PG_MASK) &&
             (env->efer & MSR_EFER_LMA)) {
    /* exit long mode */
    env->efer &= ~MSR_EFER_LMA;
    env->hflags &= ~(HF_LMA_MASK | HF_CS64_MASK);
    env->eip &= 0xffffffff;
  }
#endif
  env->cr[0] = new_cr0 | CR0_ET_MASK;

  /* update PE flag in hidden flags */
  pe_state = (env->cr[0] & CR0_PE_MASK);
  env->hflags = (env->hflags & ~HF_PE_MASK) | (pe_state << HF_PE_SHIFT);
  /* ensure that ADDSEG is always set in real mode */
  env->hflags |= ((pe_state ^ 1) << HF_ADDSEG_SHIFT);
  /* update FPU flags */
  env->hflags =
      (env->hflags & ~(HF_MP_MASK | HF_EM_MASK | HF_TS_MASK)) |
      ((new_cr0 << (HF_MP_SHIFT - 1)) & (HF_MP_MASK | HF_EM_MASK | HF_TS_MASK));
}

/* XXX: in legacy PAE mode, generate a GPF if reserved bits are set in
   the PDPT */
void cpu_x86_update_cr3(CPUX86State *env, target_ulong new_cr3) {
#ifndef CONFIG_BTMMU
  env->cr[3] = new_cr3;
  if (env->cr[0] & CR0_PG_MASK) {
    qemu_log_mask(CPU_LOG_MMU, "CR3 update: CR3=" TARGET_FMT_lx "\n", new_cr3);
    tlb_flush(env_cpu(env));
  }
#else
  if (env->cr[0] & CR0_PG_MASK) {
    qemu_log_mask(CPU_LOG_MMU, "CR3 update: CR3=" TARGET_FMT_lx "\n", new_cr3);
#ifdef CONFIG_SOFTMMU
    if (btmmu_enabled() && env->cr[3] == new_cr3)
      btmmu_set_need_flush();
#endif
    tlb_flush(env_cpu(env));
  }
  env->cr[3] = new_cr3;
#ifdef CONFIG_SOFTMMU
  btmmu_set_context(new_cr3);
#endif
#endif
}

void cpu_x86_update_cr4(CPUX86State *env, uint32_t new_cr4) {
  uint32_t hflags;

#if defined(DEBUG_MMU)
  printf("CR4 update: %08x -> %08x\n", (uint32_t)env->cr[4], new_cr4);
#endif
  if ((new_cr4 ^ env->cr[4]) &
      (CR4_PGE_MASK | CR4_PAE_MASK | CR4_PSE_MASK | CR4_SMEP_MASK |
       CR4_SMAP_MASK | CR4_LA57_MASK)) {
    tlb_flush(env_cpu(env));
  }

  /* Clear bits we're going to recompute.  */
  hflags = env->hflags & ~(HF_OSFXSR_MASK | HF_SMAP_MASK);

  /* SSE handling */
  if (!(env->features[FEAT_1_EDX] & CPUID_SSE)) {
    new_cr4 &= ~CR4_OSFXSR_MASK;
  }
  if (new_cr4 & CR4_OSFXSR_MASK) {
    hflags |= HF_OSFXSR_MASK;
  }

  if (!(env->features[FEAT_7_0_EBX] & CPUID_7_0_EBX_SMAP)) {
    new_cr4 &= ~CR4_SMAP_MASK;
  }
  if (new_cr4 & CR4_SMAP_MASK) {
    hflags |= HF_SMAP_MASK;
  }

  if (!(env->features[FEAT_7_0_ECX] & CPUID_7_0_ECX_PKU)) {
    new_cr4 &= ~CR4_PKE_MASK;
  }

  env->cr[4] = new_cr4;
  env->hflags = hflags;

  cpu_sync_bndcs_hflags(env);
}

#if !defined(CONFIG_USER_ONLY)
hwaddr x86_cpu_get_phys_page_attrs_debug(CPUState *cs, vaddr addr,
                                         MemTxAttrs *attrs) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;
  target_ulong pde_addr, pte_addr;
  uint64_t pte;
  int32_t a20_mask;
  uint32_t page_offset;
  int page_size;

  *attrs = cpu_get_mem_attrs(env);

  a20_mask = x86_get_a20_mask(env);
  if (!(env->cr[0] & CR0_PG_MASK)) {
    pte = addr & a20_mask;
    page_size = 4096;
  } else if (env->cr[4] & CR4_PAE_MASK) {
    target_ulong pdpe_addr;
    uint64_t pde, pdpe;

#ifdef TARGET_X86_64
    if (env->hflags & HF_LMA_MASK) {
      bool la57 = env->cr[4] & CR4_LA57_MASK;
      uint64_t pml5e_addr, pml5e;
      uint64_t pml4e_addr, pml4e;
      int32_t sext;

      /* test virtual address sign extension */
      sext = la57 ? (int64_t)addr >> 56 : (int64_t)addr >> 47;
      if (sext != 0 && sext != -1) {
        return -1;
      }

      if (la57) {
        pml5e_addr =
            ((env->cr[3] & ~0xfff) + (((addr >> 48) & 0x1ff) << 3)) & a20_mask;
        pml5e = x86_ldq_phys(cs, pml5e_addr);
        if (!(pml5e & PG_PRESENT_MASK)) {
          return -1;
        }
      } else {
        pml5e = env->cr[3];
      }

      pml4e_addr = ((pml5e & PG_ADDRESS_MASK) + (((addr >> 39) & 0x1ff) << 3)) &
                   a20_mask;
      pml4e = x86_ldq_phys(cs, pml4e_addr);
      if (!(pml4e & PG_PRESENT_MASK)) {
        return -1;
      }
      pdpe_addr = ((pml4e & PG_ADDRESS_MASK) + (((addr >> 30) & 0x1ff) << 3)) &
                  a20_mask;
      pdpe = x86_ldq_phys(cs, pdpe_addr);
      if (!(pdpe & PG_PRESENT_MASK)) {
        return -1;
      }
      if (pdpe & PG_PSE_MASK) {
        page_size = 1024 * 1024 * 1024;
        pte = pdpe;
        goto out;
      }

    } else
#endif
    {
      pdpe_addr = ((env->cr[3] & ~0x1f) + ((addr >> 27) & 0x18)) & a20_mask;
      pdpe = x86_ldq_phys(cs, pdpe_addr);
      if (!(pdpe & PG_PRESENT_MASK))
        return -1;
    }

    pde_addr =
        ((pdpe & PG_ADDRESS_MASK) + (((addr >> 21) & 0x1ff) << 3)) & a20_mask;
    pde = x86_ldq_phys(cs, pde_addr);
    if (!(pde & PG_PRESENT_MASK)) {
      return -1;
    }
    if (pde & PG_PSE_MASK) {
      /* 2 MB page */
      page_size = 2048 * 1024;
      pte = pde;
    } else {
      /* 4 KB page */
      pte_addr =
          ((pde & PG_ADDRESS_MASK) + (((addr >> 12) & 0x1ff) << 3)) & a20_mask;
      page_size = 4096;
      pte = x86_ldq_phys(cs, pte_addr);
    }
    if (!(pte & PG_PRESENT_MASK)) {
      return -1;
    }
  } else {
    uint32_t pde;

    /* page directory entry */
    pde_addr = ((env->cr[3] & ~0xfff) + ((addr >> 20) & 0xffc)) & a20_mask;
    pde = x86_ldl_phys(cs, pde_addr);
    if (!(pde & PG_PRESENT_MASK))
      return -1;
    if ((pde & PG_PSE_MASK) && (env->cr[4] & CR4_PSE_MASK)) {
      pte = pde | ((pde & 0x1fe000LL) << (32 - 13));
      page_size = 4096 * 1024;
    } else {
      /* page directory entry */
      pte_addr = ((pde & ~0xfff) + ((addr >> 10) & 0xffc)) & a20_mask;
      pte = x86_ldl_phys(cs, pte_addr);
      if (!(pte & PG_PRESENT_MASK)) {
        return -1;
      }
      page_size = 4096;
    }
    pte = pte & a20_mask;
  }

#ifdef TARGET_X86_64
out:
#endif
  pte &= PG_ADDRESS_MASK & ~(page_size - 1);
  page_offset = (addr & TARGET_PAGE_MASK) & (page_size - 1);
  return pte | page_offset;
}

void cpu_report_tpr_access(CPUX86State *env, TPRAccess access) {
  X86CPU *cpu = env_archcpu(env);
  CPUState *cs = env_cpu(env);

  if (kvm_enabled() || whpx_enabled()) {
    // env->tpr_access_type = access;
    cpu_interrupt(cs, CPU_INTERRUPT_TPR);
  } else if (tcg_enabled()) {
    cpu_restore_state(cs, cs->mem_io_pc, false);

    apic_handle_tpr_access_report(cpu->apic_state, env->eip, access);
  }
}
#endif /* !CONFIG_USER_ONLY */

// FIXME this is called by cpu_handle_interrupt, it's impressive
#if !defined(CONFIG_USER_ONLY)
void do_cpu_init(X86CPU *cpu) {
  CPUState *cs = CPU(cpu);
  CPUX86State *env = &cpu->env;
  CPUX86State *save = g_new(CPUX86State, 1);
  int sipi = cs->interrupt_request & CPU_INTERRUPT_SIPI;

  *save = *env;

  cpu_reset(cs);
  cs->interrupt_request = sipi;
  memcpy(&env->start_init_save, &save->start_init_save,
         offsetof(CPUX86State, end_init_save) -
             offsetof(CPUX86State, start_init_save));
  g_free(save);

  apic_init_reset(cpu->apic_state);
}

void do_cpu_sipi(X86CPU *cpu) { apic_sipi(cpu->apic_state); }
#else
void do_cpu_init(X86CPU *cpu) {}
void do_cpu_sipi(X86CPU *cpu) {}
#endif

/* Frob eflags into and out of the CPU temporary format.  */

void x86_cpu_exec_enter(CPUState *cs) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;

  CC_SRC = env->eflags & (CC_O | CC_S | CC_Z | CC_A | CC_P | CC_C);
  env->df = 1 - (2 * ((env->eflags >> 10) & 1));
  CC_OP = CC_OP_EFLAGS;
  env->eflags &= ~(DF_MASK | CC_O | CC_S | CC_Z | CC_A | CC_P | CC_C);
}

void x86_cpu_exec_exit(CPUState *cs) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;

  env->eflags = cpu_compute_eflags(env);
}

// FIXME implement it later
AddressSpace *cpu_addressspace(CPUState *cs, MemTxAttrs attrs);

#ifndef CONFIG_USER_ONLY
uint8_t x86_ldub_phys(CPUState *cs, hwaddr addr) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;
  MemTxAttrs attrs = cpu_get_mem_attrs(env);
  AddressSpace *as = cpu_addressspace(cs, attrs);

  return address_space_ldub(as, addr, attrs, NULL);
}

uint32_t x86_lduw_phys(CPUState *cs, hwaddr addr) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;
  MemTxAttrs attrs = cpu_get_mem_attrs(env);
  AddressSpace *as = cpu_addressspace(cs, attrs);

  return address_space_lduw(as, addr, attrs, NULL);
}

uint32_t x86_ldl_phys(CPUState *cs, hwaddr addr) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;
  MemTxAttrs attrs = cpu_get_mem_attrs(env);
  AddressSpace *as = cpu_addressspace(cs, attrs);

  return address_space_ldl(as, addr, attrs, NULL);
}

uint64_t x86_ldq_phys(CPUState *cs, hwaddr addr) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;
  MemTxAttrs attrs = cpu_get_mem_attrs(env);
  AddressSpace *as = cpu_addressspace(cs, attrs);

  return address_space_ldq(as, addr, attrs, NULL);
}

void x86_stb_phys(CPUState *cs, hwaddr addr, uint8_t val) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;
  MemTxAttrs attrs = cpu_get_mem_attrs(env);
  AddressSpace *as = cpu_addressspace(cs, attrs);

  address_space_stb(as, addr, val, attrs, NULL);
}

void x86_stl_phys_notdirty(CPUState *cs, hwaddr addr, uint32_t val) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;
  MemTxAttrs attrs = cpu_get_mem_attrs(env);
  AddressSpace *as = cpu_addressspace(cs, attrs);

  address_space_stl_notdirty(as, addr, val, attrs, NULL);
}

void x86_stw_phys(CPUState *cs, hwaddr addr, uint32_t val) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;
  MemTxAttrs attrs = cpu_get_mem_attrs(env);
  AddressSpace *as = cpu_addressspace(cs, attrs);

  address_space_stw(as, addr, val, attrs, NULL);
}

void x86_stl_phys(CPUState *cs, hwaddr addr, uint32_t val) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;
  MemTxAttrs attrs = cpu_get_mem_attrs(env);
  AddressSpace *as = cpu_addressspace(cs, attrs);

  address_space_stl(as, addr, val, attrs, NULL);
}

void x86_stq_phys(CPUState *cs, hwaddr addr, uint64_t val) {
  X86CPU *cpu = X86_CPU(cs);
  CPUX86State *env = &cpu->env;
  MemTxAttrs attrs = cpu_get_mem_attrs(env);
  AddressSpace *as = cpu_addressspace(cs, attrs);

  address_space_stq(as, addr, val, attrs, NULL);
}
#endif
