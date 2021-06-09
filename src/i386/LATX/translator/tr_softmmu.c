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

/* do memory access through BTMMU */
#ifdef CONFIG_BTMMU
void __gen_ldst_softmmu_helper_btmmu(
        IR2_OPCODE op,
        IR2_OPND   *opnd_gpr,
        IR2_OPND   *opnd_mem);
#endif

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

#ifdef CONFIG_BTMMU
void __gen_ldst_softmmu_helper_btmmu(
        IR2_OPCODE op,
        IR2_OPND   *opnd_gpr,
        IR2_OPND   *opnd_mem)
{
    lsassertm(0, "BTMMU to be implemented in LoongArch.\n");
//    TRANSLATION_DATA *td = lsenv->tr_data;
//    int mem_idx = td->sys.mem_index;
//    lsassert(mem_idx != MMU_KSMAP_IDX);
//
//    lsassert(ir2_opnd_base_reg_num(opnd_gpr) != 0x2);
//    lsassert(ir2_opnd_base_reg_num(opnd_mem) != 0x2);
//
//    if (!option_lsfpu) {
//        /* Without LSFPU, the TOP could only be obtained
//         * during translation. But the BTMMU exception
//         * handler is static generated. So we must save
//         * TOP dynamically before every memory access. */
//        tr_gen_save_curr_top();
//        td->is_top_saved = 1;
//    }
//    append_ir2_opndi(mips_setmem, mem_idx == MMU_USER_IDX ? 2 : 3);
//    append_ir2_opnd2(op, opnd_gpr, opnd_mem);
}
#endif

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

//#ifdef CONFIG_BTMMU
//        TranslationBlock *tb = lsenv->tr_data->curr_tb;
//        if (btmmu_enabled() &&
//            !tb->btmmu_disabled &&
//            lsenv->tr_data->is_btmmu_ok)
//        {
//            __gen_ldst_softmmu_helper_btmmu(op, opnd_gpr, &opnd_mem_fixed);
//        } else {
//            __gen_ldst_softmmu_helper_native(op, opnd_gpr, &opnd_mem_fixed, save_temp);
//        }
//#else
//        __gen_ldst_softmmu_helper_native(op, opnd_gpr, &opnd_mem_fixed, save_temp);
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

#ifdef CONFIG_BTMMU

#define FTLB_FUNC_ADDR (void *)0x30000000L
#define FTLB_FUNC_SIZE 0x4000

#define FTLB_DATA_ADDR (void *)0x50000000L
#define FTLB_DATA_SIZE 0x4000

#define OFFSET_OF_EPC_IN_FTLB_DATA  0
#define OFFSET_OF_REGS_IN_FTLB_DATA 1

void *build_data_xqm(void)
{
    lsassertm(0, "BTMMU build data to be implemented in LoongArch.\n");
    return NULL;
//    char *p = mmap(FTLB_DATA_ADDR, FTLB_DATA_SIZE,
//                   PROT_READ | PROT_WRITE | PROT_EXEC,
//                   MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
//
//    if (p == NULL) {
//        lsassertm(0, "[qemu] build data mmap fail.\n");
//    } else {
//        fprintf(stderr, "[qemu] build data mmap at %p.\n", (void*)p);
//    }
//
//    memset(p, 0, FTLB_DATA_SIZE);
//    return p;
}

static int get_mapped_register_index_xqm(int reg)
{
    if (24 <= reg && reg <= 31)
        return reg - 24;
//    switch (reg) {
//    case 0xf:  return 0; /* EAX $t3 0xf  */
//    case 0x3:  return 1; /* ECX $v1 0x3  */
//    case 0x1:  return 2; /* EDX $at 0x1  */
//    case 0x13: return 3; /* EBX $s3 0x13 */
//    case 0x14: return 4; /* ESP $s4 0x14 */
//    case 0x15: return 5; /* EBP $s5 0x15 */
//    case 0x16: return 6; /* ESI $s6 0x16 */
//    case 0x17: return 7; /* EDI $s7 0x17 */
//    default:   return -1;
//    }
}

uint64_t xqm_tlb_miss_pc = 0;

