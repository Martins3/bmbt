/* X86toMIPS headers */
#include "../include/common.h"
#include "../include/reg_alloc.h"
#include "../include/env.h"
#include "../ir2/ir2.h"
#include "../x86tomips-options.h"

/* QEMU headers */
#include "../include/qemu-def.h"

#ifdef CONFIG_XTM_PROFILE
#include "../x86tomips-profile-sys.h"
#endif

/* Macro : Exception Check for specific instructions */
#include "../translator/tr_excp.h"

/* Execute after every ir1_disasm to fix something
 *
 * 1> A little bug of capstone
 *    Under the following situction :
 *    - address size is 2
 *    - modrm is used and
 *      - mod is 0
 *      - rm is 6
 *    The disp should be considered as unsigned 16-bit value.
 *
 * 2. Set mem opnd's default segment
 *
 * 3. opnd size for rep string operation
 *
 * */
void fix_up_ir1(IR1_INST *ir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    int i = 0;

    /* 1. A little bug of capstone */
    if (ir1_addr_size(ir1) == 2) {
        uint8 modrm = ir1->info->detail->x86.modrm;
        uint8 mod = (modrm >> 6) & 0x3;
        uint8 rm  = (modrm >> 0) & 0x7;
        if (mod == 0 && rm == 6) {
            /* disp should be unsigend */
            int64 disp_old = ir1->info->detail->x86.disp;
            int64 disp_new = disp_old & 0xffff;
            ir1->info->detail->x86.disp = disp_new;
            for ( i = 0; i < ir1->info->detail->x86.op_count; ++i) {
                IR1_OPND *opnd = ir1_get_opnd(ir1, i);
                if (ir1_opnd_is_mem(opnd)) {
                    opnd->mem.disp = disp_new;
                    goto fix_up_2;
                }
            }
        }
    }

fix_up_2:
    /* 2. set mem opnd's default segment
     * --------------------------------------------
     * seg |    situction       | who deals with it
     * --------------------------------------------
     *  CS | instruction fetch  | QEMU
     * --------------------------------------------
     *  SS | stack push pop     | transalte_pop/push
     *     | base = EBP or ESP  | here !!!
     * --------------------------------------------
     *  DS | All data reference | here !!!
     * --------------------------------------------
     *  ES | strint operation's | translate_ins/outs
     *     | destination        | translate_movs
     * -------------------------------------------- */
    if (td->sys.addseg && !ir1_is_lea(ir1)) {
        for ( i = 0; i < ir1->info->detail->x86.op_count; ++i) {
            IR1_OPND *opnd = ir1_get_opnd(ir1, i);
            if (ir1_opnd_is_mem(opnd) && !ir1_opnd_has_seg(opnd)) {
                int base = ir1_opnd_base_reg_num(opnd);
                if (base == esp_index || base == ebp_index) {
                    opnd->mem.segment = X86_REG_SS;
                }
                else {
                    opnd->mem.segment = X86_REG_DS;
                }
            }
        }
    }

    /* 3. opnd size for 'rep movs/outs/ins... string operation'
     *
     * When inst is prefixed with '0x66 0xf3', only the '0xf3' (rep)
     * will be disassembled. But if it is prefixed with '0xf3 0x66',
     * the '0x66' (opnd size) will also be disassembled.
     *
     * This bug only exists in 32-bit mode between 16 and 32.
     *
     * -------------------------------------
     *   opc |      default opnd size
     * -------------------------------------
     *  inst | 8-bits |  16-bits | 32-bits
     * -------------------------------------
     *   ins |  0x6c  |   0x6d   |  0x6d
     *  outs |  0x6e  |   0x6f   |  0x6f
     *  movs |  0xa4  |   0xa5   |  0xa5
     *  cmps |  0xa6  |   0xa7   |  0xa7
     *  stos |  0xaa  |   0xab   |  0xab
     *  lods |  0xac  |   0xad   |  0xad
     *  scas |  0xae  |   0xaf   |  0xaf
     * ------------------------------------- */
    if (ir1_is_string_op(ir1)) {
        uint8 *opbytes = ir1_inst_opbytes(ir1);
        if (td->sys.code32 && opbytes[0] & 1) {
            int data_size  = ir1_data_size(ir1) >> 3;
            IR1_OPND *opnd  = NULL;
            IR1_OPND *opnd2 = NULL;
            switch (opbytes[0]) {
            case 0x6d: /* ins  */
                opnd = ir1_get_opnd(ir1, 0);
                break;
            case 0x6f: /* outs */
                opnd = ir1_get_opnd(ir1, 1);
                break;
            case 0xa5: /* movs */
            case 0xa7: /* cmps */
            case 0xab: /* stos */
            case 0xad: /* lods */
            case 0xaf: /* scas */
                opnd  = ir1_get_opnd(ir1, 0);
                opnd2 = ir1_get_opnd(ir1, 1);
                break;
            default: lsassert(0); break;
            }
            if (opnd)  opnd->size  = data_size;
            if (opnd2) opnd2->size = data_size;
                
        }
    }
}

#ifdef CONFIG_BTMMU
void tr_enable_btmmu(void)  { lsenv->tr_data->is_btmmu_ok = 1; }
void tr_disable_btmmu(void) { lsenv->tr_data->is_btmmu_ok = 0; }
void tr_reset_btmmu(void)   { lsenv->tr_data->is_btmmu_ok = 1; }
#endif

void tr_sys_init(
        TranslationBlock *tb,
        int max_insns,
        void *code_highwater)
{
    if (!tb)
        return;

    CPUX86State *env = lsenv->cpu_state;
    TRANSLATION_DATA *td = lsenv->tr_data;

    lsassert(max_insns >= 1);
    td->max_insns = max_insns > MAX_IR1_NUM_PER_TB ? MAX_IR1_NUM_PER_TB : max_insns;
    td->code_highwater = code_highwater;

    uint32 flags  = tb->flags;
    uint32 cflags = tb->cflags;

#ifdef CONFIG_BTMMU
    tr_reset_btmmu();
#endif

    td->is_top_saved = 0;

    td->slow_path_rcd_nr = 0;

    td->sys.pe = (flags >> HF_PE_SHIFT) & 1;
    td->sys.code32 = (flags >> HF_CS32_SHIFT) & 1;
    td->sys.ss32 = (flags >> HF_SS32_SHIFT) & 1;
    td->sys.addseg = (flags >> HF_ADDSEG_SHIFT) & 1;
    td->sys.f_st = 0;
    td->sys.vm86 = (flags >> VM_SHIFT) & 1;
    td->sys.cpl = (flags >> HF_CPL_SHIFT) & 3;
    td->sys.iopl = (flags >> IOPL_SHIFT) & 3;
    td->sys.tf = (flags >> TF_SHIFT) & 1;
    td->sys.cs_base = tb->cs_base;
    td->sys.popl_esp_hack = 0;

    td->sys.mem_index = cpu_mmu_index(env, false);

    td->sys.cpuid_features = env->features[FEAT_1_EDX];
    td->sys.cpuid_ext_features = env->features[FEAT_1_ECX];
    td->sys.cpuid_ext2_features = env->features[FEAT_8000_0001_EDX];
    td->sys.cpuid_ext3_features = env->features[FEAT_8000_0001_ECX];
    td->sys.cpuid_7_0_ebx_features = env->features[FEAT_7_0_EBX];
    td->sys.cpuid_xsave_features = env->features[FEAT_XSAVE];

    td->sys.flags = flags;
    td->sys.cflags = cflags;

    td->sys.pc = tb->pc;
    td->sys.bp_hit = 0;

    if (option_dump) {
        fprintf(stderr, "SYS.PE     = %d\n", td->sys.pe);
        fprintf(stderr, "SYS.code32 = %d\n", td->sys.code32);
        fprintf(stderr, "SYS.ss32   = %d\n", td->sys.ss32);
        fprintf(stderr, "SYS.addseg = %d\n", td->sys.addseg);
        fprintf(stderr, "SYS.vm86   = %d\n", td->sys.vm86);
        fprintf(stderr, "SYS.cpl    = %d\n", td->sys.cpl);
        fprintf(stderr, "SYS.iopl   = %d\n", td->sys.iopl);
        fprintf(stderr, "SYS.tf     = %d\n", td->sys.tf);
        fprintf(stderr, "SYS.CSBase = %p\n", (void*)(ADDR)td->sys.cs_base);
        fprintf(stderr, "SYS.MMUIdx = %d\n", td->sys.mem_index);
    }

    td->need_eob    = 1; /* always generate eob */
    td->inhibit_irq = 0;
    td->recheck_tf  = 0;
    td->ignore_eip_update = 0;
    td->ignore_top_update = 0;

    td->in_context_saved    = 0;
    td->context_save_depth  = 0;
    td->helper_cfg = default_helper_cfg;
}

int get_sys_stack_addr_size(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (td->sys.ss32)
        return 4;
    else
        return 2;
}

bool translate_lidt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LIDT(pir1);

    IR1_OPND* opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_mem(opnd),
            "not a valid LIDT insn: pir1 = %p\n", (void*)pir1);

    /* In i386: always load 6 bytes
     *  >  ir1_opnd_size(opnd) == 6, not a regular load operation
     *
     * 2 bytes for limit, and limit is always 16-bits long
     *
     * 4 bytes for base address, and base assress is 24-bits long
     * in real-address mode and vm86 mode, 32-bits long in PE mode. */
    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd, -1);

    IR2_OPND limit = ra_alloc_itemp();
    IR2_OPND base  = ra_alloc_itemp();

    int save_temp = 1;

    /* 1. load 2 bytes for limit at MEM(addr)*/
    gen_ldst_softmmu_helper(LISA_LD_HU, &limit, &mem_opnd, save_temp);

    /* 2. load 4 bytes for base address at MEM(addr + 2)*/
    IR2_OPND mem = convert_mem_ir2_opnd_plus_2(&mem_opnd);
    gen_ldst_softmmu_helper(LISA_LD_WU, &base, &mem, save_temp);

    /* 3. 24-bits long base address in Real-Address mode and vm86 mode */
    if (!td->sys.pe || td->sys.vm86) {
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND tmp1 = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRAI_D, &tmp, &base, 0x10);
        append_ir2_opnd2i(LISA_ANDI, &tmp, &tmp, 0xff);
        append_ir2_opnd2i(LISA_SLLI_D, &tmp, &tmp, 0x10);
        append_ir2_opnd2_(lisa_mov16z, &tmp1, &base);
        append_ir2_opnd3(LISA_OR, &tmp, &tmp, &tmp1);
        base = tmp;
    }

    /* 4. store limit/base into IDTR */
    append_ir2_opnd2i(LISA_ST_W, &base, &env_ir2_opnd,
                      lsenv_offset_of_idtr_base(lsenv));
    append_ir2_opnd2i(LISA_ST_W, &limit, &env_ir2_opnd,
                      lsenv_offset_of_idtr_limit(lsenv));

    return true;
}

bool translate_sidt(IR1_INST *pir1)
{
    IR1_OPND* opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_mem(opnd),
            "not a valid SIDT insn: pir1 = %p\n", (void*)pir1);

    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd, -1);

    /* 1. load limit/base into temp register */
    IR2_OPND base = ra_alloc_itemp();
    IR2_OPND limit = ra_alloc_itemp();

    append_ir2_opnd2i(LISA_LD_W, &base, &env_ir2_opnd,
		      lsenv_offset_of_idtr_base(lsenv));
    append_ir2_opnd2i(LISA_LD_W, &limit, &env_ir2_opnd,
		      lsenv_offset_of_idtr_limit(lsenv));

    int save_temp = 1;
    /* 2. store 2 bytes limit at MEM(addr) */
    gen_ldst_softmmu_helper(LISA_ST_H, &limit, &mem_opnd, save_temp);

    /* 3. store 4 bytes base at MEM(addr + 2) */
    IR2_OPND mem = convert_mem_ir2_opnd_plus_2(&mem_opnd);
    if (ir1_addr_size(pir1) == 2) {
        append_ir2_opnd2_(lisa_mov24z, &base, &base);
    }
    gen_ldst_softmmu_helper(LISA_ST_W, &base, &mem, save_temp);

    ra_free_temp(&base);
    ra_free_temp(&limit);
    return true;
}

bool translate_lgdt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LGDT(pir1);

    IR1_OPND* opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_mem(opnd),
            "not a valid LGDT insn: pir1 = %p\n", (void*)pir1);

    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd, -1);

    IR2_OPND limit = ra_alloc_itemp();
    IR2_OPND base  = ra_alloc_itemp();

    int save_temp = 1;

    /* 1. load 2 bytes for limit at MEM(addr)*/
    gen_ldst_softmmu_helper(LISA_LD_HU, &limit, &mem_opnd, save_temp);

    /* 2. load 4 bytes for base address at MEM(addr + 2)*/
    IR2_OPND mem = convert_mem_ir2_opnd_plus_2(&mem_opnd);
    gen_ldst_softmmu_helper(LISA_LD_WU, &base, &mem, save_temp);

    /* 3. 24-bits long base address in Real-Address mode and vm86 mode */
    if (!td->sys.pe || td->sys.vm86) {
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND tmp1 = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRAI_D, &tmp,  &base, 0x10);
        append_ir2_opnd2i(LISA_ANDI,   &tmp,  &tmp,  0xff);
        append_ir2_opnd2i(LISA_SLLI_D, &tmp,  &tmp,  0x10);
        append_ir2_opnd2_(lisa_mov16z, &tmp1, &base);
        append_ir2_opnd3 (LISA_OR,     &tmp,  &tmp,  &tmp1);
        base = tmp;
    }

    /* 4. store limit/base into GDTR */
    append_ir2_opnd2i(LISA_ST_W, &base, &env_ir2_opnd,
                      lsenv_offset_of_gdtr_base(lsenv));
    append_ir2_opnd2i(LISA_ST_W, &limit, &env_ir2_opnd,
                      lsenv_offset_of_gdtr_limit(lsenv));

    return true;
}

bool translate_sgdt(IR1_INST *pir1)
{
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_mem(opnd),
            "not a valid SGDT insn: pir1 = %p\n", (void*)pir1);

    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd, -1);

    /* 0. mem_opnd might be temp register */
    int save_temp = 1;

    /* 1. load gdtr.limit from env */
    IR2_OPND limit = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_W, &limit, &env_ir2_opnd,
                      lsenv_offset_of_gdtr_limit(lsenv));
  /* 2. store 16-bits limit at MEM(addr) */
    gen_ldst_softmmu_helper(LISA_ST_H, &limit, &mem_opnd, save_temp);
    ra_free_temp(&limit);

    /* 3. load gdtr.base  from env */
    IR2_OPND base  = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_W, &base, &env_ir2_opnd,
                      lsenv_offset_of_gdtr_base(lsenv));

    /* 4. store 32-bit base at MEM(addr + 2) */
    IR2_OPND mem = convert_mem_ir2_opnd_plus_2(&mem_opnd);
    gen_ldst_softmmu_helper(LISA_ST_W, &base, &mem, save_temp);
    ra_free_temp(&base);

    return true;
}

static void translate_lldt_gpr(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* helper might cause exception, save complete CPUX86State */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_lldt(CPUX86State  *env, int selector) */

    /* 1. get selector into arg1 (a1) */
    load_ir1_gpr_to_ir2(&arg1_ir2_opnd, opnd0, ZERO_EXTENSION);

    /* 2. call helper */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    tr_gen_call_to_helper((ADDR)helper_lldt);

    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

static void translate_lldt_mem(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* 0. load selector value */
    IR2_OPND selector = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&selector, opnd0, ZERO_EXTENSION, false, -1);

    /* helper might cause exception, save complete CPUX86State */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_lldt(CPUX86State  *env, int selector) */

    /* 1. get selector into arg1 (a1) */
    append_ir2_opnd3(LISA_OR, &arg1_ir2_opnd, &selector, &zero_ir2_opnd);

    /* 2. call helper */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    tr_gen_call_to_helper((ADDR)helper_lldt);

    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

bool translate_lldt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LLDT(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR or MEM */

    if (ir1_opnd_is_gpr(opnd0)) {
        translate_lldt_gpr(pir1, opnd0);
    } else {
        lsassertm_illop(ir1_addr(pir1),
                ir1_opnd_is_mem(opnd0),
                "not a valid LLDT insn: pir1 = %p\n", (void*)pir1);

        translate_lldt_mem(pir1, opnd0);
    }

    return true;
}

bool translate_sldt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_SLDT(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR or MEM */

    /* load ldtr.selector from env */
    IR2_OPND selector = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_W, &selector, &env_ir2_opnd,
                      lsenv_offset_of_ldtr_selector(lsenv));

    store_ir2_to_ir1(&selector, opnd0, false);

    return true;
}

