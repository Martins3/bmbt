#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/common.h"
#include "../ir1/ir1.h"

IR1_OPND al_ir1_opnd;
IR1_OPND ah_ir1_opnd;
IR1_OPND ax_ir1_opnd;
IR1_OPND eax_ir1_opnd;
IR1_OPND rax_ir1_opnd;

IR1_OPND cl_ir1_opnd;
IR1_OPND ch_ir1_opnd;
IR1_OPND cx_ir1_opnd;
IR1_OPND ecx_ir1_opnd;
IR1_OPND rcx_ir1_opnd;

IR1_OPND dl_ir1_opnd;
IR1_OPND dh_ir1_opnd;
IR1_OPND dx_ir1_opnd;
IR1_OPND edx_ir1_opnd;
IR1_OPND rdx_ir1_opnd;

IR1_OPND bl_ir1_opnd;
IR1_OPND bh_ir1_opnd;
IR1_OPND bx_ir1_opnd;
IR1_OPND ebx_ir1_opnd;
IR1_OPND rbx_ir1_opnd;

IR1_OPND sp_ir1_opnd;
IR1_OPND esp_ir1_opnd;
IR1_OPND rsp_ir1_opnd;

IR1_OPND bp_ir1_opnd;
IR1_OPND ebp_ir1_opnd;
IR1_OPND rbp_ir1_opnd;

IR1_OPND si_ir1_opnd;
IR1_OPND esi_ir1_opnd;
IR1_OPND rsi_ir1_opnd;

IR1_OPND di_ir1_opnd;
IR1_OPND edi_ir1_opnd;
IR1_OPND rdi_ir1_opnd;

IR1_OPND eax_mem8_ir1_opnd;
IR1_OPND ecx_mem8_ir1_opnd;
IR1_OPND edx_mem8_ir1_opnd;
IR1_OPND ebx_mem8_ir1_opnd;
IR1_OPND esp_mem8_ir1_opnd;
IR1_OPND ebp_mem8_ir1_opnd;
IR1_OPND esi_mem8_ir1_opnd;
IR1_OPND edi_mem8_ir1_opnd;

IR1_OPND eax_mem16_ir1_opnd;
IR1_OPND ecx_mem16_ir1_opnd;
IR1_OPND edx_mem16_ir1_opnd;
IR1_OPND ebx_mem16_ir1_opnd;
IR1_OPND esp_mem16_ir1_opnd;
IR1_OPND ebp_mem16_ir1_opnd;
IR1_OPND esi_mem16_ir1_opnd;
IR1_OPND edi_mem16_ir1_opnd;

IR1_OPND eax_mem32_ir1_opnd;
IR1_OPND ecx_mem32_ir1_opnd;
IR1_OPND edx_mem32_ir1_opnd;
IR1_OPND ebx_mem32_ir1_opnd;
IR1_OPND esp_mem32_ir1_opnd;
IR1_OPND ebp_mem32_ir1_opnd;
IR1_OPND esi_mem32_ir1_opnd;
IR1_OPND edi_mem32_ir1_opnd;

static IR1_OPND ir1_opnd_new_static_reg(IR1_OPND_TYPE opnd_type, int size,
                                        x86_reg reg)
{
    IR1_OPND ir1_opnd;
    IR1_OPND *opnd = &ir1_opnd;

    opnd->type = opnd_type;
    lsassert(size % 8 == 0);
    opnd->size = size / 8;
    opnd->reg = reg;
    opnd->avx_bcast = X86_AVX_BCAST_INVALID;

    return ir1_opnd;
}

static IR1_OPND ir1_opnd_new_static_mem(IR1_OPND_TYPE opnd_type, int size,
                                        x86_reg reg, uint64_t imm)
{
    switch (reg) {
    case X86_REG_EAX:
    case X86_REG_ECX:
    case X86_REG_EBX:
    case X86_REG_EDX:
    case X86_REG_ESI:
    case X86_REG_EDI:
    case X86_REG_ESP:
    case X86_REG_EBP:
        break;
    default:
        lsassert(0);
    }

    IR1_OPND ir1_opnd;
    IR1_OPND *opnd = &ir1_opnd;

    opnd->type = opnd_type;
    lsassert(size % 8 == 0);
    opnd->size = size / 8;
    opnd->size = size / 8;
    opnd->mem.base = reg;

    opnd->mem.index = X86_REG_INVALID;
    opnd->mem.segment = X86_REG_INVALID;
    opnd->mem.scale = 0;
    opnd->mem.disp = imm;

    opnd->avx_bcast = X86_AVX_BCAST_INVALID;

    return ir1_opnd;
}
static void __attribute__((__constructor__)) x86tomisp_ir1_init(void)
{
    al_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 8,  X86_REG_AL);
    ah_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 8,  X86_REG_AH);
    ax_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_AX);
    eax_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_EAX);
    rax_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RAX);

    cl_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 8,  X86_REG_CL);
    ch_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 8,  X86_REG_CH);
    cx_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_CX);
    ecx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_ECX);
    rcx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RCX);

    dl_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 8,  X86_REG_DL);
    dh_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 8,  X86_REG_DH);
    dx_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_DX);
    edx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_EDX);
    rdx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RDX);

    bl_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 8,  X86_REG_BL);
    bh_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 8,  X86_REG_BH);
    bx_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_BX);
    ebx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_EBX);
    rbx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RBX);

    sp_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_ESP);
    esp_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_ESP);
    rsp_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RSP);

    bp_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_EBP);
    ebp_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_EBP);
    rbp_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RBP);

    si_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_SI);
    esi_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_ESI);
    rsi_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RSI);

    di_ir1_opnd  = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_DI);
    edi_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_EDI);
    rdi_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RDI);

    eax_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_EAX, 0);
    ecx_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_ECX, 0);
    edx_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_EDX, 0);
    ebx_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_EBX, 0);
    esp_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_ESP, 0);
    ebp_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_EBP, 0);
    esi_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_ESI, 0);
    edi_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_EDI, 0);

    eax_mem16_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_EAX, 0);
    ecx_mem16_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_ECX, 0);
    edx_mem16_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_EDX, 0);
    ebx_mem16_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_EBX, 0);
    esp_mem16_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_ESP, 0);
    ebp_mem16_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_EBP, 0);
    esi_mem16_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_ESI, 0);
    edi_mem16_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_EDI, 0);

    eax_mem32_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_EAX, 0);
    ecx_mem32_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_ECX, 0);
    edx_mem32_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_EDX, 0);
    ebx_mem32_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_EBX, 0);
    esp_mem32_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_ESP, 0);
    ebp_mem32_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_EBP, 0);
    esi_mem32_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_ESI, 0);
    edi_mem32_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_EDI, 0);
};

void xtm_capstone_init(void);
csh handle;
#ifdef CONFIG_SOFTMMU
#include "../include/env.h"
csh handle16;
#endif
void xtm_capstone_init(void)
{
  // @todo this is a really quick fix
  cs_opt_mem opt;
  opt.malloc = malloc;
  opt.calloc = calloc;
  opt.realloc = realloc;
  opt.free = free;
  opt.vsnprintf = vsnprintf;
  cs_option(CS_ARCH_X86, CS_OPT_MEM, (size_t)&opt);

  if (cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK) {
    fprintf(stderr, "%s %s %d error : cs_open \n", __FILE__, __func__,
            __LINE__);
    exit(-1);
  }
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
#ifdef CONFIG_SOFTMMU
    if (cs_open(CS_ARCH_X86, CS_MODE_16, &handle16) != CS_ERR_OK) {
        fprintf(stderr, "%s %s %d error : cs_open \n", __FILE__, __func__,
                __LINE__);
        exit(-1);
    }
    cs_option(handle16, CS_OPT_DETAIL, CS_OPT_ON);
#endif
}

