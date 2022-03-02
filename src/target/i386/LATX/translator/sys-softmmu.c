#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

static int is_ldst_realized_by_softmmu(IR2_OPCODE op)
{
    if (option_fast_fpr_ldst) {
        switch (op) {
        default:
        case LISA_VLD:
        case LISA_VST:
        case LISA_FLD_S:
        case LISA_FLD_D:
        case LISA_FST_S:
        case LISA_FST_D:
            return true;
        }
    }
    switch (op) {
    case LISA_LD_B:
    case LISA_LD_BU:
    case LISA_LD_H:
    case LISA_LD_HU:
    case LISA_LD_W:
    case LISA_LD_WU:
    case LISA_LD_D:
    case LISA_ST_B:
    case LISA_ST_H:
    case LISA_ST_W:
    case LISA_ST_D:
        return 1;
    default:
        return 0;
    }
}

static int convert_to_tcgmemop(IR2_OPCODE op)
{
    switch (op) {
    case LISA_LD_B:
        return MO_SB;
    case LISA_LD_BU:
    case LISA_ST_B:
        return MO_UB;
    case LISA_LD_H:
        return MO_LESW;
    case LISA_LD_HU:
    case LISA_ST_H:
        return MO_LEUW;
    case LISA_LD_W:
        return MO_LESL;
    case LISA_FLD_S:
    case LISA_FST_S:
    case LISA_LD_WU:
    case LISA_ST_W:
        return MO_LEUL;
    case LISA_FLD_D:
    case LISA_FST_D:
    case LISA_LD_D:
    case LISA_ST_D:
        return MO_LEQ;
    case LISA_VLD:
    case LISA_VST:
        return MO_LEQ;
    default:
        lsassertm(0, "not support IR2 %d in convert_to_tcgmemop.\n", op);
        return -1;
    }
}

static int get_ldst_align_bits(IR2_OPCODE opc)
{
    if (option_fast_fpr_ldst) {
        if (opc == LISA_VLD || opc == LISA_VST) {
            return 4;
        }
        if (opc == LISA_FLD_S || opc == LISA_FST_S) {
            return 2;
        }
        if (opc == LISA_FLD_D || opc == LISA_FST_D) {
            return 3;
        }
    }
    switch (opc) {
    case LISA_LD_B:
    case LISA_LD_BU:
    case LISA_ST_B:
        return 0; /* MemOp & MO_SIZE == MO_8  == 0 */
    case LISA_LD_H:
    case LISA_LD_HU:
    case LISA_ST_H:
        return 1; /* MemOp & MO_SIZE == MO_16 == 1 */
    case LISA_LD_W:
    case LISA_LD_WU:
    case LISA_ST_W:
        return 2; /* MemOp & MO_SIZE == MO_32 == 2 */
    case LISA_LD_D:
    case LISA_ST_D:
        return 3; /* MemOp & MO_SIZE == MO_64 == 3 */
    default:
        return -1; /* no support for unaligned access */
    }
}

/* return label point to slow path */
static void tr_gen_lookup_qemu_tlb(
        IR2_OPCODE op,
        IR2_OPND *gpr_opnd,
        IR2_OPND *mem,
        int mmu_index,
        bool is_load,
        IR2_OPND label_slow_path)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    /* offset from ENV */
    int fast_off  = TLB_MASK_TABLE_OFS(mmu_index);
    int mask_off  = fast_off + offsetof(CPUTLBDescFast, mask);  /* 64-bit */
    int table_off = fast_off + offsetof(CPUTLBDescFast, table); /* 64-bit */
    /* offset from tlb entry */
    int add_off   = offsetof(CPUTLBEntry, addend); /* 64-bit */
    int tag_off   = (is_load ? offsetof(CPUTLBEntry, addr_read)
                   : offsetof(CPUTLBEntry, addr_write)); /* 32-bit */

    /* 1. load f[mmu].mask */
#ifndef TARGET_X86_64
    IR2_OPND mask_opnd = latxs_ra_alloc_itemp();
