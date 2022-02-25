#include "../include/common.h"
#include "../x86tomips-config.h"
#include "../ir1/ir1.h"
#include "../ir2/ir2.h"
#include "../include/env.h"
#include "../include/etb.h"
#include "../include/reg-alloc.h"
#include "../x86tomips-options.h"
#include "../include/etb.h"
#include "../include/shadow-stack.h"
#include "../include/profile.h"

#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
#include "../x86tomips-profile-sys.h"

#define XTM_PF_LSENV_OFFSET_FUNC(group, name) \
int lsenv_offset_of_pf_data_ ## group ## _ ## name (ENV *lsenv) \
{ \
    return offsetof(CPUX86State, xtm_pf_data.group.name); \
}

XTM_PF_LSENV_OFFSET_FUNC(jc, is_jmpdr)
XTM_PF_LSENV_OFFSET_FUNC(jc, is_jmpin)
XTM_PF_LSENV_OFFSET_FUNC(jc, is_sys_eob)
XTM_PF_LSENV_OFFSET_FUNC(jc, is_excp)

XTM_PF_LSENV_OFFSET_FUNC(tbf, is_mov)
XTM_PF_LSENV_OFFSET_FUNC(tbf, is_pop)

#endif

/* OffsetOf: integer registers */
int lsenv_offset_of_mips_iregs(ENV *lsenv, int i)
{
    CPUX86State *env = lsenv->cpu_state;
    return (int)((ADDR)(&env->mips_iregs[i]) - (ADDR)env);
}
int lsenv_offset_of_gpr(ENV *lsenv, int i)
{
    CPUX86State *env = lsenv->cpu_state;
    return (int)((ADDR)(&env->regs[i]) - (ADDR)env);
}

/* OffsetOf: eflags */
int lsenv_offset_of_eflags(ENV *lsenv) { return offsetof(CPUX86State, eflags); }
int lsenv_offset_of_cc_src(ENV *lsenv) { return offsetof(CPUX86State, cc_src); }
int lsenv_offset_of_cc_op(ENV *lsenv)  { return offsetof(CPUX86State, cc_op); }

/* OffsetOf: FPU */
int lsenv_offset_of_top(ENV *lsenv)          { return offsetof(CPUX86State, fpstt); }
int lsenv_offset_of_status_word(ENV *lsenv)  { return offsetof(CPUX86State, fpus); }
int lsenv_offset_of_control_word(ENV *lsenv) { return offsetof(CPUX86State, fpuc); }
int lsenv_offset_of_tag_word(ENV *lsenv)     { return offsetof(CPUX86State, fptags); }
int lsenv_offset_of_fptags(ENV *lsenv)       { return offsetof(CPUX86State, fptags); }
int lsenv_offset_of_fpr(ENV *lsenv, int i)
{
    CPUX86State *env = lsenv->cpu_state;
    return (int)((ADDR) & (env->fpregs[i].d) - (ADDR)env);
}
/* GetSet: FPU */
FPReg lsenv_get_fpregs(ENV *lsenv, int i) { return ((CPUX86State *)lsenv->cpu_state)->fpregs[i]; }
void  lsenv_set_fpregs(ENV *lsenv, int i, FPReg new_value) { ((CPUX86State *)lsenv->cpu_state)->fpregs[i] = new_value; }
int   lsenv_get_top(ENV *lsenv) { return ((CPUX86State *)lsenv->cpu_state)->fpstt; }
void  lsenv_set_top(ENV *lsenv, int new_fpstt) { ((CPUX86State *)lsenv->cpu_state)->fpstt = new_fpstt; }
int   lsenv_get_fpu_control_word(ENV *lsenv) { return ((CPUX86State *)lsenv->cpu_state)->fpuc; }

/* OffsetOf: SIMD */
int lsenv_offset_of_mmx(ENV *lsenv, int i)
{
    CPUX86State *env = lsenv->cpu_state;
    return (int)((ADDR)(&env->fpregs[i]) - (ADDR)env);
}
int lsenv_offset_of_xmm(ENV *lsenv, int i)
{
    CPUX86State *cpu = lsenv->cpu_state;
    return (int)((ADDR)(&cpu->xmm_regs[i]) - (ADDR)lsenv->cpu_state);
}
int lsenv_offset_of_mmx_t0(ENV *lsenv)  { return offsetof(CPUX86State, mmx_t0); }
int lsenv_offset_of_xmm_t0(ENV *lsenv)  { return offsetof(CPUX86State, xmm_t0); }
int lsenv_offset_of_mxcsr(ENV *lsenv)   { return offsetof(CPUX86State, mxcsr); }

/* OffsetOf: segment registers */
int lsenv_offset_of_seg_base(ENV *lsenv, int i)
{
    CPUX86State *env = lsenv->cpu_state;
    return (int)((ADDR)(&env->segs[i].base) - (ADDR)env);
}
int lsenv_offset_of_seg_selector(ENV *lsenv, int i)
{
    CPUX86State *env = lsenv->cpu_state;
    return (int)((ADDR)(&env->segs[i].selector) - (ADDR)env);
}
int lsenv_offset_of_seg_limit(ENV *lsenv, int i)
{
    CPUX86State *env= lsenv->cpu_state;
    return (int)((ADDR)(&env->segs[i].limit) - (ADDR)env);
}
int lsenv_offset_of_seg_flags(ENV *lsenv, int i)
{
    CPUX86State *env = lsenv->cpu_state;
    return (int)((ADDR)(&env->segs[i].flags) - (ADDR)env);
}
int lsenv_offset_of_gdt_base(ENV *lsenv)  { return offsetof(CPUX86State, gdt.base); }
int lsenv_offset_of_gdt_limit(ENV *lsenv) { return offsetof(CPUX86State, gdt.limit); }