static void translate_ltr_gpr(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* helper might cause exception, save complete CPUX86State */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_ltr(CPUX86State  *env, int selector) */

    /* 1. get selector into arg1 (a1) */
    load_ir1_gpr_to_ir2(&arg1_ir2_opnd, opnd0, ZERO_EXTENSION);

    /* 2. call helper */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    tr_gen_call_to_helper((ADDR)helper_ltr);

    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

static void translate_ltr_mem(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* helper might cause exception, save complete CPUX86State */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_ltr(CPUX86State  *env, int selector) */

    /* 1. get selector into arg1 (a1) */
    IR2_OPND selector = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&selector, opnd0, ZERO_EXTENSION, false, -1);
    append_ir2_opnd3(LISA_OR, &arg1_ir2_opnd, &selector, &zero_ir2_opnd);

    /* 2. call helper */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    tr_gen_call_to_helper((ADDR)helper_ltr);

    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

bool translate_ltr(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LTR(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR or MEM */

    if (ir1_opnd_is_gpr(opnd0)) {
        translate_ltr_gpr(pir1, opnd0);
    } else {
        lsassertm_illop(ir1_addr(pir1),
                ir1_opnd_is_mem(opnd0),
                "not a valid LTR insn: pir1 = %p\n", (void*)pir1);
        translate_ltr_mem(pir1, opnd0);
    }

    return true;
}

bool translate_str(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_STR(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    /* load tr.selector from env */
    IR2_OPND selector = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_W, &selector, &env_ir2_opnd,
                      lsenv_offset_of_tr_selector(lsenv));

    store_ir2_to_ir1(&selector, opnd0, false);

    return true;
}

/* End of TB in system-mode */
bool translate_clts(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_CLTS(pir1);

    /* void helper_clts(CPUX86State  *env) */

    IR2_OPND cr0 = ra_alloc_itemp();
    IR2_OPND hf = ra_alloc_itemp();

    int off_cr0 = lsenv_offset_of_cr(lsenv, 0);
    int off_hf  = lsenv_offset_of_hflags(lsenv);

    append_ir2_opnd2i(LISA_LD_WU, &cr0, &env_ir2_opnd, off_cr0);
    append_ir2_opnd2i(LISA_LD_WU, &hf, &env_ir2_opnd, off_hf);

    IR2_OPND mask = ra_alloc_itemp();

    /*env->cr[0] &= ~CR0_TS_MASK;*/
    append_ir2_opnd2i(LISA_ORI, &mask, &zero_ir2_opnd, CR0_TS_MASK);
    append_ir2_opnd3 (LISA_NOR, &mask, &zero_ir2_opnd, &mask);
    append_ir2_opnd3 (LISA_AND, &cr0, &cr0, &mask);

    /*env->hflags &= ~HF_TS_MASK;*/
    append_ir2_opnd2i(LISA_ORI, &mask, &zero_ir2_opnd, HF_TS_MASK);
    append_ir2_opnd3 (LISA_NOR, &mask, &zero_ir2_opnd, &mask);
    append_ir2_opnd3 (LISA_AND, &hf, &hf, &mask);

    append_ir2_opnd2i(LISA_ST_W, &cr0, &env_ir2_opnd, off_cr0);
    append_ir2_opnd2i(LISA_ST_W, &hf, &env_ir2_opnd, off_hf);

    return true;
}

static void translate_jmp_far_pe_imm(
        IR1_INST *pir1,
        IR1_OPND *opnd0)
{
    /*  void
     *  helper_ljmp_protected(
     *      CPUX86State     *env,
     *      int             new_cs,
     *      target_ulong    new_eip,
     *      target_ulong    next_eip) */

    int   new_cs   = 0;
    ADDRX new_eip  = 0;
    ADDRX next_eip = ir1_addr_next(pir1);

    /*                  ptr16:16 ptr16:32
     * opnd[0]:selector    16       16    
     * opnd[1]:offset      16       32    */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1,1);
    int opnd1_size  = ir1_opnd_size(opnd1);

    new_cs  = ir1_opnd_simm(opnd0);
    new_eip = ir1_opnd_uimm(opnd1);

    /* 1. save native context */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 2. parameters */
    /* 2.1 arg0: env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 2.2 arg1: new_cs, 16-bits */
    load_imm32_to_ir2(&arg1_ir2_opnd, new_cs & 0xffff, ZERO_EXTENSION);
    /* 2.3 arg2: new_eip, 16/32-bits */
    if (opnd1_size == 16) {
        load_imm32_to_ir2(&arg2_ir2_opnd, new_eip & 0xffff, ZERO_EXTENSION);
    } else { /* opnd1_size == 32 */
        load_imm32_to_ir2(&arg2_ir2_opnd, new_eip & 0xffffffff, ZERO_EXTENSION);
    }
    /* 2.4 arg3: next_eip, 32-bits */
    load_imm32_to_ir2(&arg3_ir2_opnd, next_eip, ZERO_EXTENSION);

    /* 3. call helper */
    tr_gen_call_to_helper((ADDR)helper_ljmp_protected);

    /* 4. restore native context */
    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 5. context switch */
    lsenv->tr_data->ignore_eip_update = 1;
    tr_generate_exit_tb(pir1, 1);
}

static void translate_jmp_far_pe_mem(
        IR1_INST *pir1,
        IR1_OPND *opnd0)
{
    /*  void
     *  helper_ljmp_protected(
     *      CPUX86State     *env,
     *      int             new_cs,
     *      target_ulong    new_eip,
     *      target_ulong    next_eip) */

    /*           m16:16     m16:32
     * offset      16         32     MEM(addr)
     * selector    16         16     MEM(addr+2/4) */
    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd0, -1);

    /* 1. load offset and selector */
    IR2_OPND offset_opnd   = ra_alloc_itemp();
    IR2_OPND selector_opnd = ra_alloc_itemp();

    int opnd_size = ir1_opnd_size(opnd0);

    switch (opnd_size) {
        case 32: /* m16:16 */
            gen_ldst_softmmu_helper(LISA_LD_WU, &offset_opnd, &mem_opnd, 0);
            append_ir2_opnd2i(LISA_SRAI_D, &selector_opnd, &offset_opnd, 0x10);
            append_ir2_opnd2_(lisa_mov16z, &offset_opnd, &offset_opnd);
            break;
        case 48: /* m16:32 */
            /* load 32-bit offset */
            gen_ldst_softmmu_helper(LISA_LD_WU, &offset_opnd, &mem_opnd, 1);
            /* memory address += 4 */
            IR2_OPND mem_opnd_adjusted = convert_mem_ir2_opnd_plus_4(&mem_opnd);
            /* load 16-bit selector */
            gen_ldst_softmmu_helper(LISA_LD_HU, &selector_opnd, &mem_opnd_adjusted, 1);
            break;
        default: 
            lsassertm_illop(ir1_addr(pir1), 0,
                    "unsupported opnd size %d in ljmp mem.\n", opnd_size);
            break;
    }

    ra_free_temp(&mem_opnd);

    /* 2. save native context here */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 3. parameters */
    /* 3.1 arg1: new_cs, 16-bits
       3.2 arg2: new_eip, 16/32-bits */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &selector_opnd);
    append_ir2_opnd2_(lisa_mov, &arg2_ir2_opnd, &offset_opnd);
    /* 3.4 arg3: next_eip, 32-bits */
    ADDRX next_eip = ir1_addr_next(pir1);
    load_imm32_to_ir2(&arg3_ir2_opnd, next_eip, ZERO_EXTENSION);
    /* 3.5 arg0: env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);

    /* 4. call helper */
    tr_gen_call_to_helper((ADDR)helper_ljmp_protected);

    /* 5. restore native context */
    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 6. context switch */
    lsenv->tr_data->ignore_eip_update = 1;
    tr_generate_exit_tb(pir1, 1);
}

static void translate_jmp_far_real_imm(IR1_INST *pir1, IR1_OPND *opnd0)
{
    uint32_t selector;
    ADDRX base;

    IR2_OPND tmp0 = ra_alloc_itemp();
    IR2_OPND tmp1 = ra_alloc_itemp();

    /*                  ptr16:16 ptr16:32
     * opnd[0]:selector    16       16    
     * opnd[1]:offset      16       32    */
    selector = ir1_opnd_uimm(opnd0);
    base     = selector << 0x4;

    load_imm32_to_ir2(&tmp0, selector, ZERO_EXTENSION);
    load_addrx_to_ir2(&tmp1, base);

    append_ir2_opnd2i(LISA_ST_W, &tmp0, &env_ir2_opnd,
                      lsenv_offset_of_seg_selector(lsenv, R_CS));
    append_ir2_opnd2i(LISA_ST_W, &tmp1, &env_ir2_opnd,
                      lsenv_offset_of_seg_base(lsenv, R_CS));

    tr_generate_exit_tb(pir1, 1);
}

static void translate_jmp_far_real_mem(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* m16:16/m16:32/m16:64 */
    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd0, -1);

    IR2_OPND dest = ra_alloc_itemp();
    IR2_OPND next_eip = ra_alloc_itemp();

    /* m16:16 */
    if (ir1_opnd_size(opnd0) == 32) {
        IR2_OPND tmp0 = ra_alloc_itemp();

        gen_ldst_softmmu_helper(LISA_LD_W, &dest, &mem_opnd, 1);

        append_ir2_opnd2_(lisa_mov16z, &next_eip, &dest);
        
        append_ir2_opnd2i(LISA_SRLI_W, &tmp0, &dest, 0x10);
        append_ir2_opnd2_(lisa_mov16z, &tmp0, &tmp0);
        append_ir2_opnd2i(LISA_ST_W, &tmp0, &env_ir2_opnd,
                          lsenv_offset_of_seg_selector(lsenv, R_CS));

        append_ir2_opnd2i(LISA_SLLI_W, &tmp0, &tmp0, 0x4);
        append_ir2_opnd2i(LISA_ST_W, &tmp0, &env_ir2_opnd,
                          lsenv_offset_of_seg_base(lsenv, R_CS));

        ra_free_temp(&tmp0);
    }
    /* m16:32 */
    else if (ir1_opnd_size(opnd0) == 48) {
        /* load 32-bit offset */
        gen_ldst_softmmu_helper(LISA_LD_WU, &dest, &mem_opnd, 1);
        append_ir2_opnd2_(lisa_mov, &next_eip, &dest);

        /* load 16-bit selector */
        IR2_OPND mem_opnd_adjusted = convert_mem_ir2_opnd_plus_4(&mem_opnd);

        gen_ldst_softmmu_helper(LISA_LD_HU, &dest, &mem_opnd_adjusted, 1);
        append_ir2_opnd2i(LISA_ST_W, &dest, &env_ir2_opnd,
                          lsenv_offset_of_seg_selector(lsenv, R_CS));

        append_ir2_opnd2i(LISA_SLLI_D, &dest, &dest, 0x4);
        append_ir2_opnd2i(LISA_ST_W, &dest, &env_ir2_opnd,
                          lsenv_offset_of_seg_base(lsenv, R_CS));
    }
    /* m16:64 */
    else {
        lsassertm_illop(ir1_addr(pir1), 0,
            "unsupported opnd size %d in ljmp mem.\n", ir1_opnd_size(opnd0));
    }

    ra_free_temp(&dest);

    IR2_OPND succ_x86_addr_opnd = ra_alloc_dbt_arg2();
    append_ir2_opnd2_(lisa_mov, &succ_x86_addr_opnd, &next_eip);
    tr_generate_exit_tb(pir1, 1);
}

bool translate_jmp_far(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    /* protected mode && not vm86 mode */
    if (td->sys.pe && !td->sys.vm86) {
        if (ir1_opnd_is_imm(opnd0)) {
            translate_jmp_far_pe_imm(pir1, opnd0);
        } else if (ir1_opnd_is_mem(opnd0)) {
            translate_jmp_far_pe_mem(pir1, opnd0);
        } else {
            lsassert(0);
        }
        return true;
    }

    /* Real-Address mode || vm86 mode */
    if (ir1_opnd_is_imm(opnd0)) {
        translate_jmp_far_real_imm(pir1, opnd0);
    } else if (ir1_opnd_is_mem(opnd0)) {
        translate_jmp_far_real_mem(pir1, opnd0);
    }

    return true;
}

bool translate_cli(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_CLI(pir1);

    IR2_OPND *eflags = &eflags_ir2_opnd;
    IR2_OPND mask = ra_alloc_itemp();

    /* helper_cli */
    append_ir2_opnd2i(LISA_ORI, &mask, &zero_ir2_opnd, IF_BIT);
    append_ir2_opnd3 (LISA_NOR, &mask, &zero_ir2_opnd, &mask);
    append_ir2_opnd3 (LISA_AND, eflags, eflags, &mask);

    ra_free_temp(&mask);

    return true;
}

/* End of TB in system-mode */
bool translate_sti(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_CLI(pir1);

    IR2_OPND *eflags = &eflags_ir2_opnd;
    IR2_OPND mask = ra_alloc_itemp();

    /* helper_sti */
    append_ir2_opnd2i(LISA_ORI, &mask, &zero_ir2_opnd, IF_BIT);
    append_ir2_opnd3 (LISA_OR,  eflags, eflags, &mask);

    ra_free_temp(&mask);

    /* sti is EOB in system-mode */
    lsenv->tr_data->inhibit_irq = 1;

    return true;
}

bool translate_in(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* dest reg : GPR */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* I/O port : GPR/IMM */

    int data_size = ir1_opnd_size(opnd0);

    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_gpr(opnd0) && (ir1_opnd_is_imm(opnd1) || ir1_opnd_is_gpr(opnd1)),
            "Unrecognized X86_INS_IN %p\n", (void*)pir1);

    /* Always check IO before any IO operation */
    tr_gen_io_check(pir1, opnd1, data_size);

    tr_gen_io_start();

    /* 0. save native context */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 1. prepare parameters for helper
     *    > arg0 : CPUX86State *env
     *    > arg1 ; uint32_t    port */

    /* 1.1 arg0: env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);

    /* 1.2 arg1 : port
     *
     * get I/O port address from opnd1
     *     > imm8 :  8-bits immediate number
     *     > dx   : 16-bits stored in dx register */
    uint32   port_imm;
    IR2_OPND port_reg;

    if (ir1_opnd_is_imm(opnd1)) {
        port_imm = (int32)(ir1_opnd_uimm(opnd1) & 0xff);
        append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd,
                &zero_ir2_opnd, port_imm);
    }
    else if (ir1_opnd_is_gpr(opnd1)) {
        port_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
        append_ir2_opnd2_(lisa_mov16z, &arg1_ir2_opnd, &port_reg);
    }

    /* 2. call the helper_inb/inw/inl */
    switch (data_size) {
        /* target/i386/misc_helper.c */
        case 8:  tr_gen_call_to_helper((ADDR)helper_inb); break;
        case 16: tr_gen_call_to_helper((ADDR)helper_inw); break;
        case 32: tr_gen_call_to_helper((ADDR)helper_inl); break;
        default:
            lsassertm(0, "Unsupported X86_INS_OUT data size %d.\n", data_size);
    }

    /* 3. restore native context */
    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 4. save $v0 to the correct destination */
    store_ir2_to_ir1_gpr(&ret0_ir2_opnd, opnd0);

    /* 5. breakpoint IO */
    tr_gen_io_bpt(pir1, opnd1, data_size >> 3);

    //tr_gen_io_end();

    return true;
}

bool translate_out(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* I/O port : GPR/IMM */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src reg  : GPR     */

    int data_size = ir1_opnd_size(opnd1);

    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_gpr(opnd1) && (ir1_opnd_is_imm(opnd0) || ir1_opnd_is_gpr(opnd0)),
             "Unrecognized X86_INS_OUT %p\n", (void*)pir1);

    /* Always check IO before any IO operation */
    tr_gen_io_check(pir1, opnd0, data_size);

    tr_gen_io_start();

    /* 0. save native context */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 1. prepare parameters for helper
     *    > arg0 : CPUX86State *env
     *    > arg1 ; uint32_t    port
     *    > arg2 : uint32_t    data */

    /* 1.0 arg2 : data
     *
     * convert function may use temp registers, so do this first
     *
     * get data from opnd0
     *    > al  :  8-bits from  al register (low)
     *    > ax  : 16-bits from  ax register
     *    > eax : 32-bits from eax regiser  */
    IR2_OPND data_reg;

    data_reg = convert_gpr_opnd(opnd1, ZERO_EXTENSION);
    append_ir2_opnd3(LISA_OR, &arg2_ir2_opnd, &data_reg, &zero_ir2_opnd);

    /* 1.1 arg0: env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);

    /* 1.2 arg1 : port
     *
     * get I/O port address from opnd1
     *     > imm8 :  8-bits immediate number
     *     > dx   : 16-bits stored in dx register */
    int32    port_imm;
    IR2_OPND port_reg;

    if (ir1_opnd_is_imm(opnd0)) {
        port_imm = (int32)(ir1_opnd_uimm(opnd0) & 0xff);
        append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd,
                &zero_ir2_opnd, port_imm);
    }
    else if (ir1_opnd_is_gpr(opnd0)) {
        port_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        append_ir2_opnd2_(lisa_mov16z, &arg1_ir2_opnd, &port_reg);
    }

    /* 2. call the helper_outb/outw/outl */
    switch (data_size) {
        /* target/i386/misc_helper.c */
        case 8:  tr_gen_call_to_helper((ADDR)helper_outb); break;
        case 16: tr_gen_call_to_helper((ADDR)helper_outw); break;
        case 32: tr_gen_call_to_helper((ADDR)helper_outl); break;
        default:
            lsassertm(0, "Unsupported X86_INS_OUT data size %d.\n", data_size);
    }
    
    /* 3. restore native context */
    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 4. breakpoint IO */
    tr_gen_io_bpt(pir1, opnd1, data_size >> 3);

    //tr_gen_io_end();

    return true;
}

void tr_gen_io_check(IR1_INST *ir1, IR1_OPND *opnd_io, int data_size)
{
    int need_check = 0;

    TRANSLATION_DATA *td = lsenv->tr_data;
    if (td->sys.pe && (td->sys.cpl > td->sys.iopl || td->sys.vm86)) {
        need_check = 1;
    }

    if (!need_check) {
        return;
    }

    /* 0. save complete context because exception might be raised */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* 1. prepare parameters for helper */

    /* 1.1 arg0: env*/
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);

    /* 1.2 arg1: IO port address */
    if (ir1_opnd_is_imm(opnd_io)) {
        /* 8-bits imm */
        uint32 port_imm = (int32)(ir1_opnd_uimm(opnd_io) & 0xff);
        append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd,
                &zero_ir2_opnd, port_imm);
    }
    else if (ir1_opnd_is_gpr(opnd_io)) {
        /* 16-bits DX register */
        int port_reg_num  = ir1_opnd_base_reg_num(opnd_io);
        int port_reg_size = ir1_opnd_size(opnd_io);
        lsassertm(port_reg_num == edx_index && port_reg_size == 16,
                "unknown GPR of IO_OPND in check io. IR1 = %p.\n", ir1);
        IR2_OPND port_reg = ra_alloc_gpr(port_reg_num);
        append_ir2_opnd2_(lisa_mov16z, &arg1_ir2_opnd, &port_reg);
    }
    else {
        lsassertm(0, "unknown type of IO_OPND in check io. IR1 = %p.\n", ir1);
    }

    /* 2. call helper from target/i386/seg_helper.c
     * helper_check_iob(env, port) size = 8
     * helper_check_iow(env, port) size = 16
     * helper_check_iol(env, port) size = 32 */
    switch (data_size) {
        /* target/i386/misc_helper.c */
        case 8:  tr_gen_call_to_helper((ADDR)helper_check_iob); break;
        case 16: tr_gen_call_to_helper((ADDR)helper_check_iow); break;
        case 32: tr_gen_call_to_helper((ADDR)helper_check_iol); break;
        default:
            lsassertm(0, "Unsupported check IO data size %d. IR1 = %p\n", data_size, ir1);
    }

    /* 3. restore native context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
}

void tr_gen_io_bpt(IR1_INST *ir1, IR1_OPND *port, int size)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (!(td->sys.flags & HF_IOBPT_MASK))
        return;

    /*  target/i386/bpt_helper.c
     *  void helper_bpt_io(
     *      CPUX86State *env,
     *      uint32_t port,
     *      uint32_t size, 1:8-bit 2:16-bit 4:32-bit 8:64-bit
     *      target_ulong next_eip) */

    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 1.1 arg0: env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);

    /* 1.2 arg1 : port
     * get I/O port address from opnd1
     *     > imm8 :  8-bits immediate number
     *     > dx   : 16-bits stored in dx register */
    if (ir1_opnd_is_imm(port)) {
        uint32_t port_imm = (uint32_t)(ir1_opnd_uimm(port) & 0xff);
        append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd,
                &zero_ir2_opnd, port_imm);
    }
    else if (ir1_opnd_is_gpr(port)) {
        IR2_OPND port_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(port));
        append_ir2_opnd2_(lisa_mov16z, &arg1_ir2_opnd, &port_reg);
    }

    /* 1.3 arg2: size */
    append_ir2_opnd2i(LISA_ORI, &arg2_ir2_opnd,
            &zero_ir2_opnd, size);

    /* 1.4 next eip = IR1.address + IR1.size */
    ADDRX next_eip = ir1_addr_next(ir1);
    load_addrx_to_ir2(&arg3_ir2_opnd, next_eip);

    /* 1.5 call helper */
    tr_gen_call_to_helper((ADDR)helper_bpt_io);

    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

void tr_gen_tb_start(void) 
{
#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
    xtm_pf_step_gen_tb_start();
#endif

    if (xtm_sigint_opt()) return;
//    lsassertm(0, "tb start to be implemented in LoongArch.\n");

    TRANSLATION_DATA *td = lsenv->tr_data;
//    TranslationBlock *tb = td->curr_tb;
//    uint32 cflags = atomic_read(&tb->cflags);
    IR2_OPND count = ra_alloc_itemp();

    append_ir2_opnd2i(LISA_LD_W, &count, &env_ir2_opnd,
        (int32)offsetof(X86CPU, neg.icount_decr.u32) -
        (int32)offsetof(X86CPU, env));

//    int ir1_nr = td->ir1_nr;
//    if (cflags & CF_USE_ICOUNT) {
//        /* Record the instruction that decrease icount.
//         * If exception occurs during translation, the
//         * number of IR1 to decrease might be less. */
//        IR2_INST *pir2 = append_ir2_opnd2i(mips_addiu,
//                &count, &count, 0 - ir1_nr);
//        td->dec_icount_inst_id = ir2_get_id(pir2);
//    }

    append_ir2_opnd3(LISA_BLT, &count, &zero_ir2_opnd,
                     &(td->exitreq_label));

//    if (cflags & CF_USE_ICOUNT) {
//        append_ir2_opnd2i(LISA_ST_H, &count, &env_ir2_opnd,
//            (int32)offsetof(X86CPU, neg.icount_decr.u16.low) -
//            (int32)offsetof(X86CPU, env));
//        tr_gen_io_end();
//    }

    ra_free_temp(&count);
}