#else
    IR2_OPND mask_opnd = latxs_arg0_ir2_opnd;
#endif
    if (int32_in_int12(mask_off)) {
        latxs_append_ir2_opnd2i(LISA_LD_D, &mask_opnd, env, mask_off);
    } else {
        latxs_load_imm32_to_ir2(&mask_opnd, mask_off, EXMode_S);
        latxs_append_ir2_opnd3(LISA_ADD_D, &mask_opnd, env, &mask_opnd);
        latxs_append_ir2_opnd2i(LISA_LD_D, &mask_opnd, &mask_opnd, 0);
    }

    /* 2. extract tlb index */
#ifndef TARGET_X86_64
    IR2_OPND index_opnd = latxs_ra_alloc_itemp();
#else
    IR2_OPND index_opnd = latxs_arg1_ir2_opnd;
#endif
    /* 2.1 index = address >> shift */
    int shift = TARGET_PAGE_BITS - CPU_TLB_ENTRY_BITS;
    latxs_append_ir2_opnd2i(LISA_SRLI_W, &index_opnd, mem, shift);

    /* 2.2 index = index & mask */
    latxs_append_ir2_opnd3(LISA_AND, &index_opnd, &index_opnd, &mask_opnd);
#ifndef TARGET_X86_64
    latxs_ra_free_temp(&mask_opnd);
#endif

    /* 3. load f[mmu].table */
#ifndef TARGET_X86_64
    IR2_OPND table_opnd = latxs_ra_alloc_itemp();
#else
    IR2_OPND table_opnd = latxs_arg2_ir2_opnd;
#endif
    if (int32_in_int12(table_off)) {
        latxs_append_ir2_opnd2i(LISA_LD_D, &table_opnd, env, table_off);
    } else {
        latxs_load_imm32_to_ir2(&table_opnd, table_off, EXMode_S);
        latxs_append_ir2_opnd3(LISA_ADD_D, &table_opnd, env, &table_opnd);
        latxs_append_ir2_opnd2i(LISA_LD_D, &table_opnd, &table_opnd, 0);
    }

    /* 4. tlb entry = table + index */
    latxs_append_ir2_opnd3(LISA_ADD_D, &table_opnd, &table_opnd, &index_opnd);
    IR2_OPND tlb_opnd = table_opnd;
#ifndef TARGET_X86_64
    latxs_ra_free_temp(&index_opnd);
#endif

    /* 5. load compare part from tlb entry */
#ifndef TARGET_X86_64
    IR2_OPND tag_opnd = latxs_ra_alloc_itemp();
#else
    IR2_OPND tag_opnd = latxs_arg0_ir2_opnd;
#endif
    if (TARGET_LONG_BITS == 32) {
        latxs_append_ir2_opnd2i(LISA_LD_WU, &tag_opnd, &tlb_opnd, tag_off);
    } else {
        latxs_append_ir2_opnd2i(LISA_LD_D, &tag_opnd, &tlb_opnd, tag_off);
    }
    /* 6. cmp = vaddr & mask */
    /*
     *                      12 11 8 7  4 3  0
     * vaddr 0x 1111 .... 1111 0000 0000 0000
     *                       ^              \- align_bits = 0 ; B  8-bits
     *                       |                              1 ; H 16-bits
     *          i386.TARGET_PAGE_BITS = 12                  2 ; W 32-bits
     *                                                      3 ; D 64-bits
     */
    int align_bits = get_ldst_align_bits(op);
#ifndef TARGET_X86_64
    IR2_OPND cmp_opnd = latxs_ra_alloc_itemp();
#else
    IR2_OPND cmp_opnd = latxs_arg1_ir2_opnd;
