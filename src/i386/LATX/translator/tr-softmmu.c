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

static int get_ldst_align_bits(IR2_OPCODE opc);

/* Do QEMU TLB lookup in native context.
 * If TLB miss, call QEMU's softmmu helper. */
void __gen_ldst_softmmu_helper_native(
        IR2_OPCODE  op,
        IR2_OPND   *opnd_gpr,
        IR2_OPND   *opnd_mem,
        int save_temp);
void tr_gen_ldst_slow_path(
        IR2_OPCODE op,
        IR2_OPND  *gpr_opnd,
        IR2_OPND  *mem_opnd,
        IR2_OPND  *label_slow_path,
        IR2_OPND  *label_exit,
        int        mmu_index,
        int        is_load,
        int        save_temp);

int convert_to_tcgmemop(IR2_OPCODE op);

void tr_gen_lookup_qemu_tlb(
        IR2_OPCODE op,
        IR2_OPND *gpr,
        IR2_OPND *mem,
        int mmu_index,
        bool is_load,
        IR2_OPND label_slow_path);

static int is_ldst_realized_by_softmmu(IR2_OPCODE op)
{
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

void __gen_ldst_softmmu_helper_native(
        IR2_OPCODE op,
        IR2_OPND *opnd_gpr,
        IR2_OPND *opnd_mem,
        int save_temp)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    lsassert(is_ldst_realized_by_softmmu(op));

    int mmu_index = td->sys.mem_index;
    bool is_load  = ir2_opcode_is_load(op);

    /* 1. lookup QEMU TLB in native code
     *    and finish memory access if TLB hit
     *    temp register is free to use inside. */
    IR2_OPND label_slow_path = ir2_opnd_new_label();
    tr_gen_lookup_qemu_tlb(op, opnd_gpr, opnd_mem, mmu_index, is_load, label_slow_path);

    /* 2. memory access finish. jump slow path. */
    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd1(LISA_B, &label_exit);

    /* 3. slow path : call QEMU's helper
     *    Here we just record the data to generate slow path
     *    The real slow path will be generated at the end of TB */
    tr_gen_ldst_slow_path(op, opnd_gpr, opnd_mem,
            &label_slow_path, &label_exit,
            mmu_index, is_load, save_temp);

    /* 4. exit from fast path or return from slow path */
    append_ir2_opnd1(LISA_LABEL, &label_exit);
}


/*
 * load:  GPR(opnd_gpr.reg) <- mem(opnd_mem.address)
 * store: GPR(opnd_gpr.reg) -> mem(opnd_mem.address)
 *
 * NOTE: opnds are both IR2_OPND, which means MIPS opnd.
 *
 * Usually the x86vaddress is stored in IR1_OPND, which
 * means X86 opnd. The function 'translate_xxx ' will be
 * responsible for exacting the x86vaddr in IR1_OPND and
 * move it to an IR2_OPND(which is usually a temp register).
 *
 * Since the addressing of x86 architecture is complex, we
 * remain the original processing that exacts the final
 * x86vaddr from IR1_OPND(x86 opnd).
 * It is usually done by functions load_ireg_from_ir1_mem()
 * and store_ireg_from_ir1_mem().
 *
 * @ op: the opcode for MIPS's load/store
 * > prefix is 'mips_' for IR2_OPCODE.
 * > only integer is considered now.
 * > ll(load link) and sc(store condition) is not considered now.
 *   They are usually used to simulate target's atomic instructions.
 *   They are not used in current user-mode X86toMIPS.
 *   -----------------------------------
 *   lbu lhu lwu
 *   lb  lh  lw  lwl lwr ld  ldl ldr
 *   sb  sh  sw  swl swr sdi sdl sdr
 *   -----------------------------------
 * @ opnd_gpr: the mips registers dest for load and src for store
 * @ opnd_mem: the mips registers src for load and dest for store
 * @ save_temp: wether to save/restore temp registers around helper
 */