void tr_gen_tb_end(void) 
{
    if (xtm_sigint_opt()) return;
//    lsassertm(0, "tb end to be implemented in LoongArch.\n");

    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    append_ir2_opnd1(LISA_LABEL, &td->exitreq_label);

//    IR2_OPND mips_ret_opnd = ir2_opnd_new(IR2_OPND_IREG, 2); /* v0 */
    IR2_OPND tb_ptr_opnd = ra_alloc_dbt_arg1();
    IR2_OPND eip_opnd = ra_alloc_dbt_arg2();

    ADDR tb_addr = (ADDR)tb;
//#ifndef N64
//    append_ir2_opnd1i(mips_lui, &tb_ptr_opnd, (int16_t)(tb_addr >> 16));
//    append_ir2_opnd1i(mips_ori, &tb_ptr_opnd, tb_ptr_opnd, (int16_t)tb_addr);
//#else
    tr_gen_exit_tb_load_tb_addr(&tb_ptr_opnd, tb_addr);
//#endif

    int eip = tb->pc - tb->cs_base;
    load_imm32_to_ir2(&eip_opnd, eip, ZERO_EXTENSION);

//#ifdef CONFIG_XTM_FAST_CS
//    uint8_t fast_cs_mask = tb->extra_tb->fast_cs_mask;
//    if (!(fast_cs_mask & XTM_FAST_CS_MASK_FPU)) {
//        goto _GEN_TB_END_;
//    }
//    /* With FastCS, if current TB does not use FPU.
//     * the env->fpstt should contains the correct value already. */
//#endif

    if (!option_lsfpu) {
        /* FPU top shoud be TB's top_in, not top_out. */
        IR2_OPND top_in = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ORI, &top_in, &zero_ir2_opnd,
                etb_get_top_in(tb->extra_tb) & 0x7);
        append_ir2_opnd2i(LISA_ST_W, &top_in, &env_ir2_opnd,
                lsenv_offset_of_top(lsenv));
        ra_free_temp(&top_in);
    }

//#ifdef CONFIG_XTM_FAST_CS
//_GEN_TB_END_:
//#endif

    append_ir2_opnd2i(LISA_ORI, &ret0_ir2_opnd, &tb_ptr_opnd,
                      TB_EXIT_REQUESTED);

    void *code_buf = tb->tc.ptr;
    int offset = td->real_ir2_inst_num << 2;
    append_ir2_opnda(LISA_B, (context_switch_native_to_bt
                              - (ADDR)code_buf - offset) >> 2);
}

void tr_gen_io_start(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;
    uint32 cflags = atomic_read(&tb->cflags);
    if (cflags & CF_USE_ICOUNT) {
        lsassertm(0, "[ICOUNT mode] IO start to be implemented in LoongArch.\n");
//        IR2_OPND tmp = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_ori, &tmp, &zero_ir2_opnd, 1);
//        append_ir2_opnd2i(mips_sw, &tmp, &env_ir2_opnd,
//                (int32)offsetof(X86CPU, parent_obj.can_do_io) - 
//                (int32)offsetof(X86CPU, env));
//        ra_free_temp(&tmp);
    }
}

void tr_gen_io_end(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;
    uint32 cflags = atomic_read(&tb->cflags);
    if (cflags & CF_USE_ICOUNT) {
        lsassertm(0, "[ICOUNT mode] IO end to be implemented in LoongArch.\n");
//        append_ir2_opnd2i(mips_sw, &zero_ir2_opnd, &env_ir2_opnd,
//                (int32)offsetof(X86CPU, parent_obj.can_do_io) - 
//                (int32)offsetof(X86CPU, env));
    }
}

/* push/pop
 *
 * In system-mode, for precise exception, the effect of instruction
 * should be executed at the end of this instruction's translated code.
 * 
 * For pop, these two things need to be done at the end:
 *  1> save the read value into destination
 *  2> update esp register
 *
 * For push, since the write is done by softmmu, and the exception can
 * only be generated by the softmmu helper, only one thing need to be
 * done at the end:
 *  1> update esp register */

/* End of TB in system-mode
 * > pop es/ss/ds */
bool translate_pop(IR1_INST *pir1)
{
    /* pop
     * ----------------------
     * >  dest <= MEM(SS:ESP)
     * >  ESP  <= ESP + 2/4
     * ----------------------
     * 1. tmp  <= MEM(SS:ESP) : softmmu
     * 2. ESP  <= ESP + 2/4
     * 3. tmp  => dest : gpr/mem/seg */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

//#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
//    if (ir1_opnd_is_seg(opnd0) &&
//        option_monitor_tbf &&
//        ir1_opnd_base_reg_num(opnd0) == ss_index) {
//        /* mark is pop operation, used in helper */
//        IR2_OPND tmp = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_ori, &tmp, &zero_ir2_opnd, 0x1);
//        append_ir2_opnd2i(mips_sb, &tmp, &env_ir2_opnd,
//                lsenv_offset_of_pf_data_tbf_is_pop(lsenv));
//        ra_free_temp(&tmp);
//    }
//#endif

    TRANSLATION_DATA *td = lsenv->tr_data;

    bool is_gpr_esp = 0;
    bool is_mem_esp = 0;
    bool ss32 = lsenv->tr_data->sys.ss32;

    if (ir1_opnd_is_gpr(opnd0)) {
        if (ir1_opnd_base_reg_num(opnd0) == esp_index) {
            is_gpr_esp = 1;
        }
    }

    if (ir1_opnd_is_mem(opnd0)) {
        if (ir1_opnd_base_reg_num(opnd0) == esp_index) {
            is_mem_esp = 1;
        }
    }

    int data_size = ir1_data_size(pir1);
    int esp_inc   = data_size >> 3;

    int opnd_size = ir1_opnd_size(opnd0);
    lsassert(opnd_size == 16 || opnd_size == 32);
    if (!ir1_opnd_is_seg(opnd0)) {
        lsassert(data_size == opnd_size);
    }

    lsassert(data_size >= opnd_size);

//    if (option_by_hand && lsenv->tr_data->sys.ss32 &&
//        !lsenv->tr_data->sys.addseg && ir1_opnd_is_gpr(opnd0) &&
//        ir1_opnd_num(pir1) == 1 && !is_gpr_esp && data_size == 32) {
//        IR2_OPND esp_mem_opnd;
//        IR2_OPND esp = ra_alloc_gpr(esp_index);
//        ir2_opnd_build2(&esp_mem_opnd, IR2_OPND_MEM, esp._reg_num, 0);
//        IR2_OPND dest_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
//        gen_ldst_softmmu_helper(
//            ir1_opnd_default_em(opnd0) == SIGN_EXTENSION ? mips_lw : mips_lwu,
//            &dest_reg, &esp_mem_opnd, 0);
//        IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
//        append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd, esp_inc);
//        return true;
//    }

    /* 1.1 build MEM(SS:ESP) */
    IR1_OPND mem_ir1_opnd;
    ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0, 0, 0);

    /* 1.2 read data from stack   : might generate exception */
    IR2_OPND tmp = ra_alloc_itemp();
    int ss_addr_size = get_sys_stack_addr_size();
    load_ir1_mem_to_ir2(&tmp, &mem_ir1_opnd, ZERO_EXTENSION, false, ss_addr_size);
    
    /* 2. update ESP */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    if (is_gpr_esp) {
        if(ss32) {
            append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, esp_inc);
        } else {
            IR2_OPND tmp = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, esp_inc);
            store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
            ra_free_temp(&tmp);
        }
    }

    /* 3. write into destination  : might generate exception */
    if (is_mem_esp) {
        td->sys.popl_esp_hack = data_size >> 3;
        store_ir2_to_ir1_mem(&tmp, opnd0, false, ss_addr_size);
        td->sys.popl_esp_hack = 0;
    } else {
        store_ir2_to_ir1(&tmp, opnd0, false);
    }

    /* Order is important for pop %esp
     * The %esp is increased first. Then the pop value is loaded into %esp.
     * This executing order is the same as that in TCG. */
    if (!is_gpr_esp) {
        if(ss32) {
            append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, esp_inc);
        } else {
            IR2_OPND tmp = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, esp_inc);
            store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
            ra_free_temp(&tmp);
        }
    }

//#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
//    /* clear this mark which is used in helpr */
//    if (option_monitor_tbf) {
//        append_ir2_opnd2i(mips_sb, &zero_ir2_opnd, &env_ir2_opnd,
//                lsenv_offset_of_pf_data_tbf_is_pop(lsenv));
//    }
//#endif

    return true;
}

bool translate_push(IR1_INST *pir1)
{
    /* push
     * ----------------------
     * >  ESP  <= ESP - 2/4
     * >  src  => MEM(SS:ESP)
     * ----------------------
     * 1. tmp  <= source : gpr/mem/seg
     * 2. tmp  => MEM(SS:ESP - 2/4) : softmmu
     * 3. ESP  <= ESP - 2/4           */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    bool ss32 = lsenv->tr_data->sys.ss32;

    int data_size = ir1_data_size(pir1);
    int esp_dec   = 0 - (data_size >> 3);

    int opnd_size = ir1_opnd_size(opnd0);
    lsassert(opnd_size == 16 || opnd_size == 32);
    if (!ir1_opnd_is_seg(opnd0)) {
        lsassert(data_size == opnd_size);
    }


//    if (option_by_hand && lsenv->tr_data->sys.ss32 &&
//        !lsenv->tr_data->sys.addseg && ir1_opnd_is_gpr(opnd0) &&
//        ir1_opnd_num(pir1) == 1 && data_size == 32) {
//        IR2_OPND esp_mem_opnd;
//        IR2_OPND esp = ra_alloc_gpr(esp_index);
//        ir2_opnd_build2(&esp_mem_opnd, IR2_OPND_MEM, esp._reg_num, esp_dec);
//        IR2_OPND src_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
//        gen_ldst_softmmu_helper(mips_sw, &src_reg, &esp_mem_opnd, 0);
//        IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
//        append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd, esp_dec);
//        return true;
//    }


    /* 1. load source data   : might generate exception */
    IR2_OPND tmp = ra_alloc_itemp();
    load_ir1_to_ir2(&tmp, opnd0, UNKNOWN_EXTENSION, false);

    /* 2.1 build MEM(SS:ESP - 2/4)*/
    IR1_OPND mem_ir1_opnd;
    ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, esp_dec, 0, 0);

    /* 2.2 write data into stack   : might generate exception */
    int ss_addr_size = get_sys_stack_addr_size();
    store_ir2_to_ir1_mem(&tmp, &mem_ir1_opnd, false, ss_addr_size);

    /* 3. update ESP */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    if (ss32) {
        append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, esp_dec);
    } else {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, esp_dec);
        store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        ra_free_temp(&tmp);
    }

    return true;
}

/* End of TB in system-mode */
bool translate_rsm(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_RSM(pir1);

    /* 1. save next instruciton's EIP to env */
    tr_gen_save_next_eip();

    /* 2. helper_rsm
     *
     * target/i386/smm_helper.c
     * void helper_rsm(
     *      CPUX86State *env) */
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_rsm, default_helper_cfg);

    /* the eip is already updated in helper_rsm */
    lsenv->tr_data->ignore_eip_update = 1;

    return true;
}

bool translate_verr(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_VERR(pir1);

    /* 1. load segment selector from opnd0 */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR2_OPND seg = ra_alloc_itemp();
    load_ir1_to_ir2(&seg, opnd0, ZERO_EXTENSION, false);

    /* 2. call helper function */
    /* 2.1 save context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
    /* target/i386/seg_helper.c
     *
     * void helper_verr(
     *      CPUX86State *env,
     *      target_ulong selector1) */

    /* 2.2 arg1 : selector */
    append_ir2_opnd3(LISA_OR, &arg1_ir2_opnd, &seg, &zero_ir2_opnd);
    /* 2.3 arg0 : env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    /* 2.4 call helper */
    tr_gen_call_to_helper((ADDR)helper_verr);
    /* 2.5 resotre context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

    return true;
}

bool translate_verw(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_VERW(pir1);

    /* 1. load segment selector from opnd0 */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR2_OPND seg = ra_alloc_itemp();
    load_ir1_to_ir2(&seg, opnd0, ZERO_EXTENSION, false);

    /* 2. call helper function */
    /* 2.1 save context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
    /* target/i386/seg_helper.c
     *
     * void helper_verw(
     *      CPUX86State *env,
     *      target_ulong selector1) */

    /* 2.2 arg1 : selector */
    append_ir2_opnd3(LISA_OR, &arg1_ir2_opnd, &seg, &zero_ir2_opnd);
    /* 2.3 arg0 : env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    /* 2.4 call helper */
    tr_gen_call_to_helper((ADDR)helper_verw);
    /* 2.5 resotre context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

    return true;
}

bool translate_invd(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_INVD(pir1);
    /* nothing to do */
    return true;
}

bool translate_wbinvd(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_WBINVD(pir1);
    /* nothing to do */
    return true;
}

/* End of TB in system-mode
 * > reps nop */
bool translate_nop(IR1_INST *pir1)
{
    if (ir1_is_repz_nop(pir1)) {

        /* 1. save full context */
        helper_cfg_t cfg = default_helper_cfg;
        tr_sys_gen_call_to_helper_prologue_cfg(cfg);

        /* 2. call helper_pause
         *
         * target/i386/misc_helper.c
         * void helper_pause(
         *      CPUX86State *env,
         *      int next_eip_addend) */

        /* 2.1 arg0: env */
        append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
        /* 2.2 arg1: this inst's size, 12-bit is enough */
        int size = ir1_inst_size(pir1);
        append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd,
                &zero_ir2_opnd, size & 0xfff);
        /* 2.3 call helper pause */
        tr_gen_call_to_helper((ADDR)helper_pause);

        /* 3. This helper never return */
        tr_gen_infinite_loop();

    } else {
        /* nothing to do */
    }

    return true;
}

/* End of TB in system-mode */
bool translate_pause(IR1_INST *pir1)
{
    /* 0. save current instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. save full context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* 2. call helper_pause
     *
     * target/i386/misc_helper.c
     * void helper_pause(
     *      CPUX86State *env,
     *      int next_eip_addend) */

    /* 2.1 arg0: env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    /* 2.2 arg1: this inst's size, 16-bit is enough */
    int size = ir1_inst_size(pir1);
    append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd,
            &zero_ir2_opnd, size & 0xfff);
    /* 2.3 call helper pause */
    tr_gen_call_to_helper((ADDR)helper_pause);

    /* 3. This helper never return */
    tr_gen_infinite_loop();

    return true;
}

static bool do_translate_iret(IR1_INST *pir1, int size)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_IRET(pir1);

    /* only supported 16-bit and 32-bit now */
    lsassert(size == 0 || size == 1);

    /* 1. save complete context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* 2. protected mode iret
     *
     * target/i386/seg_helper.c
     * void helper_iret_protected(
     *      CPUX86State *env,
     *      int shift,
     *      int next_eip)       */
    if (td->sys.pe && !td->sys.vm86) {
        ADDRX next_eip = ir1_addr_next(pir1);
        /* 2.1 arg2: next eip */
        load_addrx_to_ir2(&arg2_ir2_opnd, next_eip);
        /* 2.2 arg1: size */
        append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd,
                &zero_ir2_opnd, size);
        /* 2.3 arg0: env */
        append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
        /* 2.4 call helper_iret_protected : might generate exception  */
        tr_gen_call_to_helper((ADDR)helper_iret_protected);
    }
    /* 2. real and vm86 mode iret
     *
     * target/i386/seg_helper.c
     * void helper_iret_real(
     *      CPUX86State *env,
     *      int shift)          */
    else{
        /* 2.1 arg0: env */
        append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
        /* 2.2 arg1: size */
        append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd,
                &zero_ir2_opnd, size);
        /* 2.3 call helper_iret_real : ESP is updated, so we must save all gpr */
        tr_gen_call_to_helper((ADDR)helper_iret_real);
    }

    /* 3. restore native context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

    /* 5. disable eip update, since the helper modify eip */
    td->ignore_eip_update = 1;

    return true;
}

/* End of TB in system-mode */
bool translate_iret(IR1_INST *pir1)
{
    /* 16-bit opnd size */
    return do_translate_iret(pir1, 0);
}

/* End of TB in system-mode */
bool translate_iretd(IR1_INST *pir1)
{
    /* 32-bit opnd size */
    return do_translate_iret(pir1, 1);
}

bool translate_int(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_INT(pir1);

    /* 0. save current instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. save complete context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* 2. raise interrupt
     *
     *  target/i386/excp_helper.c
     *  void helper_raise_interrupt(
     *      CPUX86State *env,
     *      int intno,
     *      int next_eip_addend) */

    /* 2.1 arg1: intno */
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    int intno = ir1_opnd_simm(opnd);
    load_imm32_to_ir2(&arg1_ir2_opnd, intno, ZERO_EXTENSION);
    /* 2.2 arg2 : next eip addend : instruction size */
    int size = ir1_inst_size(pir1);
    append_ir2_opnd2i(LISA_ORI, &arg2_ir2_opnd,
            &zero_ir2_opnd, size & 0xfff);
    /* 2.3 arg0 : env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    /* 2.4 call helper_raise_interrupt: generate interrupt */
    tr_gen_call_to_helper((ADDR)helper_raise_interrupt);

    /* 3. This helper never return */
    tr_gen_infinite_loop();

    return true;
}

bool translate_int_3(IR1_INST *pir1)
{
    /* 0. save current instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. save complete context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* 2. raise interrupt
     *
     *  target/i386/excp_helper.c
     *  void helper_raise_interrupt(
     *      CPUX86State *env,
     *      int intno,
     *      int next_eip_addend) */

    /* 2.1 arg1: intno = EXCP03_INT3 */
    append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd,
            &zero_ir2_opnd, EXCP03_INT3);
    /* 2.2 arg2: next eip addend */
    int size = ir1_inst_size(pir1);
    append_ir2_opnd2i(LISA_ORI, &arg2_ir2_opnd,
            &zero_ir2_opnd, size & 0xfff);
    /* 2.3 arg0 : env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    /* 2.4 call helper_raise_interrupt: generate interrupt */
    tr_gen_call_to_helper((ADDR)helper_raise_interrupt);

    /* 3. This helper never return */
    tr_gen_infinite_loop();

    return true;
}

bool translate_into(IR1_INST *pir1)
{
    /* 0. save current instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. save complete context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* 2. helper_into
     *
     *  target/i386/misc_helper.c
     *  void helper_into(
     *      CPUX86State *env,
     *      int next_eip_addend) */

    /* 2.1 arg2: next eip addend */
    int size = ir1_inst_size(pir1);
    append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd,
            &zero_ir2_opnd, size & 0xfff);
    /* 2.2 arg0 : env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    /* 2.3 call helper_raise_interrupt: generate interrupt */
    tr_gen_call_to_helper((ADDR)helper_into);

    /* 3. restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
 
    return true;
}

bool translate_int1(IR1_INST *pir1)
{
    /* 0. save current instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. helper_debug
     *
     * target/i386/misc_helper.c
     * void helper_debug(
     *      CPUX86State *env) */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_debug, cfg);

    /* 2. helper debug never return */
    tr_gen_infinite_loop();

    return true;
}

static void translate_lmsw_gpr(IR1_INST *pir1, IR1_OPND *opnd0)
{
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_lmsw(CPUX86State  *env, target_ulong t0) */

    /* 1. get machine status word */
    IR2_OPND temp_opnd = ra_alloc_itemp();
    load_ir1_gpr_to_ir2(&temp_opnd, opnd0, ZERO_EXTENSION);
    append_ir2_opnd3(LISA_OR, &arg1_ir2_opnd, &temp_opnd, &zero_ir2_opnd);

    /* 2. call helper */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);

    tr_gen_call_to_helper((ADDR)helper_lmsw);

    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

static void translate_lmsw_mem(IR1_INST *pir1, IR1_OPND *opnd0)
{
#ifdef CONFIG_BTMMU
    tr_disable_btmmu();
#endif

    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_lmsw(CPUX86State  *env, target_ulong t0) */

    /* 1. get machine status word */
    IR2_OPND data = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&data, opnd0, ZERO_EXTENSION, false, -1);
    append_ir2_opnd3(LISA_OR, &arg1_ir2_opnd, &data, &zero_ir2_opnd);

    /* 2. call helper */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);

    tr_gen_call_to_helper((ADDR)helper_lmsw);

    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

#ifdef CONFIG_BTMMU
    tr_reset_btmmu();
#endif
}

/* End of TB in system-mode */
bool translate_lmsw(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LMSW(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    if (ir1_opnd_is_gpr(opnd0)) {
        translate_lmsw_gpr(pir1, opnd0);
    } else {
        lsassertm_illop(ir1_addr(pir1),
                ir1_opnd_is_mem(opnd0),
                "not a valid LMSW insn: pir1 = %p\n", (void*)pir1);

        translate_lmsw_mem(pir1, opnd0);
    }

    return true;
}

bool translate_smsw(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    /* load status word (cr0) from env */
    IR2_OPND status_word = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_HU, &status_word, &env_ir2_opnd,
                      lsenv_offset_of_cr(lsenv, 0));

    store_ir2_to_ir1(&status_word, opnd0, false);

    return true;
}