#endif
    latxs_append_ir2_opnd2_(lisa_mov, &cmp_opnd, mem);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &cmp_opnd, zero,
            TARGET_PAGE_BITS - 1, align_bits);

    /* 6.5 do something before tlb compare */
    if (option_native_printer) {
        latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
                zero, LATXS_NP_TLBCMP);

        latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg2_ir2_opnd,
                zero, latxs_ir2_opnd_reg(&cmp_opnd));
        latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg3_ir2_opnd,
                zero, latxs_ir2_opnd_reg(&tag_opnd));
        /* latxs_arg4/5/6_ir2_opnd is itemp in x86_64, please be careful */
        latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg4_ir2_opnd,
                zero, latxs_ir2_opnd_reg(mem));
        latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg5_ir2_opnd,
                zero, mmu_index);
        latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg6_ir2_opnd,
                zero, is_load);

        TRANSLATION_DATA *td = lsenv->tr_data;
        TranslationBlock *tb = td->curr_tb;
        ADDR code_buf = (ADDR)tb->tc.ptr;
        int offset = td->real_ir2_inst_num << 2;

        int64_t ins_offset = (latxs_native_printer - code_buf - offset) >> 2;
        latxs_append_ir2_jmp_far(ins_offset, 1);
    }

    /* 7. compare cmp and tag */
    if (!option_smmu_slow) {
        latxs_append_ir2_opnd3(LISA_BNE,
                &cmp_opnd, &tag_opnd, &label_slow_path);
    } else {
        /* Always jump to Softmmu Slow Path : mainly for debug */
        latxs_append_ir2_opnd3(LISA_BEQ,
                zero, zero, &label_slow_path);
    }
#ifndef TARGET_X86_64
    latxs_ra_free_temp(&tag_opnd);
    latxs_ra_free_temp(&cmp_opnd);
#endif

    /* 8. load addend from tlb entry */
#ifndef TARGET_X86_64
    IR2_OPND add_opnd = latxs_ra_alloc_itemp();
#else
    IR2_OPND add_opnd = latxs_arg1_ir2_opnd;
#endif
    latxs_append_ir2_opnd2i(LISA_LD_D, &add_opnd, &tlb_opnd, add_off);
#ifndef TARGET_X86_64
    latxs_ra_free_temp(&tlb_opnd);