void gen_ldst_softmmu_helper(
        IR2_OPCODE op,
        IR2_OPND *opnd_gpr,
        IR2_OPND *opnd_mem,
        int save_temp)
{
//    IR2_OPND opnd_mem_fixed = *opnd_mem;
//    int mem_reg_num = ir2_opnd_reg(opnd_mem);

    if (ir2_opcode_is_load(op) || ir2_opcode_is_store(op)) {
        __gen_ldst_softmmu_helper_native(op, opnd_gpr, opnd_mem, save_temp);
//#ifdef N64
//        mem_reg_num = em_validate_address(mem_reg_num);
//        IR2_OPND mem_base_new = ir2_opnd_new(IR2_OPND_IREG, mem_reg_num);
//        ir2_opnd_mem_set_base_ireg(&opnd_mem_fixed, &mem_base_new);
//#endif

        return;
    }

    lsassertm(0, "Softmmu not support non-load/store instruction.");
}

/*
 * mips_lwc1 : load  32-bits from memory to FPR
 * mips_ldc1 : load  64-bits from memory to FPR
 * mips_swc1 : store 32-bits from FPR to memory
 * mips_sdc1 : store 64-bits from FPR to memory
 *
 * This function does not work like one memory access instruction.
 * Because this function will always use temp register.
 */
void gen_ldst_c1_softmmu_helper(
        IR2_OPCODE op,
        IR2_OPND *opnd_fpr,
        IR2_OPND *opnd_mem,
        int save_temp)
{
    IR2_OPCODE ldst_op;
    IR2_OPCODE mfmt_op;
    switch (op) {
        case LISA_FLD_S: ldst_op = LISA_LD_WU; mfmt_op = LISA_MOVGR2FR_W;  break;
        case LISA_FLD_D: ldst_op = LISA_LD_D;  mfmt_op = LISA_MOVGR2FR_D; break;
        case LISA_FST_S: ldst_op = LISA_ST_W;  mfmt_op = LISA_MOVFR2GR_S;  break;
        case LISA_FST_D: ldst_op = LISA_ST_D;  mfmt_op = LISA_MOVFR2GR_D; break;
        default: lsassert(0); break;
    }

    IR2_OPND tmp = ra_alloc_itemp();
    if (ir2_opcode_is_load(op)) {
        /* 1. load to GPR from memory */
        gen_ldst_softmmu_helper(ldst_op, &tmp, opnd_mem, save_temp);
        /* 2. move GPR to FPR */
        append_ir2_opnd2(mfmt_op, opnd_fpr, &tmp);
    } else {
        /* 1. move FPR to GPR */
        append_ir2_opnd2(mfmt_op, &tmp, opnd_fpr);
        /* 2. store GPR to to memory */
        gen_ldst_softmmu_helper(ldst_op, &tmp, opnd_mem, save_temp);
    }
}

static int get_ldst_align_bits(IR2_OPCODE opc)
{
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

/*
 * MemOp in tcg/tcg.h
 *
 * [1:0] op size
 *     = 0 : MO_8
 *     = 1 : MO_16
 *     = 2 : MO_32
 *     = 3 : MO_64
 *    mask : MO_SIZE = 3 = 0b11
 *
 * [  2] sign or unsign
 *     = 1 : signed
 *     = 0 : unsigned
 *    mask : MO_SIGN = 4 = 0b100
 *
 * [  3] host reverse endian
 *   if host is big endian
 *     = 1 : MO_LE
 *     = 0 : MO_BE
 *   if host is little endian
 *     = 1 : MO_BE
 *     = 0 : MO_LE
 *
 * [6:4] aligned or unaligned
 *    = 1 : MO_ALIGN_2  = 0b001000
 *    = 2 : MO_ALIGN_4  = 0b010000
 *    = 3 : MO_ALIGN_8  = 0b011000
 *    = 4 : MO_ALIGN_16 = 0b100000
 *    = 5 : MO_ALIGN_32 = 0b101000
 *    = 6 : MO_ALIGN_64 = 0b110000
 *   mask : MO_AMASK    = 0b111000
 *
 * Conbinatioms :
 * -----------------------------------------
 * MO_UB   : unsigned  8-bits
 * MO_SB   : signed    8-bits
 *
 * MO_UW   : unsigned 16-bits
 * MO_LEUW : unsigned 16-bits little endian
 * MO_LESW : signed   16-bits little endian
 * MO_BEUW : unsigned 16-bits big    endian
 * MO_BESW : signed   16-bits big    endian
 *
 * MO_UL   : unsigned 32-bits
 * MO_LEUL : unsigned 32-bits little endian
 * MO_LESL : signed   32-bits little endian
 * MO_BEUL : unsigned 32-bits big    endian
 * MO_BESL : signed   32-bits big    endian
 *
 * MO_Q    : 64-bits
 * MO_LEQ  : 64-bits little endian
 * MO_BEQ  : 64-bits bit    endian
 * -----------------------------------------
 */
int convert_to_tcgmemop(IR2_OPCODE op)
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
        case LISA_LD_WU:
        case LISA_ST_W:
            return MO_LEUL;
        case LISA_LD_D:
        case LISA_ST_D:
            return MO_LEQ;
        default:
            lsassertm(0, "not support IR2_OPCODE %d in convert_to_tcgmemop.\n", op);
            return -1;
    }
}