static void do_translate_ret_far_pe(IR1_INST *pir1, int size, int esp_inc)
{
    /* 0. save current instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. save complete context */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 2. prepare arguments */

    /* arg0 : env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    /* arg1 : shift = opnd size (32-bit:1 ; 16-bit:0)*/
    append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd,
            &zero_ir2_opnd, size);
    /* arg2 : addend to esp: esp inc */
    append_ir2_opnd2i(LISA_ORI, &arg2_ir2_opnd,
            &zero_ir2_opnd, esp_inc);

    /* 3. call helper_lret_protected
     *    > might genrate exception
     *    > eflags is updated in helper
     *    > EIP is updated in helper */
    tr_gen_call_to_helper((ADDR)helper_lret_protected);
    /* > protected mode
     *
     * target/i386/seg_helper.c
     * void helper_lret_protected(
     *      CPUX86State *env,
     *      int shift,
     *      int addend)   */

    /* 4. restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 5. disable eip update in context switch */
    lsenv->tr_data->ignore_eip_update = 1;
}

static void do_translate_ret_far_real(IR1_INST *pir1, int opnd_size, int esp_inc)
{
    int ss_addr_size = get_sys_stack_addr_size();
    /* 1. build MEM(SS:ESP) */
    IR1_OPND mem_ir1_opnd;
    ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size,
            X86_REG_SS, X86_REG_ESP, 0, 0, 0);

    /* 2. load new EIP from MEM(SS:ESP) */
    IR2_OPND tmp_new_eip = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&tmp_new_eip, &mem_ir1_opnd, ZERO_EXTENSION, false, ss_addr_size);

    /* 3. load new CS from MEM(SS:ESP + 2/4) */
    ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size,
            X86_REG_SS, X86_REG_ESP, (opnd_size >> 3), 0, 0);
    IR2_OPND tmp_new_cs = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&tmp_new_cs, &mem_ir1_opnd, ZERO_EXTENSION, false, ss_addr_size);

    /* 4. update env->eip */
    append_ir2_opnd2i(LISA_ST_W, &tmp_new_eip, &env_ir2_opnd,
                      lsenv_offset_of_eip(lsenv));
    /* 5. update env->cs.base, env->cs.selector */
    append_ir2_opnd2i(LISA_ST_W, &tmp_new_cs, &env_ir2_opnd,
                      lsenv_offset_of_seg_selector(lsenv, cs_index));
    append_ir2_opnd2i(LISA_SLLI_D, &tmp_new_cs, &tmp_new_cs, 0x4);
    append_ir2_opnd2i(LISA_ST_W, &tmp_new_cs, &env_ir2_opnd,
                      lsenv_offset_of_seg_base(lsenv, cs_index));

    /* 6. update esp */
    esp_inc += opnd_size >> 2; /* (opnd_size >> 3) << 1 */

    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    if (lsenv->tr_data->sys.ss32) {
        append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, esp_inc);
    }
    else {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, esp_inc);
        store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        //avoid em_check in tr_fini failed
//        append_ir2_opnd2(mips_mov32_zx, &esp_opnd, &esp_opnd);
        ra_free_temp(&tmp);
    }

    /* 7. disable eip update in context switch
     *    Since we already update eip here */
    lsenv->tr_data->ignore_eip_update = 1;
}

static bool do_translate_ret_far(IR1_INST *pir1, int size, int opnd_size)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    int esp_inc = 0;
    if (ir1_opnd_num(pir1)) {
        IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
        esp_inc = ir1_opnd_simm(opnd);
    }

    if (td->sys.pe && !td->sys.vm86) {
        do_translate_ret_far_pe(pir1, size, esp_inc);
    }
    /* > real mode or vm86 */
    else {
        do_translate_ret_far_real(pir1, opnd_size, esp_inc);
    }

    return true;
}

/* End of TB in system-mode */
bool translate_retf(IR1_INST *pir1)
{
    int data_size = ir1_data_size(pir1);
    if (data_size == 16) {
        /* 16-bit opnd size */
        return do_translate_ret_far(pir1, 0, 16);
    } else {
        /* 32-bit opnd size */
        return do_translate_ret_far(pir1, 1, 32);
    }
}

bool translate_call(IR1_INST *pir1)
{
    if(ir1_is_indirect_call(pir1)){
        return translate_callin(pir1);
    }
    else if (ir1_addr_next(pir1) == ir1_target_addr(pir1)) {
        return translate_callnext(pir1);
    }

    bool ss32 = lsenv->tr_data->sys.ss32;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int data_size = ir1_data_size(pir1);
    int opnd_size = ir1_opnd_size(opnd0);
    lsassert(opnd_size == 16 || opnd_size == 32);
    lsassert(data_size == opnd_size);
    int ss_addr_size = get_sys_stack_addr_size();

    /* 1. get return address */
    IR2_OPND return_addr_opnd = ra_alloc_itemp();
    load_addrx_to_ir2(&return_addr_opnd, ir1_addr_next(pir1));

    /* 2. save return address at MEM(SS:ESP - 2/4)*/
    IR1_OPND mem_ir1_opnd;
    ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size,
            X86_REG_SS, X86_REG_ESP, 0 - (opnd_size >> 3), 0, 0);
    store_ir2_to_ir1_mem(&return_addr_opnd, &mem_ir1_opnd, false, ss_addr_size);
    ra_free_temp(&return_addr_opnd);

    /* 3. update ESP */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    if (ss32) {
        append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd,
                          0 - (opnd_size >> 3));
    } else {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd,
                          0 - (opnd_size >> 3));
        store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        //avoid em_check in tr_fini failed
//        append_ir2_opnd2(mips_mov32_zx, &esp_opnd, &esp_opnd);
        ra_free_temp(&tmp);
    }

    /* 4. shadow stack : not ready for system mode
    if(option_shadow_stack)
        ss_gen_push(pir1); */

    /* 5. go to next TB */
    tr_generate_exit_tb(pir1, 0);

    return true;
}

bool translate_callnext(IR1_INST *pir1)
{
    bool ss32 = lsenv->tr_data->sys.ss32;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int data_size = ir1_data_size(pir1);
    int opnd_size = ir1_opnd_size(opnd0);
    lsassert(opnd_size == 16 || opnd_size == 32);
    lsassert(data_size == opnd_size);
    int ss_addr_size = get_sys_stack_addr_size();

    /* 1. get return address */
    IR2_OPND next_addr_opnd = ra_alloc_itemp();
    load_addrx_to_ir2(&next_addr_opnd, ir1_addr_next(pir1));

    /* 2. save return address at MEM(SS:ESP - 2/4)*/
    IR1_OPND mem_ir1_opnd;
    ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size,
            X86_REG_SS, X86_REG_ESP, 0 - (opnd_size >> 3), 0, 0);
    store_ir2_to_ir1_mem(&next_addr_opnd, &mem_ir1_opnd, false, ss_addr_size);
    ra_free_temp(&next_addr_opnd);

    /* 3. update ESP */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    if (ss32) {
        append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd,
                          0 - (opnd_size >> 3));
    } else {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd,
                          0 - (opnd_size >> 3));
        store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        //avoid em_check in tr_fini failed
//        append_ir2_opnd2(mips_mov32_zx, &esp_opnd, &esp_opnd);
        ra_free_temp(&tmp);
    }

    return true;
}

bool translate_callin(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    bool ss32 = td->sys.ss32;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int data_size = ir1_data_size(pir1);
    int opnd_size = ir1_opnd_size(opnd0);
    lsassert(opnd_size == 16 || opnd_size == 32);
    lsassert(data_size == opnd_size);
    int ss_addr_size = get_sys_stack_addr_size();

    /* 1. prepare successor x86 address */
    IR2_OPND next_eip_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&next_eip_opnd, opnd0, ZERO_EXTENSION, false);
    if (opnd_size == 16) {
    lsassertm(0, "call in 16-bit GPR to be implemented in LoongArch.\n");
//        append_ir2_opnd2i(mips_andi, &next_eip_opnd,
//                &next_eip_opnd, 0xffff);
    }

    /* 2. get return address */
    IR2_OPND return_addr_opnd = ra_alloc_itemp();
    load_addrx_to_ir2(&return_addr_opnd, ir1_addr_next(pir1));

    /* 3. save return address at MEM(SS:ESP - 2/4)*/
    IR1_OPND mem_ir1_opnd;
    ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size, /* 16 or 32 */
            X86_REG_SS, X86_REG_ESP, 0 - (opnd_size >> 3), 0, 0);
    store_ir2_to_ir1_mem(&return_addr_opnd, &mem_ir1_opnd, false, ss_addr_size);
    ra_free_temp(&return_addr_opnd);

    /* 4. update ESP */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    if (ss32) {
        append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd,
                          0 - (opnd_size >> 3));
    } else {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd,
                          0 - (opnd_size >> 3));
        store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        ra_free_temp(&tmp);
    }

    /* 5. shadow statck : not ready for system-mode
    if(option_shadow_stack) 
         ss_gen_push(pir1);  */

    /* 6. adjust em to defaul em : why? */
//    tr_adjust_em();

    /* 7. go to next TB */
    IR2_OPND succ_x86_addr_opnd = ra_alloc_dbt_arg2(); /* t9($25) */
    append_ir2_opnd3(LISA_OR, &succ_x86_addr_opnd,
            &zero_ir2_opnd, &next_eip_opnd);

    tr_generate_exit_tb(pir1, 0);

    return true;
}

static void translate_call_far_imm(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    /* 1. save native context */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 2. prepare the values for parameters */

    /* 2.1 prepare parameter new_cs and new_eip */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    /* -------------------------------------------
     *  opndtype |      opnd[0]    |     opnd[1]
     * -------------------------------------------
     *  ptr16:16 |  imm16 selector |  imm16 offset
     *  ptr16:32 |  imm16 selector |  imm32 offset
     * ------------------------------------------- */
    int data_size = ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);
    lsassert(ir1_opnd_size(opnd0) == 16);
    lsassert(ir1_opnd_size(opnd1) == data_size);

//    uint32_t selector = ir1_opnd_uimm(opnd0); /* new_cs  : 16-bit    */
    uint32_t offset   = ir1_opnd_uimm(opnd1); /* new_eip : 16/32-bit */

    /* 2.2 prepare parameter next_eip */
    ADDRX next_eip = ir1_addr_next(pir1);

    /* 2.3 prepare shift: data size
     *     16-bit: shift = 0
     *     32-bit: shift = 1   */
    int shift = (data_size >> 4) - 1;

    /* 3. building the parameters: take care of temp register */
    /* 3.1 arg0 : env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    /* 3.2 arg1 : selector
     *     arg2 : offset   */
    lsassertm(0, "call far to be implemented in LoongArch.\n");
//    append_ir2_opnd2i(mips_ori, &arg1_ir2_opnd,
//            &zero_ir2_opnd, selector & 0xffff);
    if (data_size == 32) {
        load_imm32_to_ir2(&arg2_ir2_opnd, offset, ZERO_EXTENSION);
    } else {
//        append_ir2_opnd2i(mips_ori, &arg2_ir2_opnd,
//                &zero_ir2_opnd, offset & 0xffff);
    }
    /* 3.3 arg3 : shift according to data size */
    append_ir2_opnd2i(LISA_ORI, &arg3_ir2_opnd, &zero_ir2_opnd, shift);
    /* 3.4 arg4 : next eip */
    load_imm32_to_ir2(&arg4_ir2_opnd, (uint32_t)next_eip, ZERO_EXTENSION);
 
    /* 4. call the helper */
    if (td->sys.pe && !td->sys.vm86) {
        tr_gen_call_to_helper((ADDR)helper_lcall_protected);
    } else {
        tr_gen_call_to_helper((ADDR)helper_lcall_real);
    }
    /* 3. far call in protected mode  * far call in real-mode or vm86 mode *
     *  > complete cpustate is used   *  > SS and ESP (mapping) is used    *
     *  > CS and EIP is updated       *  > CS and EIP is updated           *
     *  > might generate exception    *  > might generate exception        *
     *                                *                                    *
     * target/i386/seg_helper.c       * target/i386/seg_helper.c           *
     * void helper_lcall_protected(   * void helper_lcall_real(            *
     *      CPUX86State     *env,     *      CPUX86State     *env,         *
     *      int             new_cs,   *      int             new_cs,       *
     *      target_ulong    new_eip,  *      target_ulong    new_eip1,     *
     *      int             shift,    *      int             shift,        *
     *      target_ulong    next_eip) *      int             next_eip)     */

    /* 5. restore native context */
    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 6. disable EIP update in the later jmp */
    td->ignore_eip_update = 1;
}

static void translate_call_far_mem(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    /* 1. prepare the values for parameters */

    /* 1.1 prepare parameter new_cs and new_eip */
    IR2_OPND selector_opnd = ra_alloc_itemp();
    IR2_OPND offset_opnd   = ra_alloc_itemp();
    /* -------------------------------------------
     *  opndtype |      opnd[0]    |     opnd[1]
     * -------------------------------------------
     *    m16:16 |   mem  size = 4 |       --
     *    m16:32 |   mem  size = 6 |       --      
     * ------------------------------------------- */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd0, -1);

    /* offset at MEM(address) : size = data_size */
    int data_size = ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);
    if (data_size == 32) {
        gen_ldst_softmmu_helper(LISA_LD_WU, &offset_opnd, &mem_opnd, 1);
    } else {
        gen_ldst_softmmu_helper(LISA_LD_HU, &offset_opnd, &mem_opnd, 1);
    }

    /* selector at MEM(address + 2 or 4 ) : size = 16-bits */
    if (data_size == 32) {
        IR2_OPND mem_opnd_adjusted = convert_mem_ir2_opnd_plus_4(&mem_opnd);
        gen_ldst_softmmu_helper(LISA_LD_HU, &selector_opnd, &mem_opnd_adjusted, 1);
    } else {
        IR2_OPND mem_opnd_adjusted = convert_mem_ir2_opnd_plus_2(&mem_opnd);
        gen_ldst_softmmu_helper(LISA_LD_HU, &selector_opnd, &mem_opnd_adjusted, 1);
    }

    /* 1.2 prepare parameter next_eip */
    ADDRX next_eip = ir1_addr_next(pir1);

    /* 1.3 prepare shift: data size
     *     16-bit: shift = 0
     *     32-bit: shift = 1   */
    int shift = (data_size >> 4) - 1;

    /* 2. save native context */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 3. far call in protected mode  * far call in real-mode or vm86 mode *
     *  > complete cpustate is used   *  > SS and ESP (mapping) is used    *
     *  > CS and EIP is updated       *  > CS and EIP is updated           *
     *  > might generate exception    *  > might generate exception        *
     *                                *                                    *
     * target/i386/seg_helper.c       * target/i386/seg_helper.c           *
     * void helper_lcall_protected(   * void helper_lcall_real(            *
     *      CPUX86State     *env,     *      CPUX86State     *env,         *
     *      int             new_cs,   *      int             new_cs,       *
     *      target_ulong    new_eip,  *      target_ulong    new_eip1,     *
     *      int             shift,    *      int             shift,        *
     *      target_ulong    next_eip) *      int             next_eip)     */

    /* 3.1 arg1 : selector
     * 3.2 arg2 : offset   */
    IR2_OPND t2 = ir2_opnd_new(IR2_OPND_GPR, 14);
    append_ir2_opnd3(LISA_OR, &t2, &offset_opnd, &zero_ir2_opnd);
    append_ir2_opnd3(LISA_OR, &arg1_ir2_opnd, &selector_opnd, &zero_ir2_opnd);
    append_ir2_opnd3(LISA_OR, &arg2_ir2_opnd, &t2, &zero_ir2_opnd);
    /* 3.3 arg3 : shift according to data size */
    append_ir2_opnd2i(LISA_ORI, &arg3_ir2_opnd,
            &zero_ir2_opnd, shift);
    /* 3.4 arg4 : next eip */
    load_imm32_to_ir2(&arg4_ir2_opnd, next_eip, ZERO_EXTENSION);
    /* 3.5 arg0 : env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);

    /* 4. call the helper */
    if (td->sys.pe && !td->sys.vm86) {
        tr_gen_call_to_helper((ADDR)helper_lcall_protected);
    } else {
        tr_gen_call_to_helper((ADDR)helper_lcall_real);
    }

    /* 5. restore native context */
    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 6. disable EIP update in the later jmp */
    td->ignore_eip_update = 1;
}

/* End of TB in system-mode */
bool translate_call_far(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    if (ir1_opnd_is_imm(opnd0)) {
        translate_call_far_imm(pir1);
    }
    else {
        lsassertm_illop(ir1_addr(pir1),
                ir1_opnd_is_mem(opnd0),
                "not a valid call far: pir1 = %p\n", (void*)pir1);
        translate_call_far_mem(pir1);
    }

    return true;
}

bool translate_ret(IR1_INST *pir1)
{
    /* ESP update   according to dflag(opnd size)
     * Value size   according to dflag(opnd size)
     * Address size according to aflag(addr size) : load ret addr
     *
     * code32 || (code16 && prefix_data) : dflag = 4
     * code16 || (code32 && prefix_data) : dflag = 2
     *
     * aflag = ir1_addr_size(pir1) : provided by capstone
     * code32 || (code16 && prefix_addr) : aflag = 4
     * code16 || (code32 && prefix_addr) : aflag = 2
     *
     * Usually the dflag(opnd size) stores in inst's operand.
     * But 'ret' is allowed to have zero operand. And if it has
     * one operand, it must be 'imm16' which must has 16-bit opnd size.
     * So there is no way to get the dflag from pir1 itself.
     *
     * In normal situation, the capstone will translate zero operand with
     * different opnd size into serival instruction, such as 'ins'.
     * But near 'ret' has only one instruction in capstone...... */

    int data_size = ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);
    int addr_size = ir1_addr_size(pir1);
    lsassert(addr_size == 2 || addr_size == 4);

    /* 1. load ret_addr into $25 from MEM(SS:ESP) */
    IR1_OPND mem_ir1_opnd;
    ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0, 0, 0);
    IR2_OPND return_addr_opnd = ra_alloc_dbt_arg2();
    int ss_addr_size = get_sys_stack_addr_size();
    load_ir1_mem_to_ir2(&return_addr_opnd, &mem_ir1_opnd, ZERO_EXTENSION, false, ss_addr_size);

    /* 2. apply address size */
    if (data_size == 32 && addr_size == 2) {
        append_ir2_opnd2_(lisa_mov16z, &return_addr_opnd, &return_addr_opnd);
    }

    /* 3. update ESP */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    if (pir1 != NULL && ir1_opnd_num(pir1) && ir1_opnd_is_imm(ir1_get_opnd(pir1, 0))) {
        if (lsenv->tr_data->sys.ss32) {
            append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd,
                ir1_opnd_uimm(ir1_get_opnd(pir1, 0)) + (data_size >> 3));
        } else {
            IR2_OPND tmp = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd,
                ir1_opnd_uimm(ir1_get_opnd(pir1, 0)) + (data_size >> 3));
            store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
            //avoid em_check in tr_fini failed
//            append_ir2_opnd2(mips_mov32_zx, &esp_opnd, &esp_opnd);
            ra_free_temp(&tmp);
        }
    }
    else {
        if (lsenv->tr_data->sys.ss32) {
            append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd,
                              (data_size >> 3));
        } else {
            IR2_OPND tmp = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd,
                              (data_size >> 3));
            store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
            //avoid em_check in tr_fini failed
//            append_ir2_opnd2(mips_mov32_zx, &esp_opnd, &esp_opnd);
            ra_free_temp(&tmp);
        }
    }

    tr_generate_exit_tb(pir1, 0);

    return true;
}