#endif

    /* 9. get hvaddr if not branch */
    switch (op) {
    case LISA_LD_B:
        latxs_append_ir2_opnd3(LISA_LDX_B, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_LD_H:
        latxs_append_ir2_opnd3(LISA_LDX_H, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_LD_W:
        latxs_append_ir2_opnd3(LISA_LDX_W, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_LD_D:
        latxs_append_ir2_opnd3(LISA_LDX_D, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_LD_BU:
        latxs_append_ir2_opnd3(LISA_LDX_BU, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_LD_HU:
        latxs_append_ir2_opnd3(LISA_LDX_HU, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_LD_WU:
        latxs_append_ir2_opnd3(LISA_LDX_WU, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_ST_B:
        latxs_append_ir2_opnd3(LISA_STX_B, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_ST_H:
        latxs_append_ir2_opnd3(LISA_STX_H, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_ST_W:
        latxs_append_ir2_opnd3(LISA_STX_W, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_ST_D:
        latxs_append_ir2_opnd3(LISA_STX_D, gpr_opnd, mem, &add_opnd);
        break;
    /* option_fast_fpr_ldst */
    case LISA_VLD:
        latxs_append_ir2_opnd3(LISA_VLDX, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_VST:
        latxs_append_ir2_opnd3(LISA_VSTX, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_FLD_S:
        latxs_append_ir2_opnd3(LISA_FLDX_S, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_FLD_D:
        latxs_append_ir2_opnd3(LISA_FLDX_D, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_FST_S:
        latxs_append_ir2_opnd3(LISA_FSTX_S, gpr_opnd, mem, &add_opnd);
        break;
    case LISA_FST_D:
        latxs_append_ir2_opnd3(LISA_FSTX_D, gpr_opnd, mem, &add_opnd);
        break;
    default:
        lsassertm(0, "wrong in softmmu\n");
        break;
    }
#ifndef TARGET_X86_64
    latxs_ra_free_temp(&add_opnd);
#endif
}

static void td_rcd_softmmu_slow_path(
        IR2_OPCODE op,
        IR2_OPND *gpr_ir2_opnd,
        IR2_OPND *mem_ir2_opnd,
        IR2_OPND *label_slow_path,
        IR2_OPND *label_exit,
        int mmu_index,
        int save_tmp,
        int is_load)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    int sp_nr  = td->slow_path_rcd_nr;
    int sp_max = td->slow_path_rcd_max;

    /* get Slow Path Record entry */

    softmmu_sp_rcd_t *sp = NULL;

    if (sp_nr == sp_max) {
        td->slow_path_rcd_max = sp_max << 1;
        sp_max = sp_max << 1;
        sp = mm_realloc(td->slow_path_rcd,
                sp_max * sizeof(softmmu_sp_rcd_t));
        td->slow_path_rcd = sp;
        sp = &sp[sp_nr];
    } else {
        softmmu_sp_rcd_t *sp_array = td->slow_path_rcd;
        sp = (void *)(&sp_array[sp_nr]);
    }

    td->slow_path_rcd_nr += 1;

    /* save information about slow path */
    sp->op = op;
    sp->gpr_ir2_opnd = *gpr_ir2_opnd;
    sp->mem_ir2_opnd = *mem_ir2_opnd;
    sp->label_slow_path = *label_slow_path;
    sp->label_exit = *label_exit;
    sp->mmu_index = mmu_index;
    sp->tmp_need_save = save_tmp;
    sp->is_load = is_load;

    sp->retaddr = (ADDR)(tb->tc.ptr) +
                  (ADDR)((td->real_ir2_inst_num - 2) << 2);

    if (!option_lsfpu && !option_soft_fpu) {
        sp->fpu_top = latxs_td_fpu_get_top();
    }

    sp->itmp_mask = td->itemp_mask;
    sp->ftmp_mask = td->ftemp_mask;

    if (option_by_hand) {
        int i = 0;
        for (i = 0; i < CPU_NB_REGS; ++i) {
            sp->reg_exmode[i] = td->reg_exmode[i];
            sp->reg_exbits[i] = td->reg_exbits[i];
        }
    }
}

static void tr_gen_ldst_slow_path(
        IR2_OPCODE op,
        IR2_OPND  *gpr_opnd, /* temp(t0-t7) or mapping(s1-s8) */
        IR2_OPND  *mem_opnd,
        IR2_OPND  *label_slow_path,
        IR2_OPND  *label_exit,
        int        mmu_index,
        int        is_load,
        int        save_temp)
{
    td_rcd_softmmu_slow_path(
            op, gpr_opnd, mem_opnd,
            label_slow_path, label_exit,
            mmu_index, save_temp, is_load);
}


/* option_fast_fpr_ldst : opnd_gpr can be fpr */
static void __gen_ldst_softmmu_helper_native(
        IR2_OPCODE op,
        IR2_OPND *opnd_gpr,
        IR2_OPND *opnd_mem,
        int save_temp)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    int mmu_index = td->sys.mem_index;
    bool is_load  = latxs_ir2_opcode_is_load(op);

    /*
     * 0. format the mem opnd
     */
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = latxs_convert_mem_ir2_opnd_no_offset(opnd_mem,
            &mem_no_offset_new_tmp);
    IR2_OPND base_only_mem = latxs_ir2_opnd_mem_get_base(&mem_no_offset);
#ifdef TARGET_X86_64
    /* TODO: addr_size */
    if (!lsenv->tr_data->sys.code64) {
        latxs_append_ir2_opnd2_(lisa_mov32z, &base_only_mem, &base_only_mem);
    }
#else
    if (mem_no_offset_new_tmp) {
        latxs_append_ir2_opnd2_(lisa_mov32z, &base_only_mem, &base_only_mem);
    }
#endif

    /*
     * 1. lookup QEMU TLB in native code
     *    and finish memory access if TLB hit
     *    temp register is free to use inside.
     */
    IR2_OPND label_slow_path = latxs_ir2_opnd_new_label();
    tr_gen_lookup_qemu_tlb(op, opnd_gpr, &base_only_mem, mmu_index,
                           is_load, label_slow_path);

    /* 2. memory access finish. jump slow path. */
    IR2_OPND label_exit = latxs_ir2_opnd_new_label();

    /*
     * 3. slow path : call QEMU's helper
     *    Here we just record the data to generate slow path
     *    The real slow path will be generated at the end of TB
     */
    tr_gen_ldst_slow_path(op, opnd_gpr, &base_only_mem,
            &label_slow_path, &label_exit,
            mmu_index, is_load, save_temp);

    /* 4. exit from fast path or return from slow path */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

    if (mem_no_offset_new_tmp) {
        latxs_ra_free_temp(&mem_no_offset);
    }
}

static void __tr_gen_softmmu_sp_rcd(softmmu_sp_rcd_t *sp)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    IR2_OPND *arg0 = &latxs_arg0_ir2_opnd;
    IR2_OPND *arg1 = &latxs_arg1_ir2_opnd;
    IR2_OPND *arg2 = &latxs_arg2_ir2_opnd;
    IR2_OPND *arg3 = &latxs_arg3_ir2_opnd;
    /* latxs_arg4_ir2_opnd is itemp in x86_64, please be careful */
    IR2_OPND *arg4 = &latxs_arg4_ir2_opnd;

    latxs_append_ir2_opnd1(LISA_LABEL, &sp->label_slow_path);

    TRANSLATION_DATA *td = lsenv->tr_data;
    int itmp_mask_bak = td->itemp_mask;
    int ftmp_mask_bak = td->ftemp_mask;
    td->itemp_mask = sp->itmp_mask;
    td->ftemp_mask = sp->ftmp_mask;

    int top_bak = 0;
    if (!option_lsfpu && !option_soft_fpu) {
        top_bak = latxs_td_fpu_get_top();
        latxs_td_fpu_set_top(sp->fpu_top);
    }

    if (sp->tmp_need_save) {
        latxs_tr_save_temp_register_mask(sp->itmp_mask);
    }

    EXMode gpr_em_bak[CPU_NB_REGS];
    EXBits gpr_eb_bak[CPU_NB_REGS];

    int i = 0;

    /* option_fast_fpr_ldst : mov fpr to ftemp, avoid top mode */
    IR2_OPND ftemp = latxs_zero_ir2_opnd;
    if (sp->op == LISA_FST_S || sp->op == LISA_FST_D) {
        ftemp = latxs_ra_alloc_ftemp();
        latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &latxs_ra_ir2_opnd,
                &sp->gpr_ir2_opnd);
        latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &ftemp,
                &latxs_ra_ir2_opnd);
    }

    /* 1. save native context */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    /* 2. prepare arguments for softmmu helper */

    /*
     * build arguemnts for the helper
     *               LOAD     |  STORE
     * ---------------------------------------
     * helper arg0 : env      |  env
     * helper arg1 : x86vaddr |  x86vaddr
     * helper arg2 : memop    |  data
     * helper arg3 : retaddr  |  memop
     * helper arg4 :          |  retaddr
     */

    IR2_OPND mem = sp->mem_ir2_opnd;
    MemOp memop = convert_to_tcgmemop(sp->op);
    TCGMemOpIdx memopidx = (memop << 4) | sp->mmu_index;

    /* 3.1 arg1: mem address */
    /* latxs_append_ir2_opnd2_(lisa_mov, arg1, &mem); */
#ifdef TARGET_X86_64
    /* TODO: addr_size */
    if (lsenv->tr_data->sys.code64) {
        latxs_append_ir2_opnd2_(lisa_mov, arg1, &mem);
    } else {
        latxs_append_ir2_opnd2_(lisa_mov32z, arg1, &mem);
    }
#else
    latxs_append_ir2_opnd2_(lisa_mov32s, arg1, &mem);
#endif

    /* 3.2 arg0 : env */
    latxs_append_ir2_opnd2_(lisa_mov, arg0, env);
    /* 3.3 arg2 : memop(LOAD) data(STORE) */
    if (sp->is_load) {
        latxs_append_ir2_opnd2i(LISA_ORI, arg2, zero, memopidx);
    } else {
        if (sp->op == LISA_VST) {
            latxs_append_ir2_opnd2i(LISA_VST, &sp->gpr_ir2_opnd,
                                    &latxs_env_ir2_opnd,
                                    offsetof(CPUX86State, temp_xmm));
        } else if (sp->op == LISA_FST_S) {
            /* option_fast_fpr_ldst : data mov to ftemp before prologue */
            latxs_append_ir2_opnd2(LISA_MOVFR2GR_S, arg2, &ftemp);
        } else if (sp->op == LISA_FST_D) {
            latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, arg2, &ftemp);
        } else {
            latxs_append_ir2_opnd2_(lisa_mov, arg2, &sp->gpr_ir2_opnd);
        }
    }
    /* 3.4 arg3 : retaddr(LOAD) memop(STORE) */
    if (sp->is_load) {
        latxs_load_imm64_to_ir2(arg3, sp->retaddr);
    } else {
        latxs_append_ir2_opnd2i(LISA_ORI, arg3, zero, memopidx);
        /* 3.5 arg4 : retaddr(STORE) */
        latxs_load_imm64_to_ir2(arg4, sp->retaddr);
    }

    /* 4. call the helper */
    switch (sp->op) {
    case LISA_ST_B:
        latxs_tr_gen_call_to_helper((ADDR)helper_ret_stb_mmu);
        break;
    case LISA_ST_H:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_stw_mmu);
        break;
    case LISA_FST_S:
    case LISA_ST_W:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_stl_mmu);
        break;
    case LISA_FST_D:
    case LISA_ST_D:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_stq_mmu);
        break;
    case LISA_LD_B:
        latxs_tr_gen_call_to_helper((ADDR)helper_ret_ldsb_mmu);
        break;
    case LISA_LD_BU:
        latxs_tr_gen_call_to_helper((ADDR)helper_ret_ldub_mmu);
        break;
    case LISA_LD_H:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_ldsw_mmu);
        break;
    case LISA_LD_HU:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_lduw_mmu);
        break;
    case LISA_LD_W:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_ldsl_mmu);
        break;
    case LISA_FLD_S:
    case LISA_LD_WU:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_ldul_mmu);
        break;
    case LISA_FLD_D:
    case LISA_LD_D:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_ldq_mmu);
        break;
    case LISA_VLD:
        latxs_tr_gen_call_to_helper((ADDR)latxs_helper_le_lddq_mmu);
        break;
    case LISA_VST:
        latxs_tr_gen_call_to_helper((ADDR)latxs_helper_le_stdq_mmu);
        break;
    default:
        lsassert(0);
        break;
    }

    /* 6. restore context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);

    if (option_by_hand) {
        for (i = 0; i < CPU_NB_REGS; ++i) {
            gpr_em_bak[i] = td->reg_exmode[i];
            gpr_eb_bak[i] = td->reg_exbits[i];
            td->reg_exmode[i] = sp->reg_exmode[i];
            td->reg_exbits[i] = sp->reg_exbits[i];
        }
        /*
         * Since slow path is generated at the end of TB,
         * it does not need to manage GPR's EM like translation.
         *
         * Instead, it need to keep the GPR's EM equal to
         * the original EM stored from fast path.
         */
        latxs_tr_setto_extmb_after_cs(0xFF);
        /*
         * The purpose of save/restore the EM in TRANSLATION_DATA
         * is NOT to manage the GPR's EM. It is mean to make the
         * slow path generation function does not have side effects.
         *
         * This should be good for other modification in the future.
         */
        for (i = 0; i < CPU_NB_REGS; ++i) {
            td->reg_exmode[i] = gpr_em_bak[i];
            td->reg_exbits[i] = gpr_eb_bak[i];
        }
    }

    if (sp->tmp_need_save) {
        latxs_tr_restore_temp_register_mask(sp->itmp_mask);
    }

    if (sp->is_load) {
        if (sp->op == LISA_VLD) {
            latxs_append_ir2_opnd2i(LISA_VLD, &sp->gpr_ir2_opnd,
                                    &latxs_env_ir2_opnd,
                                    offsetof(CPUX86State, temp_xmm));
        } else if (sp->op == LISA_FLD_S) {
            latxs_append_ir2_opnd2(LISA_MOVGR2FR_W, &sp->gpr_ir2_opnd,
                                   &latxs_ret0_ir2_opnd);
        } else if (sp->op == LISA_FLD_D) {
            latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &sp->gpr_ir2_opnd,
                                   &latxs_ret0_ir2_opnd);
        } else {
            latxs_append_ir2_opnd2_(lisa_mov, &sp->gpr_ir2_opnd,
                                    &latxs_ret0_ir2_opnd);
        }
    }

    td->itemp_mask = itmp_mask_bak;
    td->ftemp_mask = ftmp_mask_bak;

    if (!option_lsfpu && !option_soft_fpu) {
        latxs_td_fpu_set_top(top_bak);
    }

    latxs_append_ir2_opnd1(LISA_B, &sp->label_exit);

    /* option_fast_fpr_ldst : free ftemp */
    if (sp->op == LISA_FST_S || sp->op == LISA_FST_D) {
        latxs_ra_free_temp(&ftemp);
    }
}