/* return label point to slow path */
void tr_gen_lookup_qemu_tlb(
        IR2_OPCODE op,
        IR2_OPND *gpr_opnd,
        IR2_OPND *mem_opnd,
        int mmu_index,
        bool is_load,
        IR2_OPND label_slow_path)
{
//    lsassertm(0, "softmmu lookup TLB to be implemented in LoongArch.\n");
    /* offset from ENV */
    int fast_off  = TLB_MASK_TABLE_OFS(mmu_index);
    int mask_off  = fast_off + offsetof(CPUTLBDescFast, mask);  /* 64-bit */
    int table_off = fast_off + offsetof(CPUTLBDescFast, table); /* 64-bit */
    /* offset from tlb entry */
    int add_off   = offsetof(CPUTLBEntry, addend); /* 64-bit */
    int tag_off   = (is_load ? offsetof(CPUTLBEntry, addr_read)
                   : offsetof(CPUTLBEntry, addr_write)); /* 32-bit */

    /* 0. format the mem opnd */
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(mem_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND mem = ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp)
        append_ir2_opnd2_(lisa_mov32z, &mem, &mem);

    /* 1. load f[mmu].mask */
    IR2_OPND mask_opnd = ra_alloc_itemp();
    if (int32_in_int12(mask_off)) {
        append_ir2_opnd2i(LISA_LD_D, &mask_opnd, &env_ir2_opnd, mask_off);
    } else {
        load_imm32_to_ir2(&mask_opnd, mask_off, SIGN_EXTENSION);
        append_ir2_opnd3(LISA_ADD_D, &mask_opnd, &env_ir2_opnd, &mask_opnd);
        append_ir2_opnd2i(LISA_LD_D, &mask_opnd, &mask_opnd, 0);
    }

    /* 2. extract tlb index */
    IR2_OPND index_opnd = ra_alloc_itemp();
    /* 2.1 index = address >> shift */
    int shift = TARGET_PAGE_BITS - CPU_TLB_ENTRY_BITS;
    append_ir2_opnd2i(LISA_SRLI_W, &index_opnd, &mem, shift);

    /* 2.2 index = index & mask */
    append_ir2_opnd3(LISA_AND, &index_opnd, &index_opnd, &mask_opnd);
    ra_free_temp(&mask_opnd);

    /* 3. load f[mmu].table */
    IR2_OPND table_opnd = ra_alloc_itemp();
    if (int32_in_int12(table_off)) {
        append_ir2_opnd2i(LISA_LD_D, &table_opnd, &env_ir2_opnd, table_off);
    } else {
        load_imm32_to_ir2(&table_opnd, table_off, SIGN_EXTENSION);
        append_ir2_opnd3(LISA_ADD_D, &table_opnd, &env_ir2_opnd, &table_opnd);
        append_ir2_opnd2i(LISA_LD_D, &table_opnd, &table_opnd, 0);
    }

    /* 4. tlb entry = table + index */
    append_ir2_opnd3(LISA_ADD_D, &table_opnd, &table_opnd, &index_opnd);
    IR2_OPND tlb_opnd = table_opnd;
    ra_free_temp(&index_opnd);

    /* 5. load compare part from tlb entry */
//    ir2_opnd_set_em(&tlb_opnd, EM_MIPS_ADDRESS, 32);
    IR2_OPND tag_opnd = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_WU, &tag_opnd, &tlb_opnd, tag_off);

    /* 6. cmp = vaddr & mask */
    int align_bits = get_ldst_align_bits(op);
    ADDRX vaddr_mask = (ADDRX)( TARGET_PAGE_MASK | ((1 << align_bits) - 1) );
    IR2_OPND vaddr_mask_opnd = ra_alloc_itemp();
    load_imm32_to_ir2(&vaddr_mask_opnd, vaddr_mask, ZERO_EXTENSION);
    append_ir2_opnd3(LISA_AND, &vaddr_mask_opnd, &mem, &vaddr_mask_opnd);
    IR2_OPND cmp_opnd = vaddr_mask_opnd;

    /* 7. compare cmp and tag */
    append_ir2_opnd3(LISA_BNE, &cmp_opnd, &tag_opnd, &label_slow_path);
    ra_free_temp(&tag_opnd);
    ra_free_temp(&cmp_opnd);

    /* 8. load addend from tlb entry */
    IR2_OPND add_opnd = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_D, &add_opnd, &tlb_opnd, add_off);
    ra_free_temp(&tlb_opnd);

    /* 9. get hvaddr if not branch */