ADDRX ir1_disasm(IR1_INST *ir1, uint8_t *addr, ADDRX t_pc, int *error)
{
    cs_insn *info;
#ifdef CONFIG_SOFTMMU
#ifdef CONFIG_XTM_PROFILE
    xtm_pf_tc_tr_disasm_capstone_st();
#endif
    TRANSLATION_DATA *td = lsenv->tr_data;
    int count = 0;
    ADDRX t_eip = t_pc - td->sys.cs_base; /* EIP = PC - CS_BASE */
    if (td->sys.code32) {
#ifdef CONFIG_XTM_MD_CAPSTONE
        count = xtm_disasm_x86(handle, addr, 15, (uint64_t)t_eip, 1, &info);
#else
        count = cs_disasm(handle, addr, 15, (uint64_t)t_eip, 1, &info);
#endif
    } else{
#ifdef CONFIG_XTM_MD_CAPSTONE
        count = xtm_disasm_x86(handle16, addr, 15, (uint64_t)t_eip, 1, &info);
#else
        count = cs_disasm(handle16, addr, 15, (uint64_t)t_eip, 1, &info);
#endif
    }
#ifdef CONFIG_XTM_PROFILE
    xtm_pf_tc_tr_disasm_capstone_ed();
#endif
#else /* NOT CONFIG_SOFTMM */
#ifdef CONFIG_XTM_MD_CAPSTONE
    int count = xtm_disasm_x86(handle, addr, 15, (uint64_t)t_pc, 1, &info);
#else
    int count = cs_disasm(handle, addr, 15, (uint64_t)t_pc, 1, &info);
#endif
#endif

    ir1->info = info;
#ifndef CONFIG_XTM_MD_CAPSTONE
    ir1->info_count = count;
#endif

    /* Disasm error */
    if (count != 1) {
        *error = 1;
        return 0;
    }

    ir1->flags = 0;
    ir1->_eflag_def = 0;
    ir1->_eflag_use = 0;
    ir1->_native_inst_num = 0;

#ifndef CONFIG_SOFTMMU /* only in user-mode */
    // xtm treat opnd with default seg(without segment-override prefix) as a mem
    // opnd so, we make it invalid
    if (info->detail->x86.prefix[1] != X86_PREFIX_CS &&
        info->detail->x86.prefix[1] != X86_PREFIX_DS &&
        info->detail->x86.prefix[1] != X86_PREFIX_SS &&
        info->detail->x86.prefix[1] != X86_PREFIX_ES &&
        info->detail->x86.prefix[1] != X86_PREFIX_FS &&
        info->detail->x86.prefix[1] != X86_PREFIX_GS) {
        for (int i = 0; i < info->detail->x86.op_count; i++) {
            if (info->detail->x86.operands[i].type == X86_OP_MEM) {
                info->detail->x86.operands[i].mem.segment = X86_REG_INVALID;
            }
        }
    }
#endif

#ifdef CONFIG_SOFTMMU
    fix_up_ir1(ir1);
    t_pc = ir1_addr(ir1) + td->sys.cs_base;
    return (ADDRX)(t_pc + ir1->info->size);
#else
    return (ADDRX)(ir1->info->address + ir1->info->size);
#endif
}

void ir1_free_info(IR1_INST *ir1)
{
    if (ir1) {
#ifdef CONFIG_XTM_MD_CAPSTONE
        if (ir1->info) {
            if (ir1->info->detail) {
                xtm_free_cs_detail(ir1->info->detail);
            }
            xtm_free_cs_insn(ir1->info);
        }
#else
        cs_free(ir1->info, ir1->info_count);
#endif
    }
}

void ir1_opnd_build_reg(IR1_OPND *opnd, int size, x86_reg reg)
{
    opnd->type = X86_OP_REG;
    lsassert(size % 8 == 0);
    opnd->size = size / 8;
    opnd->reg = reg;
    opnd->avx_bcast = X86_AVX_BCAST_INVALID;
}

void ir1_opnd_build_imm(IR1_OPND *opnd, int size, int64_t imm)
{
    opnd->type = X86_OP_IMM;
    lsassert(size % 8 == 0);
    opnd->size = size / 8;
    opnd->imm = imm;
    opnd->avx_bcast = X86_AVX_BCAST_INVALID;
}

void ir1_opnd_build_mem(IR1_OPND *opnd, int size, x86_reg base, int64_t disp)
{
    opnd->type = X86_OP_MEM;
    lsassert(size % 8 == 0);
    opnd->size = size / 8;
    opnd->mem.base = base;
    opnd->mem.disp = disp;
    opnd->mem.index = X86_REG_INVALID;
    opnd->mem.scale = 0;
    opnd->mem.segment = X86_REG_INVALID;
    opnd->avx_bcast = X86_AVX_BCAST_INVALID;
}

#ifdef CONFIG_SOFTMMU
void ir1_opnd_build_full_mem(
        IR1_OPND *opnd,
        int size,
        x86_reg seg,
        x86_reg base,
        int64_t disp,
        int64_t index,
        int64_t scale)
{
    opnd->type = X86_OP_MEM;

    lsassert(size % 8 == 0);
    opnd->size = size / 8;

    opnd->mem.segment = seg;
    opnd->mem.base    = base;

    opnd->mem.disp  = disp;
    opnd->mem.index = index;
    opnd->mem.scale = scale;

    opnd->avx_bcast = X86_AVX_BCAST_INVALID;
}
#endif

IR1_OPND_TYPE ir1_opnd_type(IR1_OPND *opnd) { return opnd->type; }

int ir1_opnd_size(IR1_OPND *opnd) { return opnd->size << 3; }

int ir1_opnd_base_reg_bits_start(IR1_OPND *opnd)
{
    lsassert(opnd->type == X86_OP_REG && opnd->reg > X86_REG_INVALID &&
             opnd->reg < X86_REG_ENDING);
    switch (opnd->reg) {
    case X86_REG_AH:
    case X86_REG_BH:
    case X86_REG_CH:
    case X86_REG_DH:
        return 8;
    default:
        return 0;
    }
}

x86_reg ir1_opnd_index_reg(IR1_OPND *opnd) { return opnd->mem.index; }

x86_reg ir1_opnd_base_reg(IR1_OPND *opnd) { return opnd->mem.base; }

int ir1_opnd_scale(IR1_OPND *opnd) { return opnd->mem.scale; }