static void btmmu_exception_handler_xqm(void)
{
    lsassertm(0, "BTMMU build handler to be implemented in LoongArch.\n");
//    uint64_t *data = FTLB_DATA_ADDR;
//    uint32_t *epc = (uint32_t *)(*(uint64_t*)(data + OFFSET_OF_EPC_IN_FTLB_DATA));
//    uint64_t *regs = &data[OFFSET_OF_REGS_IN_FTLB_DATA];
//    uint64_t *reg_rt;
//    uint32_t pre_inst, inst;
//    target_ulong addr;
//    int mmu_idx;
//    CPUArchState *env = current_cpu->env_ptr;
//    TCGMemOpIdx oi;
//    int is_load = 0;
//
//    pre_inst = epc[0];
//    inst = epc[1];
//    xqm_tlb_miss_pc = (uint64_t)epc + 8;
//
//    // Analysis SpaceID
//    int is_setmem = (pre_inst & 0xfff00000) == ((0x12 << 26) | (0xf << 21));
//    int mid = (pre_inst >> 18) & 0x3;
//
//    short int offset = inst & 0xffff;
//    unsigned int rt = (inst >> 16) & 0x1F;
//    unsigned int base = (inst >> 21) & 0x1F;
//    unsigned int opc = (inst >> 26) << 26;
//
//    if (!(is_setmem && (mid > 1))) {
//        lsassertm(0, "[MMU] Illegal epc=%p, pre_inst=%x, inst=%x, mid=%d, abort\n",
//                epc, pre_inst, inst, mid);
//    }
//
//    mmu_idx = mid - 1;
//	addr = (target_ulong)regs[base] + offset;
//    reg_rt = &regs[rt];
//
//    switch(opc) {
//        case (044 << 26) : // OPC_LBU
//            oi = (MO_UB << 4) | mmu_idx;
//            *reg_rt = helper_ret_ldub_mmu(env, addr, oi, xqm_tlb_miss_pc);
//            is_load = 1;
//            break;
//        case (040 << 26) : // OPC_LB
//            oi = (MO_SB << 4) | mmu_idx;
//            *reg_rt = (int8_t)helper_ret_ldsb_mmu(env, addr, oi, xqm_tlb_miss_pc);
//            is_load = 1;
//            break;
//        case (045 << 26) : // OPC_LHU
//            oi = (MO_LEUW << 4) | mmu_idx;
//            *reg_rt = helper_le_lduw_mmu(env, addr, oi, xqm_tlb_miss_pc);
//            is_load = 1;
//            break;
//        case (041 << 26) : // OPC_LH
//            oi = (MO_LESW << 4) | mmu_idx;
//            *reg_rt = (int16_t)helper_le_ldsw_mmu(env, addr, oi, xqm_tlb_miss_pc);
//            is_load = 1;
//            break;
//        case (043 << 26) : // OPC_LW
//            oi = (MO_LESL << 4) | mmu_idx;
//            *reg_rt = (int32_t)helper_le_ldsl_mmu(env, addr, oi, xqm_tlb_miss_pc);
//            is_load = 1;
//            break;
//        case (047 << 26) : // OPC_LWU
//            oi = (MO_LEUL << 4) | mmu_idx;
//            *reg_rt = helper_le_ldul_mmu(env, addr, oi, xqm_tlb_miss_pc);
//            is_load = 1;
//            break;
//        case (067 << 26) : // OPC_LD
//            oi = (MO_LEQ << 4) | mmu_idx;
//            *reg_rt = helper_le_ldq_mmu(env, addr, oi, xqm_tlb_miss_pc);
//            is_load = 1;
//            break;
//        case (050 << 26) : // OPC_SB
//            oi = (MO_UB << 4) | mmu_idx;
//            helper_ret_stb_mmu(env, addr, (uint8_t)(*reg_rt & 0xff), oi, xqm_tlb_miss_pc);
//            break;
//        case (051 << 26) : // OPC_SH
//            oi = (MO_LEUW << 4) | mmu_idx;
//            helper_le_stw_mmu(env, addr, (uint16_t)(*reg_rt & 0xffff), oi, xqm_tlb_miss_pc);
//            break;
//        case (053 << 26) : // OPC_SW
//            oi = (MO_LEUL<< 4) | mmu_idx;
//            helper_le_stl_mmu(env, addr, (uint32_t)(*reg_rt & 0xffffffff), oi, xqm_tlb_miss_pc);
//            break;
//        case (077 << 26) : // OPC_SD
//            oi = (MO_LEQ << 4) | mmu_idx;
//            helper_le_stq_mmu(env, addr, (uint64_t)*reg_rt, oi, xqm_tlb_miss_pc);
//            break;
//        default:
//            lsassertm(0, "Illegal opc=%d in SIGILL, abort: --- \n", opc);
//	}
//    if (is_load) {
//        /* load will change the destine register, update it into target register storage */
//        int index = get_mapped_register_index_xqm(rt);
//        if (index >=0) {
//            env->regs[index] = (target_ulong)(*reg_rt);
//        }
//    }
//
//    btmmu_except_count ++;
//    xqm_tlb_miss_pc = 0;
//
//    return;
}