static bool do_translate_pusha(IR1_INST *pir1, int size)
{
    lsassert(size == 2 || size == 4);

    int data_size = ir1_data_size(pir1);
    lsassert(size == (data_size >> 3));

#ifdef CONFIG_BTMMU
    tr_disable_btmmu();
#endif

    /* 1. save complete context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* 2. pusha: push all 8 GRP to stack
     *
     * ===== pushal =======|====== pushaw =====
     *                     |
     * ESP      -> ------- | ESP      -> ------
     *             | EAX | |             | AX |
     * ESP - 4  -> ------- | ESP - 2  -> ------
     *             | ECX | |             | CX |
     * ESP - 8  -> ------- | ESP - 4  -> ------
     *             | EDX | |             | DX |
     * ESP - 12 -> ------- | ESP - 6  -> ------
     *             | EBX | |             | BX |
     * ESP - 16 -> ------- | ESP - 8  -> ------ push stack
     *             | ESP | |             | SP | pointer value
     * ESP - 20 -> ------- | ESP - 10 -> ------ before updating
     *             | EBP | |             | BP |
     * ESP - 24 -> ------- | ESP - 12 -> ------
     *             | ESI | |             | SI |
     * ESP - 28 -> ------- | ESP - 14 -> ------
     *             | EDI | |             | DI |
     * ESP - 32 -> ------- | ESP - 16 -> ------ */

    IR1_OPND mem_ir1_opnd;
    IR2_OPND gpr_value = ra_alloc_itemp();
    int esp_dec = 0;
    int esp_dec_step = 0 - size;
    int i = 0;

    /* 2.1 build IR2 opnd for MEM(SS:ESP) */
    IR2_OPND mem_opnd;
    int ss_addr_size = get_sys_stack_addr_size();

    IR2_OPCODE ld_ir2_op;
    IR2_OPCODE st_ir2_op;

    if (size == 2) {
        ld_ir2_op = LISA_LD_HU;
        st_ir2_op = LISA_ST_H;
    } else  if (size == 4) {
        ld_ir2_op = LISA_LD_WU;
        st_ir2_op = LISA_ST_W;
    } else {
        lsassert(0);
    }

    int save_temp = 1;
    for (i = 0; i < 8; ++i) {
        /* 2.2 adjust ESP
         *     i = 0 : esp_inc =  -4 or  -2
         *     i = 1 : esp_inc =  -8 or  -4
         *     
         *     i = 7 : esp_inc = -32 or -16 */
        esp_dec = esp_dec + esp_dec_step;
        /* 2.3 load GPR value from ENV */
        append_ir2_opnd2i(ld_ir2_op, &gpr_value, &env_ir2_opnd,
                          lsenv_offset_of_gpr(lsenv, i));
        /* 2.4 update mem offset */
        ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
                                X86_REG_SS, X86_REG_ESP, esp_dec, 0, 0);
        convert_mem_opnd(&mem_opnd, &mem_ir1_opnd, ss_addr_size);
        /* 2.5 store into stack */
        gen_ldst_softmmu_helper(st_ir2_op, &gpr_value, &mem_opnd, save_temp);
        ra_free_temp(&mem_opnd);
    }
    ra_free_temp(&gpr_value);

    /* 3. restore complete context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

    /* 4. update ESP */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    if (lsenv->tr_data->sys.ss32) {
        append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, -8 * size);
    } else {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, -8 * size);
        store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        ra_free_temp(&tmp);
    }

#ifdef CONFIG_BTMMU
    tr_reset_btmmu();
#endif

    return true;
}

static bool do_translate_popa(IR1_INST *pir1, int size)
{
    lsassert(size == 2 || size == 4);

    int data_size = ir1_data_size(pir1);
    lsassert(size == (data_size >> 3));

#ifdef CONFIG_BTMMU
    tr_disable_btmmu();
#endif

    /* 1. save complete context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* 2. popa: pop all 8 GRP to stack
     *
     * ===== popal ========|====== popaw ======
     *                     |
     * ESP + 32 -> ------- | ESP + 16 -> ------
     *             | EAX | |             | AX |
     * ESP + 28 -> ------- | ESP + 14 -> ------
     *             | ECX | |             | CX |
     * ESP + 24 -> ------- | ESP + 12 -> ------
     *             | EDX | |             | DX |
     * ESP + 20 -> ------- | ESP + 10 -> ------
     *             | EBX | |             | BX |
     * ESP + 16 -> ------- | ESP + 8  -> ------ NOT pop
     *             | ESP | |             | SP | stack pointer
     * ESP + 12 -> ------- | ESP + 6  -> ------
     *             | EBP | |             | BP |
     * ESP + 8  -> ------- | ESP + 4  -> ------
     *             | ESI | |             | SI |
     * ESP + 4  -> ------- | ESP + 2  -> ------
     *             | EDI | |             | DI |
     * ESP      -> ------- | ESP      -> ------ */

    IR1_OPND mem_ir1_opnd;
    IR2_OPND gpr_value = ra_alloc_itemp();
    int esp_inc = 8 * size;
    int esp_inc_step = 0 - size;
    int i = 0;

    /* 2.1 build IR2 opnd for MEM(SS:ESP) */
    IR2_OPND mem_opnd;
    int ss_addr_size = get_sys_stack_addr_size();

    IR2_OPCODE ld_ir2_op;
    IR2_OPCODE st_ir2_op;

    if (size == 2) {
        ld_ir2_op = LISA_LD_HU;
        st_ir2_op = LISA_ST_H;
    } else  if (size == 4) {
        ld_ir2_op = LISA_LD_WU;
        st_ir2_op = LISA_ST_W;
    } else {
        lsassert(0);
    }

    int save_temp = 1;
    for (i = 0; i < 8; ++i) {
        /* 2.2 adjust ESP
         *     i = 0 : esp_inc = 28 or 14
         *     i = 1 : esp_inc = 24 or 12
         *     
         *     i = 7 : esp_inc = 0        */
        esp_inc = esp_inc + esp_inc_step;
        /* 2.3 ignore ESP pop */
        if (i == esp_index) continue;
        /* 2.4 update mem offset */
        ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
                                X86_REG_SS, X86_REG_ESP, esp_inc, 0, 0);
        convert_mem_opnd(&mem_opnd, &mem_ir1_opnd, ss_addr_size);
        /* 2.5 load from stack */
        gen_ldst_softmmu_helper(ld_ir2_op, &gpr_value, &mem_opnd, save_temp);
        ra_free_temp(&mem_opnd);
        /* 2.6 write poped GPR value to ENV */
        append_ir2_opnd2i(st_ir2_op, &gpr_value, &env_ir2_opnd,
                          lsenv_offset_of_gpr(lsenv, i));
    }
    ra_free_temp(&gpr_value);

    /* 4. restore complete context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

    /* 5. update ESP */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    if (lsenv->tr_data->sys.ss32) {
        append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, 8 * size);
    } else {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, 8 * size);
        store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        ra_free_temp(&tmp);
    }

#ifdef CONFIG_BTMMU
    tr_reset_btmmu();
#endif

    return true;
}

bool translate_pushaw(IR1_INST *pir1)
{
    return do_translate_pusha(pir1, 2); /* 16-bit */
}
bool translate_pushal(IR1_INST *pir1)
{
    return do_translate_pusha(pir1, 4); /* 32-bit */
}
bool translate_popaw(IR1_INST *pir1)
{
    return do_translate_popa(pir1, 2);  /* 16-bit */
}
bool translate_popal(IR1_INST *pir1)
{
    return do_translate_popa(pir1, 4);  /* 32-bit */
}

static bool do_translate_bcd(IR1_INST *pir1, int val)
{
    /* 1. only need to save eflags */
    helper_cfg_t cfg = zero_helper_cfg;
    cfg.sv_eflags = 1;

    /* 2. call helper and sync the eflags
     *    > EAX is used (caller-saved, always saved)
     *    > eflags is used and updated
     *
     * target/i386/int_helper.c
     * void helper_daa(CPUX86State *env)
     * void helper_das(CPUX86State *env)
     * void helper_aaa(CPUX86State *env)
     * void helper_aas(CPUX86State *env)
     * void helper_aam(CPUX86State *env, int base)
     * void helper_aad(CPUX86State *env, int base) */
    switch (ir1_opcode(pir1)) {
    case X86_INS_DAA:
        tr_sys_gen_call_to_helper1_cfg((ADDR)helper_daa, cfg);
        break;
    case X86_INS_DAS:
        tr_sys_gen_call_to_helper1_cfg((ADDR)helper_das, cfg);
        break;
    case X86_INS_AAA:
        tr_sys_gen_call_to_helper1_cfg((ADDR)helper_aaa, cfg);
        break;
    case X86_INS_AAS:
        tr_sys_gen_call_to_helper1_cfg((ADDR)helper_aas, cfg);
        break;
    case X86_INS_AAM:
        tr_sys_gen_call_to_helper2_cfg((ADDR)helper_aam, val, cfg);
        break;
    case X86_INS_AAD:
        tr_sys_gen_call_to_helper2_cfg((ADDR)helper_aad, val, cfg);
        break;
    default: 
        /* should never reach here */
        break;
    }

    return true;
}

bool translate_daa(IR1_INST *pir1) { return do_translate_bcd(pir1, 1); }
bool translate_aaa(IR1_INST *pir1) { return do_translate_bcd(pir1, 1); }
bool translate_aas(IR1_INST *pir1) { return do_translate_bcd(pir1, 1); }
bool translate_das(IR1_INST *pir1) { return do_translate_bcd(pir1, 1); }
bool translate_aam(IR1_INST *pir1)
{
    int val;
    if(!ir1_opnd_num(pir1)){
        val = 0x0A;
    }
    else {
        val = ir1_opnd_uimm(ir1_get_opnd(pir1, 0));
    }

    return do_translate_bcd(pir1, val);
}
bool translate_aad(IR1_INST *pir1)
{
    int val;
    if(!ir1_opnd_num(pir1)){
        val = 0x0A;
    }
    else {
        val = ir1_opnd_uimm(ir1_get_opnd(pir1, 0));
    }

    return do_translate_bcd(pir1, val);
}

bool translate_enter(IR1_INST *pir1)
{
    lsassertm(0, "enter to be implemented in LoongArch.\n");
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* esp_addend */
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* level */
//    int esp_addend  = ir1_opnd_uimm(opnd0);
//    int level       = ir1_opnd_uimm(opnd1);
//
//    TRANSLATION_DATA *td = lsenv->tr_data;
//
//    int data_size = ir1_data_size(pir1);
//
//    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
//    IR2_OPND ebp_opnd = ra_alloc_gpr(ebp_index);
//
//    IR2_OPND frametemp = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_subi_addrx, &frametemp, &esp_opnd, data_size >> 3);
//
//    /* 1. push EBP into stack MEM(SS:ESP-d)*/
//    IR1_OPND mem_ir1_opnd;
//    /* 1.1 build MEM(SS:ESP-d) */
//    ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
//            X86_REG_SS, X86_REG_ESP, 0 - (data_size >> 3), 0, 0);
//    /* 1.2 store */
//    int ss_addr_size = get_sys_stack_addr_size();
//    store_ir2_to_ir1_mem(&ebp_opnd, &mem_ir1_opnd, false, ss_addr_size);
//
//    level &= 31;
//    if (level) {
//        int i;
//        /* copy level-1 pointers from the previous frame */
//        for (i = 1; i < level; ++i) {
//            ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
//                    X86_REG_SS, X86_REG_EBP, 0 - (data_size >> 3) * i, 0, 0);
//            IR2_OPND value = ra_alloc_itemp();
//            load_ir1_mem_to_ir2(&value, &mem_ir1_opnd, ZERO_EXTENSION, false, ss_addr_size);
//
//            ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
//                    X86_REG_SS, X86_REG_ESP, 0 - (data_size >> 3) * (i + 1), 0, 0);
//            store_ir2_to_ir1_mem(&value, &mem_ir1_opnd, false, ss_addr_size);
//        }
//        /* push current FrameTemp as the last level */
//        ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
//                X86_REG_SS, X86_REG_ESP, 0 - (data_size >> 3) * (level + 1), 0, 0);
//        store_ir2_to_ir1_mem(&frametemp, &mem_ir1_opnd, false, ss_addr_size);
//    }
//
//    /* copy FrameTemp vlaue to EBP */
//    if (td->sys.ss32) {
//        store_ir2_to_ir1_gpr(&frametemp, &ebp_ir1_opnd);
//    } else {
//        store_ir2_to_ir1_gpr(&frametemp, &bp_ir1_opnd);
//    }
//
//    /* compute final value of ESP */
//    IR2_OPND final_esp = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_subi_addrx, &final_esp, &frametemp, esp_addend + (data_size >> 3) * level);
//    ra_free_temp(&frametemp);
//    if (td->sys.ss32) {
//        store_ir2_to_ir1_gpr(&final_esp, &esp_ir1_opnd);
//    } else {
//        store_ir2_to_ir1_gpr(&final_esp, &sp_ir1_opnd);
//    }
//    ra_free_temp(&final_esp);
//
    return true;
}

bool translate_leave(IR1_INST *pir1)
{
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    IR2_OPND ebp_opnd = ra_alloc_gpr(ebp_index);

    int ss_addr_size = get_sys_stack_addr_size();
    int data_size = ir1_data_size(pir1);

    /* 1. load value from MEM(SS:EBP) */
    IR1_OPND mem_ir1_opnd;
    ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_EBP, 0, 0, 0);

    IR2_OPND new_ebp = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&new_ebp, &mem_ir1_opnd, ZERO_EXTENSION, false, ss_addr_size);;

    IR2_OPND new_esp = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ADDI_D, &new_esp, &ebp_opnd, data_size >> 3);

    /* 2. update EBP according to data_size */
    if (data_size == 32){
        append_ir2_opnd2_(lisa_mov, &ebp_opnd, &new_ebp);
    } else if (data_size == 16){
        store_ir2_to_ir1_gpr(&new_ebp, &bp_ir1_opnd);
    }

    /* 3. update ESP according to ss_addr_size */
    if(ss_addr_size == 4) {
        append_ir2_opnd2_(lisa_mov, &esp_opnd, &new_esp);
    } else {
        store_ir2_to_ir1_gpr(&new_esp, &sp_ir1_opnd);
    }

    return true;
}

static bool do_translate_lxx(IR1_INST *pir1, IR1_OPND *seg_ir1_opnd)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* dest: GPR */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src : MEM */

    /*      data size | 16 | 32 | mem opnd
     * -------------------------------------
     *    offset size | 16 | 32 |  m16:16
     *  selector size | 16 | 16 |  m16:32    */

    int opnd_size = ir1_opnd_size(opnd0);
    int data_size = ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);
    lsassert(opnd_size == data_size);

    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd1, -1);
    int save_temp = 1;

    TRANSLATION_DATA *td = lsenv->tr_data;

    /* 1. load offset at MEM */
    IR2_OPND offset_opnd   = ra_alloc_itemp();
    IR2_OPCODE opc = data_size == 16 ? LISA_LD_HU: LISA_LD_WU;
    gen_ldst_softmmu_helper(opc, &offset_opnd, &mem_opnd, save_temp);
    /* 1.1 save offset to not be ruined by store_ir2_to_ir1_seg */
    if (td->sys.pe && !td->sys.vm86) {
        append_ir2_opnd2i(LISA_ST_D, &offset_opnd, &env_ir2_opnd,
                lsenv_offset_of_mips_iregs(lsenv, 0));
    }

    /* 2. load selector at MEM + 2 or 4 */
    IR2_OPND mem;
    switch (data_size >> 3) {
    case 2: mem = convert_mem_ir2_opnd_plus_2(&mem_opnd); break;
    case 4: mem = convert_mem_ir2_opnd_plus_4(&mem_opnd); break;
    default: break;
    }
    IR2_OPND selector_opnd = ra_alloc_itemp();
    gen_ldst_softmmu_helper(LISA_LD_HU, &selector_opnd, &mem, save_temp);

    /* 3. store selector to segment register */
    store_ir2_to_ir1_seg(&selector_opnd, seg_ir1_opnd);
    ra_free_temp(&selector_opnd);

    /* 4.0 load offset if it is saved */
    if (td->sys.pe && !td->sys.vm86) {
        append_ir2_opnd2i(LISA_LD_D, &offset_opnd, &env_ir2_opnd,
                lsenv_offset_of_mips_iregs(lsenv, 0));
    }
    /* 4. move offset into GPR.
     *    Do this at the end for precise exception. */
    store_ir2_to_ir1_gpr(&offset_opnd, opnd0);
    ra_free_temp(&offset_opnd);

    /* 5. no inhibit irq from store_ir2_to_ir1_seg */
    td->inhibit_irq = 0;

    return true;
}

/* End of TB in system-mode : PE & !vm86 & code32 */
bool translate_lds(IR1_INST *pir1)
{
    IR1_OPND seg_ir1_opnd;
    ir1_opnd_build_reg(&seg_ir1_opnd, 16, X86_REG_DS);
    return do_translate_lxx(pir1, &seg_ir1_opnd);
}
/* End of TB in system-mode : PE & !vm86 & code32*/
bool translate_les(IR1_INST *pir1)
{
    IR1_OPND seg_ir1_opnd;
    ir1_opnd_build_reg(&seg_ir1_opnd, 16, X86_REG_ES);
    return do_translate_lxx(pir1, &seg_ir1_opnd);
}
bool translate_lfs(IR1_INST *pir1)
{
    IR1_OPND seg_ir1_opnd;
    ir1_opnd_build_reg(&seg_ir1_opnd, 16, X86_REG_FS);
    return do_translate_lxx(pir1, &seg_ir1_opnd);
}
bool translate_lgs(IR1_INST *pir1)
{
    IR1_OPND seg_ir1_opnd;
    ir1_opnd_build_reg(&seg_ir1_opnd, 16, X86_REG_GS);
    return do_translate_lxx(pir1, &seg_ir1_opnd);
}
/* End of TB in system-mode */
bool translate_lss(IR1_INST *pir1)
{
    IR1_OPND seg_ir1_opnd;
    ir1_opnd_build_reg(&seg_ir1_opnd, 16, X86_REG_SS);
    return do_translate_lxx(pir1, &seg_ir1_opnd);
}

/* End of TB in system-mode */
bool translate_vmrun(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    tr_gen_save_curr_eip();
    helper_cfg_t cfg = default_helper_cfg;
    int aflag = ir1_addr_size(pir1) == 2 ? 0 : 1;
    tr_sys_gen_call_to_helper3_cfg((ADDR)helper_vmrun, aflag, ir1_inst_size(pir1), cfg);

    // the original qemu code is:
    // tcg_gen_exit_tb(NULL, 0);
    // s->base.is_jmp = DISAS_NORETURN;
    // But there is no translator_loop in xqm, so I simply set this inst as a eob inst.
    return true;
}

bool translate_vmsave(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    tr_gen_save_curr_eip();
    helper_cfg_t cfg = default_helper_cfg;
    int aflag = ir1_addr_size(pir1) == 2 ? 0 : 1;
    tr_sys_gen_call_to_helper2_cfg((ADDR)helper_vmsave, aflag, cfg);

    return true;
}

bool translate_vmload(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    tr_gen_save_curr_eip();
    helper_cfg_t cfg = default_helper_cfg;
    int aflag = ir1_addr_size(pir1) == 2 ? 0 : 1;
    tr_sys_gen_call_to_helper2_cfg((ADDR)helper_vmload, aflag, cfg);

    return true;
}

/* End of TB in system-mode */
bool translate_stgi(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_stgi, cfg);
    tr_gen_save_curr_eip(); // tcg place jmp_im after gen helper

    return true;
}

bool translate_clgi(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    tr_gen_save_curr_eip();
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_clgi, cfg);
    
    return true;
}

bool translate_vmmcall(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    tr_gen_save_curr_eip();
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_vmmcall, cfg);

    return true;
}

bool translate_skinit(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    tr_gen_save_curr_eip();
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_skinit, cfg);

    return true;
}

bool translate_invlpga(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    int addr_size = ir1_addr_size(pir1);
    lsassert(addr_size == 2 || addr_size == 4);

    /* 0. save current instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. helper_invlpga
     *
     * target/i386/svm_helper.c
     * void helper_invlpga(
     *      CPUX86State *env,
     *      int         aflag)           */
    helper_cfg_t cfg = default_helper_cfg;

    /* 1.1 aflag */
    int aflag = addr_size == 2 ? 0 : 1;

    /* 1.2 call helper_invlpga */
    tr_sys_gen_call_to_helper2_cfg((ADDR)helper_invlpga, aflag, cfg);

    return true;
}