int ir1_opnd_index_reg_num(IR1_OPND *opnd)
{
    switch (opnd->mem.index) {
    case X86_REG_AL:
    case X86_REG_AH:
    case X86_REG_AX:
    case X86_REG_EAX:
        return eax_index;
    case X86_REG_BL:
    case X86_REG_BH:
    case X86_REG_BX:
    case X86_REG_EBX:
        return ebx_index;
    case X86_REG_CL:
    case X86_REG_CH:
    case X86_REG_CX:
    case X86_REG_ECX:
        return ecx_index;
    case X86_REG_DL:
    case X86_REG_DH:
    case X86_REG_DX:
    case X86_REG_EDX:
        return edx_index;
    case X86_REG_BP:
    case X86_REG_EBP:
        return ebp_index;
    case X86_REG_SI:
    case X86_REG_ESI:
        return esi_index;
    case X86_REG_DI:
    case X86_REG_EDI:
        return edi_index;
    case X86_REG_SP:
    case X86_REG_ESP:
        return esp_index;
    case X86_REG_CS:
        return cs_index;
    case X86_REG_DS:
        return ds_index;
    case X86_REG_SS:
        return ss_index;
    case X86_REG_ES:
        return es_index;
    case X86_REG_GS:
        return gs_index;
    case X86_REG_FS:
        return fs_index;

    default:
        lsassert(0);
        // may fpu?
        return -1;
    }
}
int ir1_opnd_base_reg_num(IR1_OPND *opnd)
{
    // xtm : base may not a mem opnd
    if (opnd->type == X86_OP_MEM && opnd->mem.base == X86_REG_INVALID) {
        return -1;
    }
    //    lsassert(opnd->type == X86_OP_MEM || opnd->type == X86_OP_REG);
    switch (opnd->type == X86_OP_REG ? opnd->reg : opnd->mem.base) {
    case X86_REG_AL:
    case X86_REG_AH:
    case X86_REG_AX:
    case X86_REG_EAX:
        return eax_index;
    case X86_REG_BL:
    case X86_REG_BH:
    case X86_REG_BX:
    case X86_REG_EBX:
        return ebx_index;
    case X86_REG_CL:
    case X86_REG_CH:
    case X86_REG_CX:
    case X86_REG_ECX:
        return ecx_index;
    case X86_REG_DL:
    case X86_REG_DH:
    case X86_REG_DX:
    case X86_REG_EDX:
        return edx_index;
    case X86_REG_BP:
    case X86_REG_EBP:
        return ebp_index;
    case X86_REG_SI:
    case X86_REG_ESI:
        return esi_index;
    case X86_REG_DI:
    case X86_REG_EDI:
        return edi_index;
    case X86_REG_SP:
    case X86_REG_ESP:
        return esp_index;
    case X86_REG_CS:
        return cs_index;
    case X86_REG_DS:
        return ds_index;
    case X86_REG_SS:
        return ss_index;
    case X86_REG_ES:
        return es_index;
    case X86_REG_GS:
        return gs_index;
    case X86_REG_FS:
        return fs_index;
    case X86_REG_MM0:
        return 0;
    case X86_REG_MM1:
        return 1;
    case X86_REG_MM2:
        return 2;
    case X86_REG_MM3:
        return 3;
    case X86_REG_MM4:
        return 4;
    case X86_REG_MM5:
        return 5;
    case X86_REG_MM6:
        return 6;
    case X86_REG_MM7:
        return 7;
    case X86_REG_XMM0:
        return 0;
    case X86_REG_XMM1:
        return 1;
    case X86_REG_XMM2:
        return 2;
    case X86_REG_XMM3:
        return 3;
    case X86_REG_XMM4:
        return 4;
    case X86_REG_XMM5:
        return 5;
    case X86_REG_XMM6:
        return 6;
    case X86_REG_XMM7:
        return 7;
    case X86_REG_YMM0:
        return 0;
    case X86_REG_YMM1:
        return 1;
    case X86_REG_YMM2:
        return 2;
    case X86_REG_YMM3:
        return 3;
    case X86_REG_YMM4:
        return 4;
    case X86_REG_YMM5:
        return 5;
    case X86_REG_YMM6:
        return 6;
    case X86_REG_YMM7:
        return 7;
    case X86_REG_ST0:
        return 0;
    case X86_REG_ST1:
        return 1;
    case X86_REG_ST2:
        return 2;
    case X86_REG_ST3:
        return 3;
    case X86_REG_ST4:
        return 4;
    case X86_REG_ST5:
        return 5;
    case X86_REG_ST6:
        return 6;
    case X86_REG_ST7:
        return 7;
    default:
        lsassert(0);
        return -1;
    }
}

longx ir1_opnd_simm(IR1_OPND *opnd)
{
    if (ir1_opnd_type(opnd) == X86_OP_IMM) {
        switch (ir1_opnd_size(opnd)) {
        case 8:
            return (longx)((int8_t)(opnd->imm));
        case 16:
            return (longx)((int16_t)(opnd->imm));
        case 32:
            return (longx)((int32_t)(opnd->imm));
        case 64:
            lsassert(0);
        default:
            lsassert(0);
        }
    } else if (ir1_opnd_type(opnd) == X86_OP_MEM) {
        return (longx)(opnd->mem.disp);
    } else {
        lsassertm(0, "REG opnd has no imm\n");
    }
}

ulongx ir1_opnd_uimm(IR1_OPND *opnd)
{
    if (ir1_opnd_type(opnd) == X86_OP_IMM) {
        return (ulongx)(opnd->imm);
    } else if (ir1_opnd_type(opnd) == X86_OP_MEM) {
        return (ulongx)(opnd->mem.disp);
    } else {
        lsassertm(0, "REG opnd has no imm\n");
    }
}

x86_reg ir1_opnd_seg_reg(IR1_OPND *opnd) { return opnd->mem.segment; }

int ir1_opnd_is_imm(IR1_OPND *opnd) { return opnd->type == X86_OP_IMM; }

int ir1_opnd_is_8h(IR1_OPND *opnd)
{
    return opnd->type == X86_OP_REG && opnd->size == 1 &&
           (opnd->reg == X86_REG_AH || opnd->reg == X86_REG_BH ||
            opnd->reg == X86_REG_CH || opnd->reg == X86_REG_DH);
}

int ir1_opnd_is_8l(IR1_OPND *opnd)
{
    return opnd->type == X86_OP_REG && opnd->size == 1 &&
           (opnd->reg == X86_REG_AL || opnd->reg == X86_REG_BL ||
            opnd->reg == X86_REG_CL || opnd->reg == X86_REG_DL);
}

int ir1_opnd_is_gpr(IR1_OPND *opnd)
{
    if (opnd->type != X86_OP_REG) {
        return 0;
    }
    switch (opnd->reg) {
    case X86_REG_AL:
    case X86_REG_AH:
    case X86_REG_AX:
    case X86_REG_EAX:
    case X86_REG_BL:
    case X86_REG_BH:
    case X86_REG_BX:
    case X86_REG_EBX:
    case X86_REG_CL:
    case X86_REG_CH:
    case X86_REG_CX:
    case X86_REG_ECX:
    case X86_REG_DL:
    case X86_REG_DH:
    case X86_REG_DX:
    case X86_REG_EDX:

    case X86_REG_BP:
    case X86_REG_EBP:
    case X86_REG_SI:
    case X86_REG_ESI:
    case X86_REG_DI:
    case X86_REG_EDI:
    case X86_REG_SP:
    case X86_REG_ESP:
        return 1;
    default:
        return 0;
    }
}

int ir1_opnd_is_uimm_within_16bit(IR1_OPND *opnd)
{
    return ir1_opnd_is_imm(opnd) && ir1_opnd_uimm(opnd) < 65536;
}

int ir1_opnd_is_simm_within_16bit(IR1_OPND *opnd)
{
    return ir1_opnd_is_imm(opnd) && ir1_opnd_simm(opnd) > -32768 &&
           ir1_opnd_simm(opnd) < 32767;
}

int ir1_opnd_is_gpr_used(IR1_OPND *opnd, uint8_t gpr_index)
{
    if (ir1_opnd_is_gpr(opnd)) {
        return ir1_opnd_base_reg_num(opnd) == gpr_index;
    } else if (ir1_opnd_is_mem(opnd)) {
        if (ir1_opnd_has_base(opnd)) {
            return ir1_opnd_base_reg_num(opnd) == gpr_index;
        }
        if (ir1_opnd_has_index(opnd)) {
            return ir1_opnd_index_reg_num(opnd) == gpr_index;
        }
    }
    return 0;
}

int ir1_opnd_is_mem(IR1_OPND *opnd) { return opnd->type == X86_OP_MEM; }

int ir1_opnd_is_sib(IR1_OPND *opnd) { lsassert(0); }

int ir1_opnd_is_fpr(IR1_OPND *opnd)
{
    if (opnd->type != X86_OP_REG) {
        return 0;
    }
    switch (opnd->reg) {
    case X86_REG_ST0:
    case X86_REG_ST1:
    case X86_REG_ST2:
    case X86_REG_ST3:
    case X86_REG_ST4:
    case X86_REG_ST5:
    case X86_REG_ST6:
    case X86_REG_ST7:
        return 1;
    default:
        return 0;
    }
}