/* OffsetOf: others */
int lsenv_offset_of_eip(ENV *lsenv)     { return offsetof(CPUX86State, eip); }
int lsenv_offset_of_df(ENV *lsenv)      { return offsetof(CPUX86State, df); }
int lsenv_offset_of_tr_data(ENV *lsenv) { return offsetof(ENV, tr_data); }

int lsenv_get_last_executed_tb_top_out(ENV *lsenv)
{
    TranslationBlock *tb = (TranslationBlock*)lsenv_get_last_executed_tb(lsenv);
    ETB *etb = tb->extra_tb;
    lsassert(etb);
    return etb->_top_out;
}

/* OffsetOF and GetSet: virtual registers */
int lsenv_offset_of_vreg(ENV *lsenv, int i)
{
    CPUX86State *cpu = lsenv->cpu_state;
    return (int)((ADDR)(&cpu->vregs[i]) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_guest_base(ENV *lsenv)       { return lsenv_offset_of_vreg(lsenv, 0); }
int lsenv_offset_of_last_executed_tb(ENV *lsenv) { return lsenv_offset_of_vreg(lsenv, 1); }
int lsenv_offset_of_next_eip(ENV *lsenv)         { return lsenv_offset_of_vreg(lsenv, 2); }
int lsenv_offset_of_top_bias(ENV *lsenv)         { return lsenv_offset_of_vreg(lsenv, 3); }
int lsenv_offset_of_ss(ENV *lsenv)               { return lsenv_offset_of_vreg(lsenv, 4); }
int lsenv_offset_of_fast_cs_mask(ENV *lsenv)     { return lsenv_offset_of_vreg(lsenv, 5); }

ADDR lsenv_get_vreg(ENV *lsenv, int i)
{
    CPUX86State *env = lsenv->cpu_state;
    return (ADDR)env->vregs[i];
}
void lsenv_set_vreg(ENV *lsenv, int i, ADDR val)
{
    CPUX86State *env = lsenv->cpu_state;
    env->vregs[i] = (uint64_t)val;
}

ADDR  lsenv_get_guest_base(ENV *lsenv)       { return (ADDR) lsenv_get_vreg(lsenv, 0); }
ADDR  lsenv_get_last_executed_tb(ENV *lsenv) { return (ADDR) lsenv_get_vreg(lsenv, 1); }
ADDRX lsenv_get_next_eip(ENV *lsenv)         { return (ADDRX)lsenv_get_vreg(lsenv, 2); }
int   lsenv_get_top_bias(ENV *lsenv)         { return (int)  lsenv_get_vreg(lsenv, 3); }
int   lsenv_get_fast_cs_mask(ENV *lsenv)     { return (int)  lsenv_get_vreg(lsenv, 5); }

void lsenv_set_guest_base(ENV *lsenv, ADDR gbase)    { lsenv_set_vreg(lsenv, 0, gbase); }
void lsenv_set_last_executed_tb(ENV *lsenv, ADDR tb) { lsenv_set_vreg(lsenv, 1, tb); }
void lsenv_set_next_eip(ENV *lsenv, ADDRX eip)       { lsenv_set_vreg(lsenv, 2, eip); }
void lsenv_set_top_bias(ENV *lsenv, int top_bias)    { lsenv_set_vreg(lsenv, 3, top_bias); }
void lsenv_set_fast_cs_mask(ENV *lsenv, int mask)    { lsenv_set_vreg(lsenv, 5, mask); }

#ifdef CONFIG_SOFTMMU

int lsenv_offset_of_gdtr_base(ENV *lsenv)  { return offsetof(CPUX86State, gdt.base); }
int lsenv_offset_of_gdtr_limit(ENV *lsenv) { return offsetof(CPUX86State, gdt.limit); }
int lsenv_offset_of_idtr_base(ENV *lsenv)  { return offsetof(CPUX86State, idt.base); }
int lsenv_offset_of_idtr_limit(ENV *lsenv) { return offsetof(CPUX86State, idt.limit); }
int lsenv_offset_of_ldtr_selector(ENV *lsenv) { return offsetof(CPUX86State, ldt.selector); }
int lsenv_offset_of_tr_selector(ENV *lsenv) { return offsetof(CPUX86State, tr.selector); }

int lsenv_offset_of_cr(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->cr[i]) - (ADDR)cpu);
}

int lsenv_offset_of_hflags(ENV *lsenv) { return offsetof(CPUX86State, hflags); }

uint32 lsenv_get_hflags(ENV* lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return cpu->hflags;
}

ADDRX lsenv_get_eip(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State*)lsenv->cpu_state;
    return (ADDRX)cpu->eip;
}

#endif