/* End of TB in system-mode */
bool translate_invlpg(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    lsassert(ir1_opnd_is_mem(opnd0));

    /* 0. save current instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. get mem address */
    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd0, -1);

    int mem_no_offset_new_tmp = 0;

    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd,
            &mem_no_offset_new_tmp);
    if (mem_no_offset_new_tmp) ra_free_temp(&mem_opnd);

    IR2_OPND addr_opnd = ir2_opnd_mem_get_base(&mem_no_offset);

    /* 2. helper_invlpg
     *
     * target/i386/misc_helper.c
     * void helper_invlpg(
     *      CPUX86State *env,
     *      target_ulong addr)      */

    /* 2.0 save context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* 2.1 arg1: address */
    append_ir2_opnd3(LISA_OR, &arg1_ir2_opnd, &addr_opnd, &zero_ir2_opnd);
    ra_free_temp(&addr_opnd);
    /* 2.2 arg0: env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);

    /* 2.3 call helper_invlpg */
    tr_gen_call_to_helper((ADDR)helper_invlpg);

    /* 2.4 restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

    return true;
}

bool translate_xadd(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* dest: GPR/MEM */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src : GPR     */

    IR2_OPND src0 = ra_alloc_itemp();
    IR2_OPND src1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src1, opnd1, SIGN_EXTENSION, false);

    IR2_OPND sum = ra_alloc_itemp();

    if (ir1_opnd_is_gpr(opnd0)) {
        load_ir1_to_ir2(&src0, opnd0, SIGN_EXTENSION, false);
        append_ir2_opnd3(LISA_ADD_W, &sum, &src0, &src1);
        store_ir2_to_ir1(&src0, opnd1, false);
        store_ir2_to_ir1(&sum, opnd0, false);
    } else {
        if (ir1_has_prefix_lock(pir1) && 
            td->sys.cflags & CF_PARALLEL) {
//            /* helper_atomic_fetch_addb
//             * helper_atomic_fetch_addw_le
//             * helper_atomic_fetch_addl_le
//             * helper_atomic_fetch_addq_le */
            lsassertm(0, "compile flag parallel not supported.\n");
        } else {
            load_ir1_to_ir2(&src0, opnd0, SIGN_EXTENSION, false);
            append_ir2_opnd3(LISA_ADD_W, &sum, &src0, &src1);
            store_ir2_to_ir1(&sum, opnd0, false);
        }
        store_ir2_to_ir1(&src0, opnd1, false);
    }

    generate_eflag_calculation(&sum, &src0, &src1, pir1, true);

    ra_free_temp(&sum);

    return true;
}

bool translate_rdtsc(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    /* 0. save next instruciton's EIP to env */
    tr_gen_save_curr_eip();

    if (td->sys.cflags & CF_USE_ICOUNT) {
        tr_gen_io_start();
    }

    /* 1. call helper_rdtsc
     *
     * target/i386/misc_helper.c
     * void helper_rdtsc(CPUX86State *env)
     * */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_rdtsc, cfg);

    // if (td->sys.cflags & CF_USE_ICOUNT) {
    //     tr_gen_io_end();
    // }

    return true;
}

bool translate_rdtscp(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;

    /* 0. save next instruciton's EIP to env */
    tr_gen_save_curr_eip();

    if (td->sys.cflags & CF_USE_ICOUNT) {
        tr_gen_io_start();
    }

    /* 1. call helper_rdtsc
     *
     * target/i386/misc_helper.c
     * void helper_rdtscp(CPUX86State *env)
     * */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_rdtscp, cfg);

    // if (td->sys.cflags & CF_USE_ICOUNT) {
    //     tr_gen_io_end();
    // }

    return true;
}

bool translate_rdpmc(IR1_INST *pir1)
{
    /* 0. save next instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. call helper_rdpmc
     *
     * target/i386/misc_helper.c
     * void helper_rdpmc(CPUX86State *env)
     * >> exception might be generated */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_rdpmc, cfg);

    return true;
}

bool translate_hlt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_HLT(pir1);

    /* 0. save next instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. call helper_hlt
     *
     * target/i386/misc_helper.c
     * void helper_hlt(
     *      CPUX86State  *env,
     *      int           next_eip_addend)
     * */
    helper_cfg_t cfg = default_helper_cfg;

    int next_eip_addend = ir1_inst_size(pir1);
    tr_sys_gen_call_to_helper2_cfg((ADDR)helper_hlt, next_eip_addend, cfg);

    /* 2. should nevet reach here */
    tr_gen_infinite_loop();

    return true;
}

bool translate_cpuid(IR1_INST *pir1)
{
    /* 0. save next instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. call helper_cpuid
     *
     * void helper_cpuid(
     *      CPUX86State *env)
     * >> load new EAX/ECX/EDX/EBX
     * */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_cpuid, cfg);

    return true;
}

void tr_gen_fwait(void)
{
    helper_cfg_t cfg = all_helper_cfg;
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_fwait, cfg);
}

bool translate_wait(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    tr_gen_fwait();

    return true;
}

bool translate_cmpxchg8b(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_ARPL(pir1);

    /* 1. check illegal operation exception */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    lsassert_illop(ir1_addr(pir1), ir1_opnd_is_mem(opnd0));

    /* 2. get memory address */
    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd0, -1);
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND address = ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) ra_free_temp(&mem_opnd);

    /* 3. select helper function */
    ADDR helper_addr = 0;
    if (ir1_has_prefix_lock(pir1) && td->sys.cflags & CF_PARALLEL) {
        /* target/i386/mem_helper.c
         * void helper_cmpxchg8b(
         *      CPUX86State *env,
         *      target_ulong a0)
         * >> EAX, ECX is used
         */
        helper_addr = (ADDR)helper_cmpxchg8b;
    } else {
        /* target/i386/mem_helper.c
         * void helper_cmpxchg8b_unlocked(
         *      CPUX86State *env,
         *      target_ulong a0)
         * >> EAX, ECX is used
         */
        helper_addr = (ADDR)helper_cmpxchg8b_unlocked;
    }

    /* 4. call that helper */

    /* 4.1 save context */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    /* 4.2 arg1: address */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &address);
    /* 4.3 arg0: env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 4.4 call helper */
    tr_gen_call_to_helper(helper_addr);
    /* 4.5 restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    return true;
}

bool translate_arpl(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    lsassertm(0, "arpl to be implemented in LoongArch.\n");
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR/MEM, 16-bit */
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR,     16-bit */
//    lsassertm_illop(ir1_addr(pir1),
//            (ir1_opnd_is_mem(opnd0) || ir1_opnd_is_gpr(opnd0)) && ir1_opnd_is_gpr(opnd1),
//            "not a valid ARPL insn: pir1 = %p\n", (void*)pir1);
//
//    /* 1. load segment selectors */
//    IR2_OPND seg0 = ra_alloc_itemp();
//    IR2_OPND seg1 = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&seg0, opnd0, ZERO_EXTENSION, false);
//    load_ir1_to_ir2(&seg1, opnd1, ZERO_EXTENSION, false);
//
//    /* 2. get segment RPL */
//    IR2_OPND rpl0 = ra_alloc_itemp(); /* 000...000 00xx */
//    IR2_OPND rpl1 = ra_alloc_itemp(); /* 000...000 00xx */
//    IR2_OPND rpl_mask = ra_alloc_itemp(); /* 000...000 0011 */
//    append_ir2_opnd2i(mips_ori, &rpl_mask, &zero_ir2_opnd, 0x3);
//    append_ir2_opnd3(mips_and, &rpl0, &seg0, &rpl_mask);
//    append_ir2_opnd3(mips_and, &rpl1, &seg1, &rpl_mask);
//    ra_free_temp(&seg1);
//
//    /* 3. compare and set */
//
//    /* 3.1 prepare for eflags */
//    IR2_OPND *eflags = &eflags_ir2_opnd;
//    IR2_OPND  zf; /* 000..00 0100 0000 */
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    /* 3.2 compare RPL */
//    IR2_OPND label1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_subu, &rpl0, &rpl0, &rpl1);
//    /* rpl0 - rpl1 = res
//     * ---------------------
//     * rpl0 < rpl1 : res < 0
//     * rpl0 = rpl1 : res = 0
//     * rpl0 > rpl1 : res > 0 */
//    append_ir2_opnd2(mips_bgez, &rpl0, &label1);
//    ra_free_temp(&rpl0);
//
//    /* 3.3 RPL0 < RPL1 
//     *     1. set ZF
//     *     2. set seg0.rpl as seg1.rpl */
//    append_ir2_opnd3(mips_nor, &rpl_mask, &rpl_mask, &zero_ir2_opnd); /* 111...111 1100 */
//    append_ir2_opnd3(mips_and, &seg0, &seg0, &rpl_mask);
//    append_ir2_opnd3(mips_or, &seg0, &seg0, &rpl1);
//    ra_free_temp(&rpl_mask);
//    ra_free_temp(&rpl1);
//    store_ir2_to_ir1(&seg0, opnd0, false);
//    ra_free_temp(&seg0);
//    /* 3.3.1 set ZF */
//    zf = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_ori, &zf, &zero_ir2_opnd, ZF_BIT);
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zf, 0x8);
//    } else {
//        append_ir2_opnd3(mips_or, eflags, eflags, &zf);
//    }
//
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* 3.4 RlL0 >= RPL1
//     *     1. clear ZF */
//    append_ir2_opnd1(mips_label, &label1);
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x8);
//    } else {
//        append_ir2_opnd3(mips_nor, &zf, &zf, &zero_ir2_opnd);
//        append_ir2_opnd3(mips_and, eflags, eflags, &zf);
//    }
//    ra_free_temp(&zf);
//
//    /* 4. exit */
//    append_ir2_opnd1(mips_label, &label_exit);

    return true;
}

static bool do_translate_lar_lsl(IR1_INST *pir1, int is_lar)
{
    lsassertm(0, "lar lsl to be implemented in LoongArch.\n");
//    TRANSLATION_DATA *td = lsenv->tr_data;
//    CHECK_EXCP_LARLSL(pir1);
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR */
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/m16 */
//
//    /* 1. load selector */
//    IR2_OPND selector = ra_alloc_itemp();
//    load_ir1_to_ir2(&selector, opnd1, UNKNOWN_EXTENSION, false);
//
//    /* 2. call helper_lar
//     *
//     * target/i386/seg_helper.c
//     * target_ulong helper_lar(     target_ulong helper_lsl(
//     *      CPUX86State *env,           CPUX86State *env,
//     *      target_ulong selector1)     target_ulong selector1)
//     *
//     * >> eflags is used and updated
//     * >> exception might be generated                          */
//     
//    helper_cfg_t cfg = default_helper_cfg;
//
//    /* 2.1 save native context */
//    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
//    /* 2.2 arg1 : selector */
//    append_ir2_opnd3(mips_or, &arg1_ir2_opnd, &selector, &zero_ir2_opnd);
//    /* 2.3 arg0 : env */
//    append_ir2_opnd3(mips_or, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
//    /* 2.4 call helper */
//    if (is_lar) {
//        tr_gen_call_to_helper((ADDR)helper_lar);
//    } else {
//        tr_gen_call_to_helper((ADDR)helper_lsl);
//    }
//    /* 2.5 restore context */
//    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
//
//    /* 3. get the return value */
//    IR2_OPND value = ra_alloc_itemp();
//    append_ir2_opnd3(mips_or, &value, &ret_ir2_opnd, &zero_ir2_opnd);
//
//    /* 4. check ZF flag in EFLAGS */
//    IR2_OPND zf = ra_alloc_itemp();
//    if (option_lbt) {
//        append_ir2_opnd2(mips_mov64, &zf, &zero_ir2_opnd);
//        append_ir2_opnd1i(mips_mfflag, &zf, 0x8);
//    }
//    else {
//        append_ir2_opnd2i(mips_ori, &zf, &zero_ir2_opnd, ZF_BIT);
//        append_ir2_opnd3(mips_and, &zf, &eflags_ir2_opnd, &zf);
//    }
//
//    /* 5. mov to destination if ZF is 1 */
//    IR2_OPND label = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd3(mips_beq, &zf, &zero_ir2_opnd, &label);
//    ra_free_temp(&zf);
//
//    store_ir2_to_ir1_gpr(&value, opnd0);
//    ra_free_temp(&value);
//
//    /* 6. exit label */
//    append_ir2_opnd1(mips_label, &label);
     
    return true;
}
bool translate_lar(IR1_INST *pir1)
{
    return do_translate_lar_lsl(pir1, 1);
}
bool translate_lsl(IR1_INST *pir1)
{
    return do_translate_lar_lsl(pir1, 0);
}

/* End of TB in system-mode */
bool translate_sysenter(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_SYSENTER(pir1);

    /* 1. call helper_sysenter
     *
     * target/i386/seg_helper.c
     * void helper_sysenter(CPUX86State *env)
     * >> EIP is updated
     */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_sysenter, cfg);

    /* 2. disable EIP update in the added jmp */
    td->ignore_eip_update = 1;

    return true;
}

/* End of TB in system-mode */
bool translate_sysexit(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_SYSEXIT(pir1);

    int data_size = ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);

    /* 1. call helper_sysexit
     *
     * target/i386/seg_helper.c
     * void helper_sysexit(
     *      CPUX86State *env,
     *      int dflag)
     */
    helper_cfg_t cfg = default_helper_cfg;

    /* 1.1 dflag = 0 : 16-bit
     *     dflag = 1 : 32-bit */
    int dflag = (data_size >> 4) - 1;
    lsassert(!(dflag >> 1));

    /* 1.2 call helper_sysexit*/
    tr_sys_gen_call_to_helper2_cfg((ADDR)helper_sysexit, dflag, cfg);

    /* 2. disable EIP update in the added jmp */
    td->ignore_eip_update = 1;

    return true;
}

bool translate_wrmsr(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_WRMSR(pir1);

    /* target/i386/misc_helper.c
     * void helper_wrmsr(CPUX86State *env)
     * >> EAX, ECX, EDX are used
     * */
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_wrmsr, default_helper_cfg);

    return true;
}

bool translate_rdmsr(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_RDMSR(pir1);

    /* target/i386/misc_helper.c
     * void helper_rdmsr(CPUX86State *env)
     * >> EAX, ECX, EDX are used
     * */
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_rdmsr, default_helper_cfg);

    return true;
}

bool translate_rdfsbase(IR1_INST *p) { tr_gen_excp_illegal_op(p, 1); return true; }
bool translate_rdgsbase(IR1_INST *p) { tr_gen_excp_illegal_op(p, 1); return true; }
bool translate_wrfsbase(IR1_INST *p) { tr_gen_excp_illegal_op(p, 1); return true; }
bool translate_wrgsbase(IR1_INST *p) { tr_gen_excp_illegal_op(p, 1); return true; }

bool translate_lzcnt(IR1_INST *pir1)
{
    lsassertm(0, "lzcnt to be implemented in LoongArch.\n");
//    TRANSLATION_DATA *td = lsenv->tr_data;
//
//    if (!ir1_has_prefix_repe(pir1) ||
//        td->sys.cpuid_7_0_ebx_features & CPUID_7_0_EBX_BMI1) {
//        return translate_bsr(pir1);
//    }
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* r16,   r32   */
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 2); /* r/m16, r/m32 */
//
//    int opnd_size = ir1_opnd_size(opnd0);
//    lsassertm_illop(ir1_addr(pir1), opnd_size == 16 || opnd_size == 32,
//            "lzcnt with unsupported opnd size %d.\n", opnd_size);
//
//    /* 1. load source data */
//    IR2_OPND value = ra_alloc_itemp();
//    load_ir1_to_ir2(&value, opnd1, SIGN_EXTENSION, false);
//    
//    /* 2. count leading zero use mips_clz */
//    IR2_OPND lz_num = ra_alloc_itemp();
//    append_ir2_opnd2(mips_clz, &lz_num, &value);
//    ra_free_temp(&value);
//    /* 2.1 clz targets 32-bit, sub 16 when opnd size is 16 */
//    if (opnd_size == 16) {
//        append_ir2_opnd2i(mips_subi, &lz_num, &lz_num, 16);
//    }
//    /* 2.2 store into dest */
//    store_ir2_to_ir1_gpr(&lz_num, opnd0);
//
//    IR2_OPND *eflags = &eflags_ir2_opnd;
//    IR2_OPND mask = ra_alloc_itemp();
//
//    /* 3. if lz_num == opnd_size : CF = 1
//     *    else                   : CF = 0 */
//    /* 3.1 prepare label and CF mask */
//    append_ir2_opnd2i(mips_ori, &mask, &zero_ir2_opnd, CF_BIT);
//    IR2_OPND label_cf = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* 3.2 set CF = 1 */
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &mask, 0x1);
//    } else {
//        append_ir2_opnd3(mips_or, eflags, eflags, &mask);
//    }
//    /* 3.3 branch if opnd_size == lz_num */
//    IR2_OPND size_opnd = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_ori, &size_opnd, &zero_ir2_opnd, opnd_size);
//    append_ir2_opnd3(mips_beq, &lz_num, &size_opnd, &label_cf);
//    ra_free_temp(&size_opnd);
//    /* 3.4 not branch: set CF = 0 */
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x1);
//    } else {
//        append_ir2_opnd3(mips_nor, &mask, &mask, &zero_ir2_opnd);
//        append_ir2_opnd3(mips_and, eflags, eflags, &mask);
//    }
//    /* 3.5 branch : CF = 1 */
//    append_ir2_opnd1(mips_label, &label_cf);
//
//    /* 4. if lz_num == 0 : ZF = 1
//     *    else           : ZF = 0 */
//    /* 4.1 prepare label and ZF mask */
//    append_ir2_opnd2i(mips_ori, &mask, &zero_ir2_opnd, ZF_BIT);
//    IR2_OPND label_zf = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* 4.2 set ZF = 1 */
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &mask, 0x8);
//    } else {
//        append_ir2_opnd3(mips_or, eflags, eflags, &mask);
//    }
//    /* 4.3 branch if opnd_size == 0 */
//    append_ir2_opnd3(mips_beq, &lz_num, &zero_ir2_opnd, &label_zf);
//    ra_free_temp(&lz_num);
//    /* 4.4 not branch: set ZF = 0 */
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x8);
//    } else {
//        append_ir2_opnd3(mips_nor, &mask, &mask, &zero_ir2_opnd);
//        append_ir2_opnd3(mips_and, eflags, eflags, &mask);
//    }
//    ra_free_temp(&mask);
//    /* 4.5 branch : ZF = 1 */
//    append_ir2_opnd1(mips_label, &label_zf);

    return true;
}
bool translate_tzcnt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (!ir1_has_prefix_repe(pir1) ||
        td->sys.cpuid_ext3_features & CPUID_EXT3_ABM) {
        return translate_bsf(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* r16,   r32   */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 2); /* r/m16, r/m32 */

    int opnd_size = ir1_opnd_size(opnd0);
    lsassertm_illop(ir1_addr(pir1), opnd_size == 16 || opnd_size == 32,
            "lzcnt with unsupported opnd size %d.\n", opnd_size);

    /* 1. load source data */
    IR2_OPND value = ra_alloc_itemp();
    load_ir1_to_ir2(&value, opnd1, SIGN_EXTENSION, false);
    
    /* 2. count tailing zero */
    IR2_OPND tz_num = ra_alloc_itemp();
    append_ir2_opnd2_(lisa_mov, &tz_num, &zero_ir2_opnd);
    /* 2.1 load loop cnt */
    IR2_OPND cnt_opnd = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ORI, &cnt_opnd, &zero_ir2_opnd, opnd_size);
    /* 2.2 loop start */
    IR2_OPND loop_label = ir2_opnd_new_label();
    IR2_OPND loop_exit  = ir2_opnd_new_label();
    append_ir2_opnd1(LISA_LABEL, &loop_label);
    append_ir2_opnd3(LISA_BEQ, &cnt_opnd, &zero_ir2_opnd, &loop_exit);
    /* 2.3 compare the lease signifit bit and
     *     shift the source and
     *     minus the loop cnt */
    IR2_OPND bit = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &bit, &value, 0x1);          /* bit   = value &  1 */
    append_ir2_opnd2i(LISA_SRAI_W, &value, &value, 0x1);         /* value = value >> 1 */
    append_ir2_opnd2i(LISA_ADDI_W, &cnt_opnd, &cnt_opnd, -1);  /* cnt   = cnt   -  1 */
    append_ir2_opnd3(LISA_BNE, &bit, &zero_ir2_opnd, &loop_label);
    ra_free_temp(&cnt_opnd);
    ra_free_temp(&bit);
    ra_free_temp(&value);
    /* 2.4 not branch: tz_num += 1 */
    append_ir2_opnd2i(LISA_ADDI_D, &tz_num, &tz_num, 0x1);
    /* 2.5 next loop */
    append_ir2_opnd1(LISA_B, &loop_label);
    /* 2.6 loop exit */
    append_ir2_opnd1(LISA_LABEL, &loop_exit);

    /* 2.7 store into dest */
    store_ir2_to_ir1_gpr(&tz_num, opnd0);

    IR2_OPND *eflags = &eflags_ir2_opnd;
    IR2_OPND mask = ra_alloc_itemp();

    /* 3. if tz_num == opnd_size : CF = 1
     *    else                   : CF = 0 */
    /* 3.1 prepare label and CF mask */
    append_ir2_opnd2i(LISA_ORI, &mask, &zero_ir2_opnd, CF_BIT);
    IR2_OPND label_cf = ir2_opnd_new_label();
    /* 3.2 set CF = 1 */
    if (option_lbt) {
        append_ir2_opnd1i(LISA_X86MTFLAG, &mask, 0x1);
    } else {
        append_ir2_opnd3(LISA_OR, eflags, eflags, &mask);
    }
    /* 3.3 branch if opnd_size == tz_num */
    IR2_OPND size_opnd = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ORI, &size_opnd, &zero_ir2_opnd, opnd_size);
    append_ir2_opnd3(LISA_BEQ, &tz_num, &size_opnd, &label_cf);
    ra_free_temp(&size_opnd);
    /* 3.4 not branch: set CF = 0 */
    if (option_lbt) {
        append_ir2_opnd1i(LISA_X86MTFLAG, &zero_ir2_opnd, 0x1);
    } else {
        append_ir2_opnd3(LISA_NOR, &mask, &mask, &zero_ir2_opnd);
        append_ir2_opnd3(LISA_AND, eflags, eflags, &mask);
    }
    /* 3.5 branch : CF = 1 */
    append_ir2_opnd1(LISA_LABEL, &label_cf);

    /* 4. if tz_num == 0 : ZF = 1
     *    else           : ZF = 0 */
    /* 4.1 prepare label and ZF mask */
    append_ir2_opnd2i(LISA_ORI, &mask, &zero_ir2_opnd, ZF_BIT);
    IR2_OPND label_zf = ir2_opnd_new_label();
    /* 4.2 set ZF = 1 */
    if (option_lbt) {
        append_ir2_opnd1i(LISA_X86MTFLAG, &mask, 0x8);
    } else {
        append_ir2_opnd3(LISA_OR, eflags, eflags, &mask);
    }
    /* 4.3 branch if opnd_size == 0 */
    append_ir2_opnd3(LISA_BEQ, &tz_num, &zero_ir2_opnd, &label_zf);
    ra_free_temp(&tz_num);
    /* 4.4 not branch: set ZF = 0 */
    if (option_lbt) {
        append_ir2_opnd1i(LISA_X86MTFLAG, &zero_ir2_opnd, 0x8);
    } else {
        append_ir2_opnd3(LISA_NOR, &mask, &mask, &zero_ir2_opnd);
        append_ir2_opnd3(LISA_AND, eflags, eflags, &mask);
    }
    ra_free_temp(&mask);
    /* 4.5 branch : CF = 1 */
    append_ir2_opnd1(LISA_LABEL, &label_zf);

    return true;
}