int ir1_opnd_is_seg(IR1_OPND *opnd)
{
    if (opnd->type != X86_OP_REG) {
        return 0;
    }
    switch (opnd->reg) {
    case X86_REG_CS:
    case X86_REG_DS:
    case X86_REG_SS:
    case X86_REG_ES:
    case X86_REG_FS:
    case X86_REG_GS:
        return 1;
    default:
        return 0;
    }
}

int ir1_opnd_is_mmx(IR1_OPND *opnd)
{
    if (opnd->type != X86_OP_REG) {
        return 0;
    }
    switch (opnd->reg) {
    case X86_REG_MM0:
    case X86_REG_MM1:
    case X86_REG_MM2:
    case X86_REG_MM3:
    case X86_REG_MM4:
    case X86_REG_MM5:
    case X86_REG_MM6:
    case X86_REG_MM7:
        return 1;
    default:
        return 0;
    }
}

int ir1_opnd_is_xmm(IR1_OPND *opnd)
{
    if (opnd->type != X86_OP_REG) {
        return 0;
    }
    switch (opnd->reg) {
    case X86_REG_XMM0:
    case X86_REG_XMM1:
    case X86_REG_XMM2:
    case X86_REG_XMM3:
    case X86_REG_XMM4:
    case X86_REG_XMM5:
    case X86_REG_XMM6:
    case X86_REG_XMM7:
        return 1;
    default:
        return 0;
    }
}

int ir1_opnd_is_ymm(IR1_OPND *opnd)
{
    if (opnd->type != X86_OP_REG) {
        return 0;
    }
    switch (opnd->reg) {
    case X86_REG_YMM0:
    case X86_REG_YMM1:
    case X86_REG_YMM2:
    case X86_REG_YMM3:
    case X86_REG_YMM4:
    case X86_REG_YMM5:
    case X86_REG_YMM6:
    case X86_REG_YMM7:
        return 1;
    default:
        return 0;
    }
}

int ir1_opnd_is_gpr_defined(IR1_OPND *opnd, uint8 gpr_index) { lsassert(0); }

int ir1_opnd_is_imm0(IR1_OPND *opnd)
{
    if (ir1_opnd_is_imm(opnd) && ir1_opnd_simm(opnd) == 0) {
        return 1;
    }
    return 0;
}

int ir1_opnd_has_base(IR1_OPND *opnd)  { return opnd->mem.base    != X86_REG_INVALID; }
int ir1_opnd_has_index(IR1_OPND *opnd) { return opnd->mem.index   != X86_REG_INVALID; }
int ir1_opnd_has_seg(IR1_OPND *opnd)   { return opnd->mem.segment != X86_REG_INVALID; }

int ir1_opnd_get_seg_index(IR1_OPND *opnd)
{
    lsassert(ir1_opnd_type(opnd) == X86_OP_MEM);
    x86_reg seg = opnd->mem.segment;
    if (seg == X86_REG_ES) {
        return es_index;
    }
    if (seg == X86_REG_CS) {
        return cs_index;
    }
    if (seg == X86_REG_SS) {
        return ss_index;
    }
    if (seg == X86_REG_DS) {
        return ds_index;
    }
    if (seg == X86_REG_FS) {
        return fs_index;
    }
    if (seg == X86_REG_GS) {
        return gs_index;
    } else {
        return -1;
    }
}

IR1_PREFIX ir1_prefix(IR1_INST *ir1)
{
    return ir1->info->detail->x86.prefix[0];  // only lock rep
}

int ir1_opnd_num(IR1_INST *ir1) { return ir1->info->detail->x86.op_count; }

ADDRX ir1_addr(IR1_INST *ir1) { return ir1->info->address; }

ADDRX ir1_addr_next(IR1_INST *ir1)
{
    return ir1->info->address + ir1->info->size;
}

ADDRX ir1_target_addr(IR1_INST *ir1)
{
    lsassert(ir1_opnd_type(&(ir1->info->detail->x86.operands[0])) ==
             X86_OP_IMM);
    return (ADDRX)ir1_opnd_uimm(&(ir1->info->detail->x86.operands[0]));
}

IR1_OPCODE ir1_opcode(IR1_INST *ir1) { return ir1->info->id; }

int ir1_grp_nr(IR1_INST *ir1)
{
    return ir1->info->detail->groups_count;
}

uint8_t* ir1_get_grps(IR1_INST *ir1)
{
    return ir1->info->detail->groups;
}

int ir1_is_branch(IR1_INST *ir1)
{
    switch (ir1->info->id) {
    case X86_INS_JO:
    case X86_INS_JNO:
    case X86_INS_JB:
    case X86_INS_JAE:
    case X86_INS_JE:
    case X86_INS_JNE:
    case X86_INS_JBE:
    case X86_INS_JA:
    case X86_INS_JS:
    case X86_INS_JNS:
    case X86_INS_JP:
    case X86_INS_JNP:
    case X86_INS_JL:
    case X86_INS_JGE:
    case X86_INS_JLE:
    case X86_INS_JG:

    case X86_INS_LOOPNE:
    case X86_INS_LOOPE:
    case X86_INS_LOOP:
    case X86_INS_JCXZ:
    case X86_INS_JECXZ:
    case X86_INS_JRCXZ:

        return 1;

    default:
        return 0;
    }
}

#ifdef CONFIG_SOFTMMU
int ir1_is_jump_far(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_LJMP;
}
int ir1_is_mov_to_cr(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_MOV &&
        ir1_opnd_is_cr(ir1_get_opnd(ir1, 0));
}
int ir1_is_mov_from_cr(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_MOV &&
        ir1_opnd_is_cr(ir1_get_opnd(ir1, 1));
}
int ir1_is_mov_to_dr(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_MOV &&
        ir1_opnd_is_dr(ir1_get_opnd(ir1, 0));
}
int ir1_is_mov_to_seg(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_MOV &&
        ir1_opnd_is_seg(ir1_get_opnd(ir1, 0));
}
int ir1_is_pop_eflags(IR1_INST *ir1)
{
    IR1_OPCODE opcode = ir1_opcode(ir1);
    return opcode == X86_INS_POPF  ||
           opcode == X86_INS_POPFD ||
           opcode == X86_INS_POPFQ ;
}
int ir1_is_rsm(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_RSM;
}
int ir1_is_sti(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_STI;
}
int ir1_is_nop(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_NOP;
}
int ir1_is_repz_nop(IR1_INST *ir1)
{
    if(!ir1_is_nop(ir1))
        return 0;

    /* might be a bug of capstone:
     * For inst '0xf3 0x90' the prefix in cs_insn
     * will not be setted, but the bytes is OK. */
    uint8* bytes = (uint8*)(ir1->info->bytes);
    if (bytes[0] == X86_PREFIX_REPE) {
        return 1;
    }

    return 0;
}
int ir1_is_iret(IR1_INST *ir1)
{
    IR1_OPCODE opcode = ir1_opcode(ir1);
    return opcode == X86_INS_IRET ||
           opcode == X86_INS_IRETD ;
}
int ir1_is_lmsw(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_LMSW;
}
int ir1_is_retf(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_RETF;
}
int ir1_is_clts(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_CLTS;
}
int ir1_is_call_far(IR1_INST *ir1)
{
    return ir1->info->id == X86_INS_LCALL;
}
int ir1_is_invlpg(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_INVLPG;
}
int ir1_is_pause(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_PAUSE;
}
int ir1_is_sysenter(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_SYSENTER;
}
int ir1_is_sysexit(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_SYSEXIT;
}
int ir1_is_xrstor(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_XRSTOR;
}
int ir1_is_xsetbv(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_XSETBV;
}
int ir1_is_mwait(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_MWAIT;
}
int ir1_is_vmrun(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_VMRUN;
}
int ir1_is_stgi(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_STGI;
}
/* EOB for instruction might change FPU state */
int ir1_contains_fldenv(IR1_INST *ir1)
{
    IR1_OPCODE opc = ir1_opcode(ir1);
    return opc == X86_INS_FLDENV  ||
           opc == X86_INS_FRSTOR  ||
           opc == X86_INS_FXRSTOR ||
           opc == X86_INS_XRSTOR;
}
int ir1_is_fninit(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_FNINIT;
}
int ir1_is_fnsave(IR1_INST *ir1)
{
    return ir1_opcode(ir1) == X86_INS_FNSAVE;
}
#endif