//    IR2_OPND hvaddr = ra_alloc_itemp();
//    append_ir2_opnd3(LISA_ADD_D, &hvaddr, &mem, &add_opnd);
    switch (op) {
#define SOFTMMU_TLB_HIT_LDST_LOONGARCH(oldop, newop) \
    case oldop: \
        append_ir2_opnd3(newop, gpr_opnd, &mem, &add_opnd); \
        break;
SOFTMMU_TLB_HIT_LDST_LOONGARCH(LISA_LD_B,  LISA_LDX_B)
SOFTMMU_TLB_HIT_LDST_LOONGARCH(LISA_LD_H,  LISA_LDX_H)
SOFTMMU_TLB_HIT_LDST_LOONGARCH(LISA_LD_W,  LISA_LDX_W)
SOFTMMU_TLB_HIT_LDST_LOONGARCH(LISA_LD_D,  LISA_LDX_D)
SOFTMMU_TLB_HIT_LDST_LOONGARCH(LISA_LD_BU, LISA_LDX_BU)
SOFTMMU_TLB_HIT_LDST_LOONGARCH(LISA_LD_HU, LISA_LDX_HU)
SOFTMMU_TLB_HIT_LDST_LOONGARCH(LISA_LD_WU, LISA_LDX_WU)
SOFTMMU_TLB_HIT_LDST_LOONGARCH(LISA_ST_B,  LISA_STX_B)
SOFTMMU_TLB_HIT_LDST_LOONGARCH(LISA_ST_H,  LISA_STX_H)
SOFTMMU_TLB_HIT_LDST_LOONGARCH(LISA_ST_W,  LISA_STX_W)
SOFTMMU_TLB_HIT_LDST_LOONGARCH(LISA_ST_D,  LISA_STX_D)
    default: lsassertm(0, "wrong in softmmu\n"); break;
    }
    ra_free_temp(&add_opnd);

    /* 10. do memory access with hvaddr */
//    append_ir2_opnd2i(op, &*gpr_opnd, &hvaddr, 0);
//    ra_free_temp(&hvaddr);

    if (mem_no_offset_new_tmp)
        ra_free_temp(&mem_no_offset);

#ifdef CONFIG_XTM_PROFILE
    tr_pf_inc_tlb_hit();
#endif
}

void td_rcd_softmmu_slow_path(
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
        sp = &td->slow_path_rcd[sp_nr];
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

    sp->retaddr = (ADDR)(td->curr_tb->tc.ptr) +
                  (ADDR)((td->real_ir2_inst_num - 2) << 2);
    sp->fpu_top = td_fpu_get_top();

    sp->tmp_mask = td->itemp_mask;
}