//#define XQM_BTMMU_BUILD_FUNC_PRINT
void* build_func_xqm(void)
{
    lsassertm(0, "BTMMU build func to be implemented in LoongArch.\n");
    return NULL;
//    int i = 0;
//    int *p = mmap(FTLB_FUNC_ADDR, FTLB_FUNC_SIZE,
//                  PROT_READ | PROT_WRITE | PROT_EXEC,
//                  MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
//
//    if (p == NULL) {
//        lsassertm(0, "[qemu] build func mmap fail.\n");
//    } else {
//        fprintf(stderr, "[qemu] build func mmap at %p.\n", (void*)p);
//    }
//
//#ifdef XQM_BTMMU_BUILD_FUNC_PRINT
//    option_dump_ir2  = 1;
//    option_dump_host = 1;
//#endif
//
//    int tmp_reg = 0x2;
//    IR2_OPND tmp = ir2_opnd_new(IR2_OPND_IREG, 0x2);
//
//    IR2_OPND t9 = ir2_opnd_new(IR2_OPND_IREG, 0x19);
//
//    tr_init(NULL);
//
//    /* 1. Store EPC at data_area[0].
//     *    After kernel handling exception,
//     *    $tmp will stores the EPC */
//    append_ir2_opnd2i(mips_daddiu, &t9, &tmp, 0);
//    /* save epc */
//    append_ir2_opnd1i(mips_lui, &tmp, 0x5000);
//    append_ir2_opnd2i(mips_sdi, &t9, &tmp, 0);
//
//    /* 2. Save all registers at data_area[1...] */
//    for (i = 0; i < 32; ++i) {
//        if (i == tmp_reg) continue; /* Not $tmp */
//        IR2_OPND reg = ir2_opnd_new(IR2_OPND_IREG, i);
//        append_ir2_opnd2i(mips_sdi, &reg, &tmp,
//                (i + OFFSET_OF_REGS_IN_FTLB_DATA) * sizeof(long));
//    }
//
//    /* 3. Write mapped GPRs, FPRs, MMXs back into ENV
//     *    Also write EFLAGS into ENV */
//    tr_gen_static_save_registers_to_env(0xff, 0xff, 0xff, 0xff, 0);
//    tr_gen_static_save_eflags();
//
//    /* 4. Jump to handler */
//    tr_gen_call_to_helper((ADDR)btmmu_exception_handler_xqm);
//
//    /* 5. Restore all registers from data_area[1...]
//     *    Restore EFLAGS first since it wlll call the helper */
//    tr_gen_static_load_eflags(0);
//    append_ir2_opnd1i(mips_lui, &tmp, 0x5000);
//    for (i = 0; i < 32; ++i) {
//        if (i == tmp_reg) continue; /* Not $tmp */
//        IR2_OPND reg = ir2_opnd_new(IR2_OPND_IREG, i);
//        append_ir2_opnd2i(mips_ld, &reg, &tmp,
//                (i + OFFSET_OF_REGS_IN_FTLB_DATA) * sizeof(long));
//    }
//    ir2_opnd_set_em(&env_ir2_opnd, EM_MIPS_ADDRESS, 32);
//    tr_gen_static_load_registers_from_env(
//            0, /* GPR is already loaded from data area */
//            FPR_USEDEF_TO_SAVE,
//            XMM_LO_USEDEF_TO_SAVE,
//            XMM_HI_USEDEF_TO_SAVE, 0);
//
//    /* 6. Jump back to EPC + 8 */
//    append_ir2_opnd1i(mips_lui, &tmp, 0x5000);
//    append_ir2_opnd2i(mips_ld, &tmp, &tmp, 0);
//    append_ir2_opnd2i(mips_daddiu, &tmp, &tmp, 8);
//    append_ir2_opnd1(mips_jr, &tmp);
//
//    int code_nr = tr_ir2_assemble((void*)p);
//    if (code_nr > FTLB_FUNC_SIZE / 4) {
//        fprintf(stderr, "[qemu] build func overflow.\n");
//        exit(-1);
//    }
//    tr_fini(false);
//
//#ifdef XQM_BTMMU_BUILD_FUNC_PRINT
//    option_dump_ir2  = 0;
//    option_dump_host = 0;
//    exit(-1);
//#endif
//
//    qm_flush_icache((ADDR)FTLB_FUNC_ADDR, (ADDR)(FTLB_FUNC_ADDR + FTLB_FUNC_SIZE));
//
//    return p;
}
#endif