int ir1_is_jump(IR1_INST *ir1) { return ir1->info->id == X86_INS_JMP; }

int ir1_is_call(IR1_INST *ir1)
{
    return ir1->info->id == X86_INS_CALL;
}

int ir1_is_return(IR1_INST *ir1)
{
#ifndef CONFIG_SOFTMMU
    return ir1->info->id == X86_INS_RET || ir1->info->id == X86_INS_IRET ||
           ir1->info->id == X86_INS_RETF;
#else
    /* 'iret' and 'retf' is special EOB in sys-mode.
     * In current version, it depends on the added jmp.
     * Can be further optimized to use gen_exit_tb().
     * Take care of EIP, eflags, mapping registers, etc. */
    return ir1->info->id == X86_INS_RET;
#endif
}

int ir1_is_indirect(IR1_INST *ir1)
{
    lsassert(0);
    // TODO : some indirect call/jmp
    return ir1->info->id == X86_INS_LJMP || ir1->info->id == X86_INS_LCALL;
}

int ir1_is_syscall(IR1_INST *ir1)
{
    // TODO : 0x80?
    return ir1->info->id == X86_INS_INT;
    // X86_INS_SYSCALL,X86_INS_SYSENTER,X86_INS_SYSEXIT,X86_INS_SYSRET
}
bool ir1_is_tb_ending(IR1_INST *ir1)
{
    if (ir1_opcode(ir1) == X86_INS_CALL &&
        !ir1_is_indirect_call(ir1) &&
        ir1_addr_next(ir1) == ir1_target_addr(ir1)) {
        /* call next */
        return false;
    }
    return ir1_is_branch(ir1) ||
           ir1_is_jump(ir1)   ||
#ifdef CONFIG_SOFTMMU
           ir1_is_jump_far(ir1)   ||
#endif
           ir1_is_call(ir1)   ||
           ir1_is_return(ir1) ||
           ir1_is_syscall(ir1);
}

bool ir1_is_cf_use(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_use, 1 << 0);
}
bool ir1_is_pf_use(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_use, 1 << 1);
}
bool ir1_is_af_use(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_use, 1 << 2);
}
bool ir1_is_zf_use(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_use, 1 << 3);
}
bool ir1_is_sf_use(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_use, 1 << 4);
}
bool ir1_is_of_use(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_use, 1 << 5);
}

bool ir1_is_cf_def(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_def, 1 << 0);
}
bool ir1_is_pf_def(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_def, 1 << 1);
}
bool ir1_is_af_def(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_def, 1 << 2);
}
bool ir1_is_zf_def(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_def, 1 << 3);
}
bool ir1_is_sf_def(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_def, 1 << 4);
}
bool ir1_is_of_def(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_def, 1 << 5);
}

uint8_t ir1_get_eflag_use(IR1_INST *ir1) { return ir1->_eflag_use; }
uint8_t ir1_get_eflag_def(IR1_INST *ir1) { return ir1->_eflag_def; }
void ir1_set_eflag_use(IR1_INST *ir1, uint8_t use) { ir1->_eflag_use = use; }
void ir1_set_eflag_def(IR1_INST *ir1, uint8_t def) { ir1->_eflag_def = def; }