static
void __tr_gen_softmmu_sp_rcd(softmmu_sp_rcd_t *sp)
{
    append_ir2_opnd1(LISA_LABEL, &sp->label_slow_path);

    TRANSLATION_DATA *td = lsenv->tr_data;
    int tmp_mask_bak = td->itemp_mask;
    td->itemp_mask = sp->tmp_mask;

    int top_bak = td_fpu_get_top();
    td_fpu_set_top(sp->fpu_top);

    if (sp->tmp_need_save)
        tr_save_temp_register_mask(sp->tmp_mask);

    /* 1. save native context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* 2. prepare arguments for softmmu helper */

    /* build arguemnts for the helper
     *               LOAD     |  STORE
     * ---------------------------------------
     * helper arg0 : env      |  env
     * helper arg1 : x86vaddr |  x86vaddr
     * helper arg2 : memop    |  data
     * helper arg3 : retaddr  |  memop
     * helper arg4 :          |  retaddr    */

    /* 2.1 get mem address */
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(
            &sp->mem_ir2_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND mem = ir2_opnd_mem_get_base(&mem_no_offset);
    /* 2.2 convert memop */
    MemOp memop = convert_to_tcgmemop(sp->op);
    TCGMemOpIdx memopidx = (memop << 4) | sp->mmu_index;

    /* 3.1 arg1: mem address */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &mem);
    if (mem_no_offset_new_tmp) ra_free_temp(&mem_no_offset);
    /* 3.2 arg0 : env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 3.3 arg2 : memop(LOAD) data(STORE) */
    if (sp->is_load) {
        append_ir2_opnd2i(LISA_ORI, &arg2_ir2_opnd, &zero_ir2_opnd, memopidx);
    } else {
        append_ir2_opnd2_(lisa_mov, &arg2_ir2_opnd, &sp->gpr_ir2_opnd);
    }
    /* 3.4 arg3 : retaddr(LOAD) memop(STORE) */
    if (sp->is_load) {
        load_imm64_to_ir2(&arg3_ir2_opnd, sp->retaddr);
    } else {
        append_ir2_opnd2i(LISA_ORI, &arg3_ir2_opnd, &zero_ir2_opnd, memopidx);
        /* 3.5 arg4 : retaddr(STORE) */
        load_imm64_to_ir2(&arg4_ir2_opnd, sp->retaddr);
    }

    /* 4. call the helper */
    switch (sp->op) {
    case LISA_ST_B : tr_gen_call_to_helper((ADDR)helper_ret_stb_mmu );break;
    case LISA_ST_H : tr_gen_call_to_helper((ADDR)helper_le_stw_mmu  );break;
    case LISA_ST_W : tr_gen_call_to_helper((ADDR)helper_le_stl_mmu  );break;
    case LISA_ST_D : tr_gen_call_to_helper((ADDR)helper_le_stq_mmu  );break;
    case LISA_LD_B : tr_gen_call_to_helper((ADDR)helper_ret_ldsb_mmu);break;
    case LISA_LD_BU: tr_gen_call_to_helper((ADDR)helper_ret_ldub_mmu);break;
    case LISA_LD_H : tr_gen_call_to_helper((ADDR)helper_le_ldsw_mmu );break;
    case LISA_LD_HU: tr_gen_call_to_helper((ADDR)helper_le_lduw_mmu );break;
    case LISA_LD_W : tr_gen_call_to_helper((ADDR)helper_le_ldsl_mmu );break;
    case LISA_LD_WU: tr_gen_call_to_helper((ADDR)helper_le_ldul_mmu );break;
    case LISA_LD_D : tr_gen_call_to_helper((ADDR)helper_le_ldq_mmu  );break;
    default: lsassert(0); break;
    }

    /* 6. restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

    if (sp->tmp_need_save)
        tr_restore_temp_register_mask(sp->tmp_mask);

    if (sp->is_load)
        append_ir2_opnd2_(lisa_mov, &sp->gpr_ir2_opnd,
                                    &ret0_ir2_opnd);

    td->itemp_mask = tmp_mask_bak;
    td_fpu_set_top(top_bak);

    append_ir2_opnd1(LISA_B, &sp->label_exit);
}

void tr_gen_softmmu_slow_path(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    int i = 0;
    int sp_nr = td->slow_path_rcd_nr;

    softmmu_sp_rcd_t *sp = NULL;

    for (i = 0; i < sp_nr; ++i) {
        sp = &td->slow_path_rcd[i];
        __tr_gen_softmmu_sp_rcd(sp);
    }
}

void tr_gen_ldst_slow_path(
        IR2_OPCODE op,
        IR2_OPND  *gpr_opnd, /* temp(t0-t7) or mapping(s1-s8) */
        IR2_OPND  *mem_opnd,
        IR2_OPND  *label_slow_path,
        IR2_OPND  *label_exit,
        int        mmu_index,
        int        is_load,
        int        save_temp)
{
#ifdef CONFIG_XTM_PROFILE
    tr_pf_inc_tlb_miss();
#endif

    append_ir2_opnd1(LISA_B, label_slow_path);

    td_rcd_softmmu_slow_path(
            op, gpr_opnd, mem_opnd,
            label_slow_path, label_exit,
            mmu_index, save_temp, is_load);

    /* xtm_helper_xxx are no longer used, to be removed. */
//    /* 5. call the helper */
//    switch (op) {
//    case LISA_ST_B:  tr_gen_call_to_helper((ADDR)xtm_helper_ret_stb_mmu); break;
//    case LISA_ST_H:  tr_gen_call_to_helper((ADDR)xtm_helper_le_stw_mmu);  break;
//    case LISA_ST_W:  tr_gen_call_to_helper((ADDR)xtm_helper_le_stl_mmu);  break;
//    case LISA_ST_D:  tr_gen_call_to_helper((ADDR)xtm_helper_le_stq_mmu);  break;
//    case LISA_LD_B:  tr_gen_call_to_helper((ADDR)xtm_helper_ret_ldsb_mmu);break;
//    case LISA_LD_BU: tr_gen_call_to_helper((ADDR)xtm_helper_ret_ldub_mmu);break;
//    case LISA_LD_H:  tr_gen_call_to_helper((ADDR)xtm_helper_le_ldsw_mmu); break;
//    case LISA_LD_HU: tr_gen_call_to_helper((ADDR)xtm_helper_le_lduw_mmu); break;
//    case LISA_LD_W:  tr_gen_call_to_helper((ADDR)xtm_helper_le_ldsl_mmu); break;
//    case LISA_LD_WU: tr_gen_call_to_helper((ADDR)xtm_helper_le_ldul_mmu); break;
//    case LISA_LD_D:  tr_gen_call_to_helper((ADDR)xtm_helper_le_ldq_mmu);  break;
//    default:       lsassert(0);                                                break;
//    }

}