bool translate_lfence(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) return true;

    TRANSLATION_DATA *td = lsenv->tr_data;
    if (td->sys.cflags & CF_PARALLEL) {
        /* TODO: MTTCG */
    }

    return true;
}
bool translate_mfence(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) return true;

    TRANSLATION_DATA *td = lsenv->tr_data;
    if (td->sys.cflags & CF_PARALLEL) {
        /* TODO: MTTCG */
    }

    return true;
}
bool translate_sfence(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) return true;

    TRANSLATION_DATA *td = lsenv->tr_data;
    if (td->sys.cflags & CF_PARALLEL) {
        /* TODO: MTTCG */
    }

    return true;
}

bool translate_bound(IR1_INST *pir1)
{
    lsassertm(0, "bound to be implemented in LoongArch.\n");
//    helper_cfg_t cfg;
//    cfg.sv_allgpr = 1;
//    cfg.sv_eflags = 1;
//    cfg.cvt_fp80  = default_helper_cfg.cvt_fp80;
//    
//    IR2_OPND func_addr_opnd = ra_alloc_dbt_arg2();
//
//    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
//    /* helper_boundw/l(CPUX86State *env, target_ulong a0, int v) */
//    if(ir1_data_size(pir1)==32) {
//        load_addr_to_ir2(&func_addr_opnd, (ADDR)helper_boundl);
//    } else {
//        load_addr_to_ir2(&func_addr_opnd, (ADDR)helper_boundw);
//    }
//
//    /* 1. arg1: target_ulong a0: array index */
//    IR1_OPND *ir1_opnd0 = ir1_get_opnd(pir1, 0);
//    lsassertm(ir1_opnd_type(ir1_opnd0)==X86_OP_REG, "BOUND's first opnd should be a reg\n");
//    IR2_OPND ir2_opnd0 = ra_alloc_itemp();
//    load_ir1_to_ir2(&ir2_opnd0, ir1_opnd0, SIGN_EXTENSION, false);
//    append_ir2_opnd3(mips_or, &arg1_ir2_opnd, &ir2_opnd0, &zero_ir2_opnd);
//    ra_free_temp(&ir2_opnd0);
//
//    /* 2. arg2: int v: array upper&lower bounds memory location */
//    IR1_OPND *ir1_opnd1 = ir1_get_opnd(pir1, 1);
//    lsassertm(ir1_opnd_type(ir1_opnd1)==X86_OP_MEM, "BOUND's second opnd should be a mem location\n");
//
//    IR2_OPND ir2_opnd1;
//    convert_mem_opnd(&ir2_opnd1, ir1_opnd1, -1);
//    IR2_OPND mem_opnd1 = convert_mem_ir2_opnd_no_offset(&ir2_opnd1);
//    IR2_OPND addr_opnd1 = ir2_opnd_new(IR2_OPND_IREG, ir2_opnd_base_reg_num(&mem_opnd1));
//    /* ir2_opnd1:  IR2_OPND_MEM 
//     * mem_opnd1:  IR2_OPND_MEM
//     * addr_opnd1: IR2_OPND_IREG */
//    append_ir2_opnd3(mips_or, &arg2_ir2_opnd, &addr_opnd1, &zero_ir2_opnd);
//
//    append_ir2_opnd1_not_nop(mips_jalr, &func_addr_opnd);
//    /* 3. arg2: CPUX86State *env */
//    append_ir2_opnd3(mips_or, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
//
//    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

    return true;
}

bool translate_xchg(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    /* if two src is the same reg, do nothing */
    if (ir1_opnd_is_gpr(opnd0) && ir1_opnd_is_gpr(opnd1)) {
        if ((ir1_opnd_size(opnd0) ==
             ir1_opnd_size(opnd1)) &&
            (ir1_opnd_base_reg_num(opnd0) ==
             ir1_opnd_base_reg_num(opnd1)) &&
            (ir1_opnd_base_reg_bits_start(opnd0) ==
             ir1_opnd_base_reg_bits_start(opnd1))) {
            return true;
        }
    }

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, UNKNOWN_EXTENSION, false);
    load_ir1_to_ir2(&src_opnd_1, opnd1, UNKNOWN_EXTENSION, false);

    /* Do memory access first for precise exception */
    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&src_opnd_1, opnd0, false);
        store_ir2_to_ir1(&src_opnd_0, opnd1, false);
    } else {
        store_ir2_to_ir1(&src_opnd_0, opnd1, false);
        store_ir2_to_ir1(&src_opnd_1, opnd0, false);
    }

    return true;
}

bool translate_cmpxchg(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR1_OPND *reg_ir1 = NULL;

    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:  reg_ir1 =  &al_ir1_opnd; break;
    case 16: reg_ir1 =  &ax_ir1_opnd; break;
    case 32: reg_ir1 = &eax_ir1_opnd; break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "cmpxchg opnd size %d is unsupported.\n", opnd_size);
        break;
    }

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();
    IR2_OPND eax_opnd = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0,   SIGN_EXTENSION, false);
    load_ir1_to_ir2(&src_opnd_1, opnd1,   SIGN_EXTENSION, false);
    load_ir1_to_ir2(&eax_opnd, reg_ir1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd  = ra_alloc_itemp();
    append_ir2_opnd3(LISA_SUB_D, &dest_opnd, &eax_opnd, &src_opnd_0);

    IR2_OPND label_unequal = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BNE, &src_opnd_0, &eax_opnd, &label_unequal);

    /* equal */
#ifndef CONFIG_BTMMU
    ra_free_temp(&eax_opnd);
#else
    if (!btmmu_enabled() ||
        lsenv->tr_data->curr_tb->btmmu_disabled ||
        !lsenv->tr_data->is_btmmu_ok)
        ra_free_temp(&eax_opnd);
#endif
    store_ir2_to_ir1(&src_opnd_1, opnd0, false); /* might generate exception in softmmu */
    ra_free_temp(&src_opnd_1);
    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd1(LISA_B, &label_exit);
    /* unequal */
    append_ir2_opnd1(LISA_LABEL, &label_unequal);
    store_ir2_to_ir1_gpr(&src_opnd_0, reg_ir1);
    append_ir2_opnd1(LISA_LABEL, &label_exit);

    /* calculate elfags after compare and exchange(store) */
#ifndef CONFIG_BTMMU
    load_ir1_to_ir2(&eax_opnd, reg_ir1, SIGN_EXTENSION, false);
#else
    if (!btmmu_enabled() ||
        lsenv->tr_data->curr_tb->btmmu_disabled ||
        !lsenv->tr_data->is_btmmu_ok)
        load_ir1_to_ir2(&eax_opnd, reg_ir1, SIGN_EXTENSION, false);
#endif
    generate_eflag_calculation(&dest_opnd, &eax_opnd, &src_opnd_0, pir1, true);

    ra_free_temp(&dest_opnd);

    return true;
}

bool translate_monitor(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    IR1_OPND mem;
    IR2_OPND ptr, ptr_no_offset, ptr_no_offset_ireg;

    tr_gen_save_curr_eip(); // garantee precise interupt

    if(ir1_addr_size(pir1)==16)
        ir1_opnd_build_mem(&mem, 8, X86_REG_AX, 0);  // not intends to access this mem location, so size(=8,=16,=32) is not matter?
    else if(ir1_addr_size(pir1)==32)
        ir1_opnd_build_mem(&mem, 8, X86_REG_EAX, 0);  // not intends to access this mem location, so size(=8,=16,=32) is not matter?
    else // ir1_addr_size(pir1)==64
        lsassert(0);

    mem.mem.segment = X86_REG_DS;

    convert_mem_opnd(&ptr, &mem, -1); // type:mem
    int pir_no_offset_new_tmp = 0;
    ptr_no_offset = convert_mem_ir2_opnd_no_offset(&ptr, &pir_no_offset_new_tmp); // type:mem
    ptr_no_offset_ireg = ir2_opnd_mem_get_base(&ptr_no_offset);

    append_ir2_opnd3(LISA_OR, &arg1_ir2_opnd, &ptr_no_offset_ireg, &zero_ir2_opnd);

    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);

    tr_gen_call_to_helper((ADDR)helper_monitor);

    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

    return true;
}

/* End of TB in system-mode */
bool translate_mwait(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) {
        return true;
    }

    tr_gen_save_curr_eip(); // garantee precise interupt

    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd,
            &zero_ir2_opnd, ir1_addr_size(pir1));
    // imm12 is enough for inst size

    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd,
                     &zero_ir2_opnd);

    tr_gen_call_to_helper((ADDR)helper_mwait);

    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

    return true;
}

//=== AMD's 3DNOW! Extension ===//
// from mmx/mem64(op2) to mmx(op1)
// 2 helper will be called: enter_mmx, helper_name
#define translate_amd_3dnow(INST,helper_name) \
bool translate_##INST(IR1_INST *pir1)\
{\
    if (tr_gen_excp_check(pir1)) {\
        return true;\
    }\
    lsassertm(0, "3DNOW! to be implemented in LoongArch.\n"); \
}

#define ASJGDASGBDKASHD \
\
    helper_cfg_t cfg = default_helper_cfg;\
    /* prepare MMX state (XXX: optimize by storing fptt and fptags in the static cpu state) */\
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_enter_mmx, cfg);\
\
    IR1_OPND *dest_ir1_opnd = ir1_get_opnd(pir1, 0);\
    int dest_mmx = dest_ir1_opnd->reg - X86_REG_MM0;\
    lsassertm(0<=dest_mmx&&dest_mmx<8, "MMX register is required.\n");\
    int op1_offset = offsetof(CPUX86State,fpregs[dest_mmx].mmx);\
\
    IR1_OPND *src_ir1_opnd = ir1_get_opnd(pir1, 1);\
    int op2_offset;\
    if(ir1_opnd_type(src_ir1_opnd)==X86_OP_MEM)\
    {\
        /* 64 bit mips reg is able to load a quad-word x86 value*/\
        IR2_OPND temp = ra_alloc_itemp(); \
        load_ir1_to_ir2(&temp, src_ir1_opnd, ZERO_EXTENSION, false);\
        op2_offset = offsetof(CPUX86State,mmx_t0);\
        /* save temp(memory value) to env->mmx_t0 */\
        /* why not support mips sd instruction? */\
        /* append_ir2_opnd2i(mips_sd, &temp, &env_ir2_opnd, lsenv_offset_of_mmx_t0(lsenv));*/\
        append_ir2_opnd2i(mips_sw, &temp, &env_ir2_opnd, lsenv_offset_of_mmx_t0(lsenv));\
        append_ir2_opnd2i(mips_dsrl32, &temp, &temp, 0); /* logic right thift 32 bit */\
        append_ir2_opnd2i(mips_sw, &temp, &env_ir2_opnd, lsenv_offset_of_mmx_t0(lsenv)+4);\
    }\
    else /* X86_OP_REG */\
    {\
        int src_mmx = src_ir1_opnd->reg - X86_REG_MM0;\
        lsassertm(0<=src_mmx&&src_mmx<8, "MMX register is required.\n");\
        op2_offset = offsetof(CPUX86State,fpregs[src_mmx].mmx);\
    }\
	\
    tr_sys_gen_call_to_helper3_u64_cfg((ADDR)helper_##helper_name,\
    (uint64)lsenv->cpu_state+op1_offset, (uint64)lsenv->cpu_state+op2_offset, cfg);\
\
    return true;\
}

translate_amd_3dnow(pi2fw,pi2fw)
translate_amd_3dnow(pi2fd,pi2fd)
translate_amd_3dnow(pf2iw,pf2iw)
translate_amd_3dnow(pf2id,pf2id)
translate_amd_3dnow(pfnacc,pfnacc)
translate_amd_3dnow(pfpnacc,pfpnacc)
translate_amd_3dnow(pfcmpge,pfcmpge)
translate_amd_3dnow(pfmin,pfmin)
translate_amd_3dnow(pfrcp,pfrcp)
translate_amd_3dnow(pfrsqrt,pfrsqrt)
translate_amd_3dnow(pfsub,pfsub)
translate_amd_3dnow(pfadd,pfadd)
translate_amd_3dnow(pfcmpgt,pfcmpgt)
translate_amd_3dnow(pfmax,pfmax)
translate_amd_3dnow(pfrcpit1,movq) /* pfrcpit1; no need to actually increase precision */
translate_amd_3dnow(pfrsqit1,movq) /* pfrsqit1 */
translate_amd_3dnow(pfsubr,pfsubr)
translate_amd_3dnow(pfacc,pfacc)
translate_amd_3dnow(pfcmpeq,pfcmpeq)
translate_amd_3dnow(pfmul,pfmul)
translate_amd_3dnow(pfrcpit2,movq) /* pfrcpit2 */
translate_amd_3dnow(pmulhrw,pmulhrw_mmx)
translate_amd_3dnow(pswapd,pswapd)
translate_amd_3dnow(pavgusb,pavgb_mmx) /* pavgusb */

#undef translate_amd_3dnow
//=== End of AMD's 3DNOW! Extension ===//

#define translate_from_xmmOrmem64_to_mmx64(INST,helper_name) \
bool translate_##INST(IR1_INST *pir1)\
{\
    lsassertm(0, "xmmOrmem64_to_mmx64to be implemented in LoongArch.\n"); \
}

#define FQEYGFVHQHDJQWQ \
    helper_cfg_t cfg = default_helper_cfg;\
    /* prepare MMX state (XXX: optimize by storing fptt and fptags in the static cpu state) */\
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_enter_mmx, cfg);\
\
    IR1_OPND *dest_ir1_opnd = ir1_get_opnd(pir1, 0);\
    int dest_mmx = dest_ir1_opnd->reg - X86_REG_MM0;\
    lsassertm(0<=dest_mmx&&dest_mmx<8, "MMX register is required.\n");\
    int op1_offset = offsetof(CPUX86State,fpregs[dest_mmx].mmx);\
\
    IR1_OPND *src_ir1_opnd = ir1_get_opnd(pir1, 1);\
    int op2_offset;\
    if(ir1_opnd_type(src_ir1_opnd)==X86_OP_MEM)\
    {\
        /* 64 bit mips reg is able to load a quad-word x86 value*/\
        IR2_OPND temp = ra_alloc_itemp(); \
        load_ir1_to_ir2(&temp, src_ir1_opnd, ZERO_EXTENSION, false);\
        op2_offset = offsetof(CPUX86State,xmm_t0);\
        /* save temp(memory value) to env->xmm_t0 */\
        /* why not support mips sd instruction? */\
        /* append_ir2_opnd2i(mips_sd, &temp, &env_ir2_opnd, lsenv_offset_of_xmm_t0(lsenv));*/\
        append_ir2_opnd2i(mips_sw, &temp, &env_ir2_opnd, lsenv_offset_of_xmm_t0(lsenv));\
        append_ir2_opnd2i(mips_dsrl32, &temp, &temp, 0); /* logic right thift 32 bit */\
        append_ir2_opnd2i(mips_sw, &temp, &env_ir2_opnd, lsenv_offset_of_xmm_t0(lsenv)+4);\
    }\
    else /* X86_OP_REG */\
    {\
        int src_xmm = src_ir1_opnd->reg - X86_REG_XMM0;\
        lsassertm(0<=src_xmm&&src_xmm<8, "XMM register is required.\n");\
        op2_offset = offsetof(CPUX86State,xmm_regs[src_xmm]);\
    }\
	\
    tr_sys_gen_call_to_helper3_u64_cfg((ADDR)helper_##helper_name,\
    (uint64)lsenv->cpu_state+op1_offset, (uint64)lsenv->cpu_state+op2_offset, cfg);\
\
    return true;\
}

translate_from_xmmOrmem64_to_mmx64(cvttps2pi,cvttps2pi)

#undef from_xmm_mem_to_mmx

#define translate_from_xmmOrmem128_to_mmx64(INST,helper_name) \
bool translate_##INST(IR1_INST *pir1)\
{\
    lsassertm(0, "xmmOrmem128_to_mmx64 be implemented in LoongArch.\n"); \
}

#define SCKGQHDBQWDNQWD \
    helper_cfg_t cfg = default_helper_cfg;\
    /* prepare MMX state (XXX: optimize by storing fptt and fptags in the static cpu state) */\
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_enter_mmx, cfg);\
\
    IR1_OPND *dest_ir1_opnd = ir1_get_opnd(pir1, 0);\
    int dest_mmx = dest_ir1_opnd->reg - X86_REG_MM0;\
    lsassertm(0<=dest_mmx&&dest_mmx<8, "MMX register is required.\n");\
    int op1_offset = offsetof(CPUX86State,fpregs[dest_mmx].mmx);\