uint8_t ir1_get_eflag_inherent_use(IR1_INST *ir1)
{
    lsassert(0);
    uint8_t tmpef = 0;
    x86_insn op = ir1_opcode(ir1);
    if (op == X86_INS_FCMOVU || op == X86_INS_FCMOVNU) {
        return (uint8_t)1 << PF_USEDEF_BIT_INDEX;
    }
    if (op == X86_INS_FCMOVB || op == X86_INS_FCMOVNB) {
        return (uint8_t)1 << CF_USEDEF_BIT_INDEX;
    }
    if (op == X86_INS_FCMOVE || op == X86_INS_FCMOVNE) {
        return (uint8_t)1 << ZF_USEDEF_BIT_INDEX;
    }
    if (op == X86_INS_FCMOVBE || op == X86_INS_FCMOVNBE) {
        return (uint8_t)1 << ZF_USEDEF_BIT_INDEX & (uint8_t)1
                                                       << CF_USEDEF_BIT_INDEX;
    }

    if (X86_EFLAGS_TEST_CF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << CF_USEDEF_BIT_INDEX;
    }
    if (X86_EFLAGS_TEST_ZF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << ZF_USEDEF_BIT_INDEX;
    }
    if (X86_EFLAGS_TEST_AF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << AF_USEDEF_BIT_INDEX;
    }
    if (X86_EFLAGS_TEST_PF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << PF_USEDEF_BIT_INDEX;
    }
    if (X86_EFLAGS_TEST_SF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << SF_USEDEF_BIT_INDEX;
    }
    if (X86_EFLAGS_TEST_OF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << OF_USEDEF_BIT_INDEX;
    }
    if (X86_EFLAGS_TEST_DF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << DF_USEDEF_BIT_INDEX;
    }
    return tmpef;
}
uint8_t ir1_get_eflag_inherent_def(IR1_INST *ir1)
{
    lsassert(0);
    uint8_t tmpef = 0;
    // some fpu ins not provide eflag info

    x86_insn op = ir1_opcode(ir1);
    if (op == X86_INS_FCOMI || op == X86_INS_FCOMIP || op == X86_INS_FUCOMI ||
        op == X86_INS_FUCOMIP || op == X86_INS_COMISS || op == X86_INS_COMISD ||
        op == X86_INS_UCOMISS || op == X86_INS_UCOMISD) {
        return (uint8_t)0x3f;
    }

    if ((X86_EFLAGS_MODIFY_CF | X86_EFLAGS_RESET_CF | X86_EFLAGS_SET_CF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (CF_USEDEF_BIT_INDEX);
    }
    if ((X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_RESET_ZF | X86_EFLAGS_SET_ZF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (ZF_USEDEF_BIT_INDEX);
    }
    if ((X86_EFLAGS_MODIFY_AF | X86_EFLAGS_RESET_AF | X86_EFLAGS_SET_AF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (AF_USEDEF_BIT_INDEX);
    }
    if ((X86_EFLAGS_MODIFY_PF | X86_EFLAGS_RESET_PF | X86_EFLAGS_SET_PF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (PF_USEDEF_BIT_INDEX);
    }
    if ((X86_EFLAGS_MODIFY_SF | X86_EFLAGS_RESET_SF | X86_EFLAGS_SET_SF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (SF_USEDEF_BIT_INDEX);
    }
    if ((X86_EFLAGS_MODIFY_OF | X86_EFLAGS_RESET_OF | X86_EFLAGS_SET_OF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (OF_USEDEF_BIT_INDEX);
    }
    if ((X86_EFLAGS_MODIFY_DF | X86_EFLAGS_RESET_DF | X86_EFLAGS_SET_DF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (DF_USEDEF_BIT_INDEX);
    }
    return tmpef;
}
uint8_t ir1_get_eflag_inherent_undef(IR1_INST *ir1)
{
    lsassert(0);
    uint8_t tmpef = 0;
    if (X86_EFLAGS_UNDEFINED_CF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (CF_USEDEF_BIT_INDEX);
    }
    if (X86_EFLAGS_UNDEFINED_ZF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (ZF_USEDEF_BIT_INDEX);
    }
    if (X86_EFLAGS_UNDEFINED_AF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (AF_USEDEF_BIT_INDEX);
    }
    if (X86_EFLAGS_UNDEFINED_PF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (PF_USEDEF_BIT_INDEX);
    }
    if (X86_EFLAGS_UNDEFINED_SF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (SF_USEDEF_BIT_INDEX);
    }
    if (X86_EFLAGS_UNDEFINED_OF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (OF_USEDEF_BIT_INDEX);
    }

    return tmpef;
}

IR1_OPND *ir1_get_src_opnd(IR1_INST *ir1, int i)
{
    int src_index = -1;
    for (int j = 0; j < ir1_get_opnd_num(ir1); j++) {
        IR1_OPND *tmp = ir1_get_opnd(ir1, j);
        if ((tmp->access & CS_AC_READ) != 0 ||
            ir1_opnd_type(tmp) == X86_OP_IMM) {
            src_index++;
            if (src_index == i) {
                return tmp;
            }
        }
    }
    return ir1_get_src_opnd_implicit(ir1, i - src_index - 1);
}
IR1_OPND *ir1_get_dest_opnd(IR1_INST *ir1, int i)
{
    int dest_index = -1;
    for (int j = 0; j < ir1_get_opnd_num(ir1); j++) {
        IR1_OPND *tmp = ir1_get_opnd(ir1, j);
        if ((tmp->access & CS_AC_WRITE) != 0) {
            dest_index++;
            if (dest_index == i) {
                return tmp;
            }
        }
    }
    return ir1_get_dest_opnd_implicit(ir1, i - dest_index - 1);
}
IR1_OPND *ir1_get_src_opnd_implicit(IR1_INST *ir1, int i)
{
    switch (ir1_opcode(ir1)) {
    /* 1. no implicit operand, or only one implicit operand */
    case X86_INS_PUSH:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_DAA:
        return i == 0 ? &al_ir1_opnd : NULL;
    case X86_INS_DAS:
        return i == 0 ? &al_ir1_opnd : NULL;
    case X86_INS_AAA:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_AAS:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_CBW:
        return i == 0 ? &al_ir1_opnd : NULL;
    case X86_INS_CWDE:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_CDQE:
        return i == 0 ? &eax_ir1_opnd : NULL;
    case X86_INS_CWD:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_CDQ:
        return i == 0 ? &eax_ir1_opnd : NULL;
    case X86_INS_CQO:
        return i == 0 ? &rax_ir1_opnd : NULL;
    case X86_INS_PUSHF:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_SAHF:
        return i == 0 ? &ah_ir1_opnd : NULL;
    case X86_INS_ENTER:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_AAM:
        return i == 0 ? &al_ir1_opnd : NULL;
    case X86_INS_AAD:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_CALL:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_RDMSR:
        return i == 0 ? &ecx_ir1_opnd : NULL;
    case X86_INS_RDPMC:
        return i == 0 ? &ecx_ir1_opnd : NULL;
    /* 2. more than one implicit operand */
    case X86_INS_POP:
    case X86_INS_POPF:
    case X86_INS_POPAL:
    case X86_INS_POPAW:
    case X86_INS_RET:
        if (i == 0) {
            return &esp_ir1_opnd;
        } else if (i == 1) {
            return &esp_mem32_ir1_opnd;  // POPA?
        } else {
            return NULL;
        }
    case X86_INS_IMUL:
    case X86_INS_MUL:
        if (ir1_opnd_num(ir1) == 1 && i == 0) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return &al_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &eax_ir1_opnd;
            } else {
                return &rax_ir1_opnd;
            }
        } else {
            return NULL;
        }
    case X86_INS_IDIV:
    case X86_INS_DIV:
        if (i == 0) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &eax_ir1_opnd;
            } else {
                return &rax_ir1_opnd;
            }
        } else if (i == 1) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return NULL;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &dx_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &edx_ir1_opnd;
            } else {
                return &rdx_ir1_opnd;
            }
        } else {
            return NULL;
        }
    case X86_INS_LEAVE:
        if (i == 0) {
            return &ebp_ir1_opnd;
        } else if (i == 1) {
            return &ebp_mem32_ir1_opnd;
        } else {
            return NULL;
        }
    case X86_INS_CMPXCHG:
        if (i == 0) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return &al_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &eax_ir1_opnd;
            } else {
                return &rax_ir1_opnd;
            }
        } else {
            return NULL;
        }
    case X86_INS_CPUID:
        if (i == 0) {
            return &eax_ir1_opnd;
        } else if (i == 1) {
            return &ecx_ir1_opnd;
        } else {
            return NULL;
        }
    default:
        //lsassertm(
        //    t->_opcode_type == IR1_OPCODE_FPU ||
        //        t->_opcode_type == IR1_OPCODE_SSE,
        //    "the implicit source operand information is not available for %s\n",
        //    ir1_name(ir1_opcode(ir1)));
        return NULL;
    }
}

IR1_OPND *ir1_get_dest_opnd_implicit(IR1_INST *ir1, int i)
{
    switch (ir1_opcode(ir1)) {
    case X86_INS_POP:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_DAA:
        return i == 0 ? &al_ir1_opnd : NULL;
    case X86_INS_DAS:
        return i == 0 ? &al_ir1_opnd : NULL;
    case X86_INS_AAA:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_AAS:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_CBW:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_CWDE:
        return i == 0 ? &eax_ir1_opnd : NULL;
    case X86_INS_CDQE:
        return i == 0 ? &rax_ir1_opnd : NULL;
    case X86_INS_POPF:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_LAHF:
        return i == 0 ? &ah_ir1_opnd : NULL;
    case X86_INS_RET:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_AAM:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_AAD:
        return i == 0 ? &ax_ir1_opnd : NULL;

    case X86_INS_PUSH:
    case X86_INS_PUSHF:
    case X86_INS_PUSHAL:
    case X86_INS_PUSHAW:
    case X86_INS_CALL:
        if (i == 0) {
            return &esp_ir1_opnd;
        } else if (i == 1) {
            return &esp_mem32_ir1_opnd;
        } else {
            return NULL;
        }
    case X86_INS_IMUL:
    case X86_INS_MUL:
        if (i == 0) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &eax_ir1_opnd;
            } else {
                return &rax_ir1_opnd;
            }
        } else if (i == 1) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return NULL;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &dx_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &edx_ir1_opnd;
            } else {
                return &rdx_ir1_opnd;
            }
        } else {
            return NULL;
        }
    case X86_INS_IDIV:
    case X86_INS_DIV:
        if (i == 0) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &eax_ir1_opnd;
            } else {
                return &rax_ir1_opnd;
            }
        } else if (i == 1) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return NULL;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &dx_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &edx_ir1_opnd;
            } else {
                return &rdx_ir1_opnd;
            }
        } else {
            return NULL;
        }
    case X86_INS_LEAVE:
        if (i == 0) {
            return &esp_ir1_opnd;
        } else if (i == 1) {
            return &ebp_ir1_opnd;
        } else {
            return NULL;
        }
    case X86_INS_CMPXCHG:
        if (i == 0) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return &al_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &eax_ir1_opnd;
            } else {
                return &rax_ir1_opnd;
            }
        } else {
            return NULL;
        }
    case X86_INS_RDTSC:
        if (i == 0) {
            return &eax_ir1_opnd;
        } else if (i == 1) {
            return &edx_ir1_opnd;
        } else {
            return NULL;
        }
    case X86_INS_CPUID:
        if (i == 0) {
            return &eax_ir1_opnd;
        } else if (i == 1) {
            return &ebx_ir1_opnd;
        } else if (i == 2) {
            return &ecx_ir1_opnd;
        } else if (i == 3) {
            return &edx_ir1_opnd;
        } else {
            return NULL;
        }
    case X86_INS_CDQ:
        if (i == 0) {
            return NULL;
        } else if (i == 1) {
            return &edx_ir1_opnd;
        } else {
            return NULL;
        }
    default:
        // lsassertm(
        //     t->_opcode_type == IR1_OPCODE_FPU ||
        //         t->_opcode_type == IR1_OPCODE_SSE,
        //     "the implicit dest operand information is not available for
        //     %s\n", ir1_name(ir1_opcode(ir1)));
        return NULL;
    }
    //     {IR1_OPCODE_NONE, 1, 0, __esp, -1},   /* x86_pop implicit operand */
    //     {IR1_OPCODE_NONE, 0, 0, __al, __al},  /* x86_daa implicit operand */
    //     {IR1_OPCODE_NONE, 0, 0, __al, __al},  /* x86_das implicit operand */
    //     {IR1_OPCODE_NONE, 0, 0, __ax, __ax},  /* x86_aaa implicit operands,
    //     AL/AH */ {IR1_OPCODE_NONE, 0, 0, __ax, __ax},  /* x86_aas implicit
    //     operands, AL/AH */ {IR1_OPCODE_NONE, 0, 0, __ax, __al},  /* x86_cbw
    //     implicit operands */ {IR1_OPCODE_NONE, 0, 0, __eax, __ax}, /*
    //     x86_cwde implicit operands */ {IR1_OPCODE_NONE, 0, 0, __rax, __eax},
    //     /* x86_cdqe implicit operands */ {IR1_OPCODE_NONE, 0, 0, __esp, -1},
    //     /* x86_popf implicit operands */ {IR1_OPCODE_NONE, 0, 0, __ah, 0}, /*
    //     x86_lahf implicit operands */ {IR1_OPCODE_RET, 0, 0, __esp, -1}, /*
    //     x86_ret implicit operands */ {IR1_OPCODE_NONE, 0, 0, __ax, __al}, /*
    //     x86_aam implicit operands */ {IR1_OPCODE_NONE, 0, 0, __ax, __ax}, /*
    //     x86_aad implicit operands */
}