/// Attention: IR1 OPCODE with IR2 OPRANDS! In order to simulate atomic operation, cannot split ir1 into ir2
/// @param
void gen_helper_atomic(IR1_OPCODE op, IR2_OPND *opr1, IR2_OPND *opr2, int save_temp)
{
    lsassertm(0, "softmmu atomic to be implemented in LoongArch.\n");
//    TRANSLATION_DATA *td = lsenv->tr_data;
//    if (save_temp) {
//        /* a0-7, t0,t1 */
//        tr_save_temp_register();
//    }
//
//    int sv_all_gpr = 1;
//    int sv_eflags  = 1;
//    tr_sys_gen_call_to_helper_prologue(sv_all_gpr, sv_eflags);
//
//    // arg0 = a0 CPUX86State*
//    append_ir2_opnd3(mips_or, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
//
//    switch(op)
//    {
//        case X86_INS_BTS:
//        {
//            // Start of preparation of helper arguments //
//            int dest_reg_num = ir2_opnd_base_reg_num(opr1); // allocate a reg for return value
//            IR2_OPND dest_reg = ir2_opnd_new(IR2_OPND_IREG, dest_reg_num);
//            // int mem_reg_off = ir2_opnd_imm(opnd_mem);
//            // int mmu_index   = td->sys.mem_index;
//
//            // arg1 = a1 = target_ulong addr
//            int mem_reg_num = ir2_opnd_base_reg_num(opr2);
//            IR2_OPND mem_reg = ir2_opnd_new(IR2_OPND_IREG, mem_reg_num);
//            append_ir2_opnd3(mips_or, &arg1_ir2_opnd, &mem_reg, &zero_ir2_opnd);
//
//            // arg2 = a2 = ABI_TYPE val
//            IR2_OPND val;
//            ir2_opnd_build(&val, IR2_OPND_IMM, 1);
//            append_ir2_opnd3(mips_ori, &arg2_ir2_opnd, &zero_ir2_opnd, &val);
//
//            // arg3 = a3 = TCGMemOpIdx oi
//            int oi_int = make_memop_idx(MO_32|MO_LE, td->sys.mem_index);
//            IR2_OPND oi;
//            ir2_opnd_build(&oi, IR2_OPND_IMM, oi_int);
//            append_ir2_opnd3(mips_ori, &arg3_ir2_opnd, &zero_ir2_opnd, &oi);
//            // End of preparation of helper arguments //
//
//            tr_gen_call_to_helper((ADDR)helper_atomic_fetch_orl_le);
//            // return value
//            append_ir2_opnd3(mips_or, &dest_reg, &ret_ir2_opnd, &zero_ir2_opnd);
//            break;
//        }
//        default:
//        {
//            break;
//        }
//    }
//
//    tr_sys_gen_call_to_helper_epilogue(sv_all_gpr, sv_eflags);
//
//    if (save_temp) {
//        /* a0-7, t0,t1 */
//        tr_restore_temp_register();
//    }
}