\
    IR1_OPND *src_ir1_opnd = ir1_get_opnd(pir1, 1);\
    int op2_offset;\
    if(ir1_opnd_type(src_ir1_opnd)==X86_OP_MEM)\
    {\
        /* temp_hi[63:0] || temp_lo[63:0] = src[127:0] */\
        IR2_OPND temp_lo = ra_alloc_itemp(); \
        IR2_OPND temp_hi = ra_alloc_itemp(); \
        load_ir1_to_ir2(&temp_lo, src_ir1_opnd, ZERO_EXTENSION, false);\
        load_ir1_to_ir2(&temp_hi, src_ir1_opnd, ZERO_EXTENSION, true);\
        op2_offset = offsetof(CPUX86State,xmm_t0);\
        /* save temp(memory value) to env->xmm_t0 */\
        /* why not support mips sd instruction? */\
        append_ir2_opnd2i(mips_sw, &temp_lo, &env_ir2_opnd, lsenv_offset_of_xmm_t0(lsenv));\
        append_ir2_opnd2i(mips_dsrl32, &temp_lo, &temp_lo, 0); /* logic right thift 32 bit */\
        append_ir2_opnd2i(mips_sw, &temp_lo, &env_ir2_opnd, lsenv_offset_of_xmm_t0(lsenv)+4);\
        append_ir2_opnd2i(mips_sw, &temp_hi, &env_ir2_opnd, lsenv_offset_of_xmm_t0(lsenv)+8);\
        append_ir2_opnd2i(mips_dsrl32, &temp_hi, &temp_hi, 0); /* logic right thift 32 bit */\
        append_ir2_opnd2i(mips_sw, &temp_hi, &env_ir2_opnd, lsenv_offset_of_xmm_t0(lsenv)+12);\
    }\
    else /* X86_OP_REG */\
    {\
        int src_xmm = src_ir1_opnd->reg - X86_REG_XMM0;\
        lsassertm(0<=src_xmm&&src_xmm<8, "XMM register is required.\n");\
        op2_offset = offsetof(CPUX86State,xmm_regs[src_xmm]);\
    }\
	\
    tr_sys_gen_call_to_helper3_u64_cfg((ADDR)helper_##helper_name,\
    (uint64)lsenv->cpu_state+op1_offset, (uint64)lsenv->cpu_state+op2_offset, cfg);\
\
    return true;\
}

translate_from_xmmOrmem128_to_mmx64(cvttpd2pi,cvttpd2pi)

#undef from_xmm_mem_to_mmx

// the helper of popcnt is special!!! which only take one parameter: @tcg-runtime.c: uint32_t HELPER(ctpop_i32)(uint32_t arg)
bool translate_popcnt(IR1_INST *pir1)
{
    lsassertm(0, "popcnt to be implemented in LoongArch.\n");
//    IR1_OPND *dst_ir1 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src_ir1 = ir1_get_opnd(pir1, 1);
//
//    IR2_OPND src = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src, src_ir1, ZERO_EXTENSION, false);
//
//    IR2_OPND func_addr_opnd = ra_alloc_dbt_arg2();
//
//    helper_cfg_t cfg = default_helper_cfg;
//    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
//
//    load_addr_to_ir2(&func_addr_opnd, (ADDR)helper_ctpop_i32);
//    append_ir2_opnd3(mips_or, &arg0_ir2_opnd, &src, &zero_ir2_opnd);
//
//    append_ir2_opnd1_not_nop(mips_jalr, &func_addr_opnd);
//
//    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
//
//    IR2_OPND value = ra_alloc_itemp();
//    append_ir2_opnd3(mips_or, &value, &ret_ir2_opnd, &zero_ir2_opnd);
//    store_ir2_to_ir1(&value, dst_ir1, false);
//
//    generate_eflag_calculation(&value, &src, &zero_ir2_opnd, pir1, false);

    return true;
}

#define translate_CPUID_EXT_SSSE3_SSE41_SSE42_AES_xmm(INST) \
    int dst_xmm = dst->reg - X86_REG_XMM0;\
    op1_offset = offsetof(CPUX86State,xmm_regs[dst_xmm]);\
    if(!ir1_opnd_is_mem(src))\
    {\
        int src_xmm = src->reg - X86_REG_XMM0;\
        op2_offset = offsetof(CPUX86State,xmm_regs[src_xmm]);\
    }\
    else\
    {\
        op2_offset = offsetof(CPUX86State,xmm_t0);\
        IR2_OPND temp = ra_alloc_itemp(); \
        load_ir1_to_ir2(&temp, src, ZERO_EXTENSION, false);\
        switch(ir1_opcode(pir1)){\
        case X86_INS_PMOVSXBW: case X86_INS_PMOVZXBW:\
        case X86_INS_PMOVSXWD: case X86_INS_PMOVZXWD:\
        case X86_INS_PMOVSXDQ: case X86_INS_PMOVZXDQ:\
            append_ir2_opnd2i(mips_sdi, &temp, &env_ir2_opnd,\
                lsenv_offset_of_xmm_t0(lsenv)+offsetof(ZMMReg, ZMM_Q(0)));\
            break;\
        case X86_INS_PMOVSXBD: case X86_INS_PMOVZXBD:\
        case X86_INS_PMOVSXWQ: case X86_INS_PMOVZXWQ:\
            append_ir2_opnd2i(mips_sw, &temp, &env_ir2_opnd,\
                lsenv_offset_of_mmx_t0(lsenv)+offsetof(ZMMReg, ZMM_L(0)));\
            break;\
        case X86_INS_PMOVSXBQ: case X86_INS_PMOVZXBQ:\
            append_ir2_opnd2i(mips_sh, &temp, &env_ir2_opnd,\
                lsenv_offset_of_mmx_t0(lsenv)+offsetof(ZMMReg, ZMM_W(0)));\
            break;\
        /* case X86_INS_MOVNTQDA:break; xqm doesn't have translate_movntqda*/\
        default:\
            append_ir2_opnd2i(mips_sdi, &temp, &env_ir2_opnd,\
                lsenv_offset_of_mmx_t0(lsenv));\
            load_ir1_to_ir2(&temp, src, ZERO_EXTENSION, true); \
            append_ir2_opnd2i(mips_sdi, &temp, &env_ir2_opnd,\
                lsenv_offset_of_mmx_t0(lsenv)+8);\
        }\
    }\
    tr_sys_gen_call_to_helper3_u64_cfg((ADDR)helper_##INST##_xmm,\
        (uint64)lsenv->cpu_state+op1_offset, (uint64)lsenv->cpu_state+op2_offset, cfg);

// the operand is either xmm or mmx, comparing with translate_CPUID_EXT_SSE41_SSE42_AES
#define translate_CPUID_EXT_SSSE3(INST) \
bool translate_##INST(IR1_INST *pir1)\
{\
    if (tr_gen_excp_check(pir1)) {\
        return true;\
    }\
    lsassertm(0, "CPUID_EXT_SSSE3 to be implemented in LoongArch.\n"); \
}

#define ASJCVAQHDGQJHWVDJQW \
\
    helper_cfg_t cfg = default_helper_cfg;\
\
    IR1_OPND *dst = ir1_get_opnd(pir1, 0);\
    IR1_OPND *src = ir1_get_opnd(pir1, 1);\
\
    int op1_offset, op2_offset;\
    if(ir1_opnd_is_xmm(dst) || ir1_opnd_is_xmm(src))\
    {\
        translate_CPUID_EXT_SSSE3_SSE41_SSE42_AES_xmm(INST)\
    }\
    else /* mmx */\
    {\
        tr_sys_gen_call_to_helper1_cfg((ADDR)helper_enter_mmx, cfg);\
\
        int dst_mmx = dst->reg - X86_REG_MM0;\
        op1_offset = offsetof(CPUX86State,fpregs[dst_mmx].mmx);\
        if(!ir1_opnd_is_mem(src))\
        {\
            int src_mmx = src->reg - X86_REG_MM0;\
            op2_offset = offsetof(CPUX86State,fpregs[src_mmx].mmx);\
        }\
        else\
        {\
            op2_offset = offsetof(CPUX86State,mmx_t0);\
            IR2_OPND temp = ra_alloc_itemp(); \
            load_ir1_to_ir2(&temp, src, ZERO_EXTENSION, false);\
            append_ir2_opnd2i(mips_sdi, &temp, &env_ir2_opnd, lsenv_offset_of_mmx_t0(lsenv));\
        }\
        tr_sys_gen_call_to_helper3_u64_cfg((ADDR)helper_##INST##_mmx,\
            (uint64)lsenv->cpu_state+op1_offset, (uint64)lsenv->cpu_state+op2_offset, cfg);\
    }\
\
    return true;\
}

translate_CPUID_EXT_SSSE3(pshufb)
translate_CPUID_EXT_SSSE3(phaddw)
translate_CPUID_EXT_SSSE3(phaddd)
translate_CPUID_EXT_SSSE3(phaddsw)
translate_CPUID_EXT_SSSE3(pmaddubsw)
translate_CPUID_EXT_SSSE3(phsubw)
translate_CPUID_EXT_SSSE3(phsubd)
translate_CPUID_EXT_SSSE3(phsubsw)
translate_CPUID_EXT_SSSE3(psignb)
translate_CPUID_EXT_SSSE3(psignw)
translate_CPUID_EXT_SSSE3(psignd)
translate_CPUID_EXT_SSSE3(pmulhrsw)
translate_CPUID_EXT_SSSE3(pabsb)
translate_CPUID_EXT_SSSE3(pabsw)
translate_CPUID_EXT_SSSE3(pabsd)

#undef translate_CPUID_EXT_SSSE3

// the operand is xmm, comparing with translate_CPUID_EXT_SSSE3
#define translate_CPUID_EXT_SSE41_SSE42_AES(INST) \
bool translate_##INST(IR1_INST *pir1)\
{\
    if (tr_gen_excp_check(pir1)) {\
        return true;\
    }\
    lsassertm(0, "CPUID_EXT_SSE41_SSE42_AES to be implemented in LoongArch.\n"); \
}

#define SHCGAUHWGDBQNWDBQWJDGQW \
\
    helper_cfg_t cfg = default_helper_cfg;\
\
    IR1_OPND *dst = ir1_get_opnd(pir1, 0);\
    IR1_OPND *src = ir1_get_opnd(pir1, 1);\
\
    int op1_offset, op2_offset;\
\
    translate_CPUID_EXT_SSSE3_SSE41_SSE42_AES_xmm(INST)\
\
    return true;\
}

translate_CPUID_EXT_SSE41_SSE42_AES(pblendvb)
translate_CPUID_EXT_SSE41_SSE42_AES(blendvps)
translate_CPUID_EXT_SSE41_SSE42_AES(blendvpd)
translate_CPUID_EXT_SSE41_SSE42_AES(ptest)
translate_CPUID_EXT_SSE41_SSE42_AES(pmovsxbw)
translate_CPUID_EXT_SSE41_SSE42_AES(pmovsxbd)
translate_CPUID_EXT_SSE41_SSE42_AES(pmovsxbq)
translate_CPUID_EXT_SSE41_SSE42_AES(pmovsxwd)
translate_CPUID_EXT_SSE41_SSE42_AES(pmovsxwq)
translate_CPUID_EXT_SSE41_SSE42_AES(pmovsxdq)
translate_CPUID_EXT_SSE41_SSE42_AES(pmuldq)
translate_CPUID_EXT_SSE41_SSE42_AES(pcmpeqq)
translate_CPUID_EXT_SSE41_SSE42_AES(packusdw)
translate_CPUID_EXT_SSE41_SSE42_AES(pmovzxbw)
translate_CPUID_EXT_SSE41_SSE42_AES(pmovzxbd)
translate_CPUID_EXT_SSE41_SSE42_AES(pmovzxbq)
translate_CPUID_EXT_SSE41_SSE42_AES(pmovzxwd)
translate_CPUID_EXT_SSE41_SSE42_AES(pmovzxwq)
translate_CPUID_EXT_SSE41_SSE42_AES(pmovzxdq)
translate_CPUID_EXT_SSE41_SSE42_AES(pminsb)
translate_CPUID_EXT_SSE41_SSE42_AES(pminsd)
translate_CPUID_EXT_SSE41_SSE42_AES(pminuw)
translate_CPUID_EXT_SSE41_SSE42_AES(pminud)
translate_CPUID_EXT_SSE41_SSE42_AES(pmaxsb)
translate_CPUID_EXT_SSE41_SSE42_AES(pmaxsd)
translate_CPUID_EXT_SSE41_SSE42_AES(pmaxuw)
translate_CPUID_EXT_SSE41_SSE42_AES(pmaxud)
translate_CPUID_EXT_SSE41_SSE42_AES(pmulld)
translate_CPUID_EXT_SSE41_SSE42_AES(phminposuw)
translate_CPUID_EXT_SSE41_SSE42_AES(pcmpgtq)
translate_CPUID_EXT_SSE41_SSE42_AES(aesimc)
translate_CPUID_EXT_SSE41_SSE42_AES(aesenc)
translate_CPUID_EXT_SSE41_SSE42_AES(aesenclast)
translate_CPUID_EXT_SSE41_SSE42_AES(aesdec)
translate_CPUID_EXT_SSE41_SSE42_AES(aesdeclast)

#undef translate_CPUID_EXT_SSE41_SSE42_AES

#undef translate_CPUID_EXT_SSSE3_SSE41_SSE42_AES_xmm

bool translate_crc32(IR1_INST *pir1)
{
    lsassertm(0, "crc32 to be implemented in LoongArch.\n");
//    if (tr_gen_excp_check(pir1)) {
//        return true;
//    }
//    IR1_OPND *src_and_dst_ir1 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src_ir1 = ir1_get_opnd(pir1, 1);
//
//    IR2_OPND src_and_dst = ra_alloc_itemp();
//    IR2_OPND src = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_and_dst, src_and_dst_ir1, ZERO_EXTENSION, false);
//    load_ir1_to_ir2(&src, src_ir1, ZERO_EXTENSION, false);
//
//    IR2_OPND func_addr_opnd = ra_alloc_dbt_arg2();
//
//    helper_cfg_t cfg = default_helper_cfg;
//    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
//
//    // target_ulong helper_crc32(uint32_t crc1, target_ulong msg, uint32_t len)
//    load_addr_to_ir2(&func_addr_opnd, (ADDR)helper_crc32);
//    append_ir2_opnd2(mips_mov32_zx, &arg0_ir2_opnd, &src_and_dst);
//    append_ir2_opnd2(mips_mov64, &arg1_ir2_opnd, &src);
//    load_imm64_to_ir2(&arg2_ir2_opnd, ir1_data_size(pir1));
//
//    append_ir2_opnd1_not_nop(mips_jalr, &func_addr_opnd);
//
//    append_ir2_opnd2(mips_mov64, &src_and_dst, &ret0_ir2_opnd);
//
//    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

    return true;
}

// qemu sse_op_table7
#define translate_CPUID_EXT_SSSE3_SSE41_SSE42_PCLMULQDQ_AESNI_xmm(INST) \
    int dst_xmm = dst->reg - X86_REG_XMM0;\
    op1_offset = offsetof(CPUX86State,xmm_regs[dst_xmm]);\
    if(!ir1_opnd_is_mem(src))\
    {\
        int src_xmm = src->reg - X86_REG_XMM0;\
        op2_offset = offsetof(CPUX86State,xmm_regs[src_xmm]);\
    }\
    else\
    {\
        op2_offset = offsetof(CPUX86State,xmm_t0);\
        IR2_OPND temp = ra_alloc_itemp(); \
        load_ir1_to_ir2(&temp, src, ZERO_EXTENSION, false);\
        append_ir2_opnd2i(mips_sdi, &temp, &env_ir2_opnd,\
            lsenv_offset_of_mmx_t0(lsenv));\
        load_ir1_to_ir2(&temp, src, ZERO_EXTENSION, true); \
        append_ir2_opnd2i(mips_sdi, &temp, &env_ir2_opnd,\
            lsenv_offset_of_mmx_t0(lsenv)+8);\
    }\
    tr_sys_gen_call_to_helper4_u64_cfg((ADDR)helper_##INST##_xmm,\
        (uint64)lsenv->cpu_state+op1_offset, (uint64)lsenv->cpu_state+op2_offset,\
        ir1_get_opnd(pir1, 2)->imm, cfg);

// the operand is either xmm or mmx, comparing with translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI
#define translate_CPUID_EXT_SSSE3(INST) \
bool translate_##INST(IR1_INST *pir1)\
{\
    if (tr_gen_excp_check(pir1)) {\
        return true;\
    }\
    lsassertm(0, "CPUID_EXT_SSSE3 to be implemented in LoongArch.\n"); \
}

#define SKCVQWDGVQJWKNDKJQABDKJQ \
\
    helper_cfg_t cfg = default_helper_cfg;\
\
    IR1_OPND *dst = ir1_get_opnd(pir1, 0);\
    IR1_OPND *src = ir1_get_opnd(pir1, 1);\
\
    int op1_offset, op2_offset;\
    if(ir1_opnd_is_xmm(dst) || ir1_opnd_is_xmm(src))\
    {\
        translate_CPUID_EXT_SSSE3_SSE41_SSE42_PCLMULQDQ_AESNI_xmm(INST)\
    }\
    else /* mmx */\
    {\
        tr_sys_gen_call_to_helper1_cfg((ADDR)helper_enter_mmx, cfg);\
\
        int dst_mmx = dst->reg - X86_REG_MM0;\
        op1_offset = offsetof(CPUX86State,fpregs[dst_mmx].mmx);\
        if(!ir1_opnd_is_mem(src))\
        {\
            int src_mmx = src->reg - X86_REG_MM0;\
            op2_offset = offsetof(CPUX86State,fpregs[src_mmx].mmx);\
        }\
        else\
        {\
            op2_offset = offsetof(CPUX86State,mmx_t0);\
            IR2_OPND temp = ra_alloc_itemp(); \
            load_ir1_to_ir2(&temp, src, ZERO_EXTENSION, false);\
            append_ir2_opnd2i(mips_sdi, &temp, &env_ir2_opnd, lsenv_offset_of_mmx_t0(lsenv));\
        }\
        tr_sys_gen_call_to_helper4_u64_cfg((ADDR)helper_##INST##_mmx,\
            (uint64)lsenv->cpu_state+op1_offset, (uint64)lsenv->cpu_state+op2_offset,\
            ir1_get_opnd(pir1, 2)->imm, cfg);\
    }\
\
    return true;\
}

translate_CPUID_EXT_SSSE3(palignr)

#undef translate_CPUID_EXT_SSSE3

// the operand is xmm, comparing with translate_CPUID_EXT_SSSE3
#define translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(INST) \
bool translate_##INST(IR1_INST *pir1)\
{\
    if (tr_gen_excp_check(pir1)) {\
        return true;\
    }\
    lsassertm(0, "CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI to be implemented in LoongArch.\n"); \
}

#define XCGUHEWKJQNEFK \
\
    helper_cfg_t cfg = default_helper_cfg;\
\
    IR1_OPND *dst = ir1_get_opnd(pir1, 0);\
    IR1_OPND *src = ir1_get_opnd(pir1, 1);\
\
    int op1_offset, op2_offset;\
\
    translate_CPUID_EXT_SSSE3_SSE41_SSE42_PCLMULQDQ_AESNI_xmm(INST)\
\
    return true;\
}

translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(roundps)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(roundpd)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(roundss)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(roundsd)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(blendps)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(blendpd)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(pblendw)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(dpps)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(dppd)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(mpsadbw)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(pclmulqdq)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(pcmpestrm)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(pcmpestri)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(pcmpistrm)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(pcmpistri)
translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI(aeskeygenassist)

#undef translate_CPUID_EXT_SSE41_SSE42_PCLMULQDQ_AESNI

#undef translate_CPUID_EXT_SSSE3_SSE41_SSE42_PCLMULQDQ_AESNI_xmm
// end of qemu sse_op_table7

bool translate_invalid(IR1_INST *pir1)
{
    switch (pir1->flags) {
        case IR1_FLAGS_ILLOP:
            tr_gen_excp_illegal_op_addr(ir1_addr(pir1) - lsenv->tr_data->sys.cs_base, 1);
            break;
        case IR1_FLAGS_GENNOP:
            /* Just do nothing */
            break;
        default:
            lsassertm(0, "unsupport invalid ir1 falgs %d\n", pir1->flags);
            break;
    }

    return true;
};

bool translate_ud0(IR1_INST *pir1)
{
    tr_gen_excp_illegal_op_addr(
            ir1_addr(pir1) - lsenv->tr_data->sys.cs_base, /* EIP */
            1 /* translation end with exception */
    );

    return true;
}