void ir1_make_ins_JMP(IR1_INST *ir1, ADDRX addr, int32 off)
{
    uint8_t buf[5];
    buf[0] = 0xe9;
    int count = 0;
    cs_insn *info = NULL;
#ifdef CONFIG_SOFTMMU
    TRANSLATION_DATA *td = lsenv->tr_data;
    if (td->sys.code32) {
        *((uint32_t *)(buf + 1)) = (uint32_t)(off - 5);
#ifdef CONFIG_XTM_MD_CAPSTONE
        count = xtm_disasm_x86(handle, buf, 15, (uint64_t)addr, 1, &info);
#else
        count = cs_disasm(handle, buf, 15, (uint64_t)addr, 1, &info);
#endif
    } else {
        *((uint32_t *)(buf + 1)) = (uint32_t)(off - 3);
#ifdef CONFIG_XTM_MD_CAPSTONE
        count = xtm_disasm_x86(handle16, buf, 15, (uint64_t)addr, 1, &info);
#else
        count = cs_disasm(handle16, buf, 15, (uint64_t)addr, 1, &info);
#endif
    }
#else
    *((uint32_t *)(buf + 1)) = (uint32_t)(off - 5);
    info = cs_malloc(handle);
    count = cs_disasm(handle, buf, 15, (uint64_t)addr, 1, &info);
#endif
    lsassert(count == 1);
    ir1->info = info;
#ifndef CONFIG_XTM_MD_CAPSTONE
    ir1->info_count = 1;
#endif
}

void ir1_make_ins_ILLEGAL(IR1_INST *ir1, ADDRX addr, int size, int flags)
{
    cs_insn *info = NULL;
#ifdef CONFIG_SOFTMMU
#ifdef CONFIG_XTM_MD_CAPSTONE
    info = xtm_alloc_cs_insn();
    info->detail = xtm_alloc_cs_detail();
#else
    TRANSLATION_DATA *td = lsenv->tr_data;
    if (td->sys.code32) {
        info = cs_malloc(handle);
    } else {
        info = cs_malloc(handle16);
    }
#endif
    info->id = X86_INS_INVALID;
    info->address = addr;
    info->mnemonic[0] = 'i';
    info->mnemonic[1] = 'l';
    info->mnemonic[2] = 'l';
    info->mnemonic[3] = 'e';
    info->mnemonic[4] = 'g';
    info->mnemonic[5] = 'a';
    info->mnemonic[6] = 'l';
    info->mnemonic[7] = 0;
    info->size = size;
    info->op_str[0] = 0;
    info->detail->x86.addr_size = -1;
    info->detail->x86.op_count = 0;
#else
    lsassert(0);
#endif
    ir1->flags = flags;
    ir1->info = info;
#ifndef CONFIG_XTM_MD_CAPSTONE
    ir1->info_count = 1;
#endif
}

void ir1_make_ins_NOP(IR1_INST *ir1, ADDRX addr) { lsassert(0); }
void ir1_make_ins_RET(IR1_INST *ir1, ADDRX addr) { lsassert(0); }
void ir1_make_ins_LIBFUNC(IR1_INST *ir1, ADDRX addr) { lsassert(0); }

static const char *ir1_get_grp_name(uint8_t grp)
{
    static const char *ir1_grp_name[] = {
        /* 0 - 7 */
        "invalid", "jump", "call", "ret", "int",
        "iret", "privilege", "branch-relative",
        /* 8 - */
        "VM",    "3DNOW",  "AES",    "ADX",     "AVX",
        "AVX2",  "AVX512", "BMI",    "BMI2",    "CMOV",
        "F16C",  "FMA",    "FMA4",   "FSGBASE", "HLE",
        "MMX",   "MODE32", "MODE64", "RTM",     "SHA",
        "SSE1",  "SSE2",   "SSE3",   "SSE41",   "SSE42",
        "SSE4A", "SSSE3",  "PCLMUL", "XOP",     "CDI",
        "ERI",   "TBM",    "16BITMODE",         "NOT16BITMODE",
        "SGX",   "DQI",    "BWI",    "PFI",     "VLE",
        "SMAP",  "NOVLX",  "FPU"
    };

    if (grp < 8) {
        return ir1_grp_name[grp];
    }
    else{
        return ir1_grp_name[grp - 120];
    }

}

int ir1_dump(IR1_INST *ir1)
{
    int i = 0;
    fprintf(stderr, "0x%" PRIx64 ":\t%s\t\t%s\t;\tbytes[%d]={", ir1->info->address,
            ir1->info->mnemonic, ir1->info->op_str, ir1->info->size);
    for (i = 0; i < ir1->info->size - 1; ++i) {
        fprintf(stderr, "%#x,", ir1->info->bytes[i]);
    }
    fprintf(stderr, "%#x}. ", ir1->info->bytes[i]);

    int grp_nr = ir1_grp_nr(ir1);
    uint8_t *grps = ir1_get_grps(ir1);
    fprintf(stderr, "GRP{");
    for (i = 0; i < grp_nr - 1; ++i) {
        fprintf(stderr, "%d(%s),", grps[i], ir1_get_grp_name(grps[i]));
    }
    fprintf(stderr, "%d(%s)", grps[i], ir1_get_grp_name(grps[i]));
    fprintf(stderr, "}.\n");
    return 0;
}

int ir1_opcode_dump(IR1_INST *ir1)
{
    fprintf(stderr, "0x%" PRIx64 ":\t%s\n", ir1->info->address,
            ir1->info->mnemonic);
    return 0;
}

const char * ir1_name(IR1_OPCODE op){
    return cs_insn_name(handle, op);
}

const char * ir1_group_name(x86_insn_group grp){
    return cs_group_name(handle, grp);
}

void ir1_dump_eflag(IR1_INST *ir1) { lsassert(0); }