void tr_gen_softmmu_slow_path(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    td->in_gen_slow_path = 1;

    int i = 0;
    int sp_nr = td->slow_path_rcd_nr;

    softmmu_sp_rcd_t *sp = NULL;
    softmmu_sp_rcd_t *sp_array = td->slow_path_rcd;

    for (i = 0; i < sp_nr; ++i) {
        sp = &sp_array[i];
        __tr_gen_softmmu_sp_rcd(sp);
    }

    td->in_gen_slow_path = 0;
}

void gen_ldst_softmmu_helper(
        IR2_OPCODE op,
        IR2_OPND *opnd_gpr,
        IR2_OPND *opnd_mem,
        int save_temp)
{
    if (is_ldst_realized_by_softmmu(op)) {
        __gen_ldst_softmmu_helper_native(op, opnd_gpr, opnd_mem, save_temp);
        return;
    }

    lsassertm(0, "Softmmu not support non-load/store instruction.");
}

void gen_ldst_c1_softmmu_helper(
        IR2_OPCODE op,
        IR2_OPND *opnd_fpr,
        IR2_OPND *opnd_mem,
        int save_temp)
{
    if (option_fast_fpr_ldst) {
        gen_ldst_softmmu_helper(op, opnd_fpr, opnd_mem, 1);
        return;
    }

    IR2_OPCODE ldst_op = LISA_INVALID;
    IR2_OPCODE mfmt_op = LISA_INVALID;

    switch (op) {
    case LISA_FLD_S:
        ldst_op = LISA_LD_WU;
        mfmt_op = LISA_MOVGR2FR_W;
        break;
    case LISA_FLD_D:
        ldst_op = LISA_LD_D;
        mfmt_op = LISA_MOVGR2FR_D;
        break;
    case LISA_FST_S:
        ldst_op = LISA_ST_W;
        mfmt_op = LISA_MOVFR2GR_S;
        break;
    case LISA_FST_D:
        ldst_op = LISA_ST_D;
        mfmt_op = LISA_MOVFR2GR_D;
        break;
    default:
        lsassert(0);
        break;
    }

    IR2_OPND tmp = latxs_ra_alloc_itemp();
    if (latxs_ir2_opcode_is_load(op)) {
        /* 1. load to GPR from memory */
        gen_ldst_softmmu_helper(ldst_op, &tmp, opnd_mem, save_temp);
        /* 2. move GPR to FPR */
        latxs_append_ir2_opnd2(mfmt_op, opnd_fpr, &tmp);
    } else {
        /* 1. move FPR to GPR */
        latxs_append_ir2_opnd2(mfmt_op, &tmp, opnd_fpr);
        /* 2. store GPR to to memory */
        gen_ldst_softmmu_helper(ldst_op, &tmp, opnd_mem, save_temp);
    }
}