uint8_t ir1_get_opnd_num(IR1_INST *ir1)
{
    return ir1->info->detail->x86.op_count;
}

IR1_OPND *ir1_get_opnd(IR1_INST *ir1, int i)
{
    lsassert(i < ir1->info->detail->x86.op_count);
    return &(ir1->info->detail->x86.operands[i]);
}

bool ir1_is_indirect_call(IR1_INST *ir1)
{
    return !ir1_opnd_is_imm(ir1_get_opnd(ir1, 0));
}
bool ir1_is_indirect_jmp(IR1_INST *ir1)
{
    return !ir1_opnd_num(ir1) || !ir1_opnd_is_imm(ir1_get_opnd(ir1, 0));
}

const char *ir1_reg_name(x86_reg reg) { return cs_reg_name(handle, reg); }

int ir1_addr_size(IR1_INST *ir1)
{
    return ir1->info->detail->x86.addr_size;
}

int ir1_inst_size(IR1_INST *ir1)
{
    return ir1->info->size;
}
int ir1_is_push(IR1_INST *ir1){
    IR1_OPCODE op = ir1_opcode(ir1);
    return op == X86_INS_PUSH || op == X86_INS_PUSHAL || op == X86_INS_PUSHAW ||
           op == X86_INS_PUSHF || op == X86_INS_PUSHFD || op == X86_INS_PUSHFQ;
}
int ir1_is_pop(IR1_INST *ir1){
    IR1_OPCODE op = ir1_opcode(ir1);
    return op == X86_INS_POP || op == X86_INS_POPAL || op == X86_INS_POPAW ||
           op == X86_INS_POPF || op == X86_INS_POPFD || op == X86_INS_POPFQ;
}
int ir1_is_push_or_pop(IR1_INST *ir1){
    return ir1_is_push(ir1) || ir1_is_pop(ir1);
}
int ir1_get_segment_override_prefix(IR1_INST *ir1){
    return ir1->info->detail->x86.prefix[1];
}
int ir1_segment_override_prefix_to_index(IR1_PREFIX prefix){
    switch (prefix) {
    case X86_PREFIX_CS:
        return cs_index;
    case X86_PREFIX_SS:
        return ss_index;
    case X86_PREFIX_DS:
        return ds_index;
    case X86_PREFIX_ES:
        return es_index;
    case X86_PREFIX_FS:
        return fs_index;
    case X86_PREFIX_GS:
        return gs_index;
    default:
        lsassert(0);
    }
}

#ifdef CONFIG_SOFTMMU

int ir1_opnd_is_cr(IR1_OPND *opnd)
{
    if(ir1_opnd_type(opnd) != X86_OP_REG) {
        return 0;
    }

    switch (opnd->reg) {
        case X86_REG_CR0:
        case X86_REG_CR1:
        case X86_REG_CR2:
        case X86_REG_CR3:
        case X86_REG_CR4:
        case X86_REG_CR5:
        case X86_REG_CR6:
        case X86_REG_CR7:
        case X86_REG_CR8:
        case X86_REG_CR9:
        case X86_REG_CR10:
        case X86_REG_CR11:
        case X86_REG_CR12:
        case X86_REG_CR13:
        case X86_REG_CR14:
        case X86_REG_CR15:
            return 1;
        default:
            return 0;
    }
}

int ir1_opnd_get_cr_num(IR1_OPND *opnd, int *flag)
{
    switch (opnd->reg) {
        case X86_REG_CR0:
        case X86_REG_CR2:
        case X86_REG_CR3:
        case X86_REG_CR4:
        case X86_REG_CR8:
            *flag = 1; /* OK to read/write CR */
        case X86_REG_CR1:
        case X86_REG_CR5:
        case X86_REG_CR6:
        case X86_REG_CR7:
        case X86_REG_CR9:
        case X86_REG_CR10:
        case X86_REG_CR11:
        case X86_REG_CR12:
        case X86_REG_CR13:
        case X86_REG_CR14:
        case X86_REG_CR15:
            return opnd->reg - X86_REG_CR0;
        default:
            return -1;
    }
}

int ir1_opnd_is_dr(IR1_OPND *opnd)
{
    if(ir1_opnd_type(opnd) != X86_OP_REG) {
        return 0;
    }

    switch (opnd->reg) {
        case X86_REG_DR0:
        case X86_REG_DR1:
        case X86_REG_DR2:
        case X86_REG_DR3:
        case X86_REG_DR4:
        case X86_REG_DR5:
        case X86_REG_DR6:
        case X86_REG_DR7:
        case X86_REG_DR8:
        case X86_REG_DR9:
        case X86_REG_DR10:
        case X86_REG_DR11:
        case X86_REG_DR12:
        case X86_REG_DR13:
        case X86_REG_DR14:
        case X86_REG_DR15:
            return 1;
        default:
            return 0;
    }
}

int ir1_opnd_get_dr_num(IR1_OPND *opnd)
{
    switch (opnd->reg) {
        case X86_REG_DR0:
        case X86_REG_DR1:
        case X86_REG_DR2:
        case X86_REG_DR3:
        case X86_REG_DR4:
        case X86_REG_DR5:
        case X86_REG_DR6:
        case X86_REG_DR7:
        case X86_REG_DR8:
        case X86_REG_DR9:
        case X86_REG_DR10:
        case X86_REG_DR11:
        case X86_REG_DR12:
        case X86_REG_DR13:
        case X86_REG_DR14:
        case X86_REG_DR15:
            return opnd->reg - X86_REG_DR0;
        default:
            return -1;
    }
}

int ir1_mem_opnd_is_base_imm(IR1_OPND *opnd)
{
    if (!ir1_opnd_has_index(opnd) && ir1_opnd_has_base(opnd) &&
        !ir1_opnd_has_seg(opnd)) {
        return 1;
    }
    return 0;
}

int ir1_is_lea(IR1_INST *ir1)
{
    return ir1->info->id == X86_INS_LEA;
}
int ir1_has_prefix_lock(IR1_INST *ir1)
{
    return ir1->info->detail->x86.prefix[0] == X86_PREFIX_LOCK;
}
int ir1_has_prefix_opsize(IR1_INST *ir1)
{
    return ir1->info->detail->x86.prefix[2] == X86_PREFIX_OPSIZE;
}
int ir1_has_prefix_rep(IR1_INST *ir1)
{
    return ir1->info->detail->x86.prefix[0] == X86_PREFIX_REP;
}
int ir1_has_prefix_repe(IR1_INST *ir1)
{
    return ir1->info->detail->x86.prefix[0] == X86_PREFIX_REPE;
}
int ir1_has_prefix_repne(IR1_INST *ir1)
{
    return ir1->info->detail->x86.prefix[0] == X86_PREFIX_REPNE;
}
int ir1_data_size(IR1_INST *ir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    int prefix_opsize = ir1_has_prefix_opsize(ir1);
    if (td->sys.code32 ^ prefix_opsize) {
        /*  code32 && !prefix_opsize
         * !code32 &&  prefix_opsize  */
        return 4 << 3;
    } else {
        /*  code32 &&  prefix_opsize
         * !code32 && !prefix_opsize  */
        return 2 << 3;
    }
}
uint8 *ir1_inst_opbytes(IR1_INST *ir1)
{
    return ir1->info->detail->x86.opcode;
}
int ir1_is_string_op(IR1_INST *ir1)
{
    uint8 *opbytes = ir1_inst_opbytes(ir1);
    uint8 opc = opbytes[0];
    return (0x6c <= opc && opc <= 0x6f) || /* ins/outs       */
           (0xa4 <= opc && opc <= 0xa7) || /* movs/cmps      */
           (0xaa <= opc && opc <= 0xaf) ;  /* stos/lods/scas */
}
int ir1_is_illegal(IR1_INST* ir1){
    return ir1->info->id == X86_INS_INVALID;
}

#endif
