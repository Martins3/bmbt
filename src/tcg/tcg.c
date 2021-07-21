#include "tcg.h"
#include "../../include/exec/cpu-all.h"
#include "../../include/exec/cpu-defs.h"
#include "../../include/exec/exec-all.h"
#include "../../include/exec/tb-hash.h"
#include "../../include/exec/tb-lookup.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/qemu/atomic.h"
#include "../../include/qemu/osdep.h"
#include "../../include/qemu/thread-posix.h"
#include "../../include/sysemu/cpus.h"
#include "../../include/sysemu/replay.h"
#include "../../include/types.h"
#include "../i386/LATX/x86tomips-config.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define qemu_mutex_lock(m) ({})
#define qemu_mutex_unlock(m) ({})

#define TCG_HIGHWATER 1024

#define CPU_TEMP_BUF_NLONGS 128

#include "glib_stub.h"
struct tcg_region_tree {
  QemuMutex lock;
  struct GTree *tree;
  /* padding to avoid false sharing is computed at run-time */
};

#if UINTPTR_MAX == UINT32_MAX
#define ELF_CLASS ELFCLASS32
#else
#define ELF_CLASS ELFCLASS64
#endif
#ifdef HOST_WORDS_BIGENDIAN
#define ELF_DATA ELFDATA2MSB
#else
#define ELF_DATA ELFDATA2LSB
#endif

#include "../../include/qemu/elf.h"

/* Stack frame parameters.  */
#define REG_SIZE (TCG_TARGET_REG_BITS / 8)
#define SAVE_SIZE ((int)ARRAY_SIZE(tcg_target_callee_save_regs) * REG_SIZE)
#define TEMP_SIZE (CPU_TEMP_BUF_NLONGS * (int)sizeof(long))
#define FRAME_SIZE                                                             \
  ((TCG_STATIC_CALL_ARGS_SIZE + TEMP_SIZE + SAVE_SIZE +                        \
    TCG_TARGET_STACK_ALIGN - 1) &                                              \
   -TCG_TARGET_STACK_ALIGN)
#define SAVE_OFS (TCG_STATIC_CALL_ARGS_SIZE + TEMP_SIZE)

/*
 * We divide code_gen_buffer into equally-sized "regions" that TCG threads
 * dynamically allocate from as demand dictates. Given appropriate region
 * sizing, this minimizes flushes even when some TCG threads generate a lot
 * more code than others.
 */
struct tcg_region_state {
  QemuMutex lock;

  /* fields set at init time */
  void *start;
  void *start_aligned;
  void *end;
  size_t n;
  size_t size;   /* size of one region */
  size_t stride; /* .size + guard size */

  /* fields protected by the lock */
  size_t current;       /* current region index */
  size_t agg_size_full; /* aggregate size of full regions */
};
/*
 * LoongArch ISA opcodes
 */
typedef enum {
  /* two reg, op: [31:10], 9:5 rj, 4:0 rd */
  OPC_CLOW = 0x04,      /*LoongArch 0000000000000000000100 0x04 CLO.W*/
  OPC_CLZW = 0x05,      /*LoongArch 0000000000000000000101 0x05 CLZ.W*/
  OPC_CTOW = 0x06,      /*LoongArch 0000000000000000000110 0x06 CTO.W*/
  OPC_CTZW = 0x07,      /*LoongArch 0000000000000000000111 0x07 CTZ.W*/
  OPC_CLOD = 0x08,      /*LoongArch 0000000000000000001000 0x08 CLO.D*/
  OPC_CLZD = 0x09,      /*LoongArch 0000000000000000000101 0x09 CLZ.D*/
  OPC_CTOD = 0x0A,      /*LoongArch 0000000000000000001010 0x0A CTO.D*/
  OPC_CTZD = 0x0B,      /*LoongArch 0000000000000000001011 0x0B CTZ.D*/
  OPC_REVB_2H = 0x0C,   /*LoongArch 0000000000000000001100 0x0C REVB.2H */
  OPC_REVB_4H = 0x0D,   /*LoongArch 0000000000000000001101 0x0D REVB.4H */
  OPC_REVB_2W = 0x0E,   /*LoongArch 0000000000000000001110 0x0E REVB.2W */
  OPC_REVB_D = 0x0F,    /*LoongArch 0000000000000000001111 0x0F REVB.D */
  OPC_REVH_2W = 0x10,   /*LoongArch 0000000000000000010000 0x10 REVH.2W */
  OPC_REVH_D = 0x11,    /*LoongArch 0000000000000000010001 0x11 REVH.D */
  OPC_BITREV_4B = 0x12, /*LoongArch 0000000000000000010010 0x12 BITREV.4B */
  OPC_BITREV_8B = 0x13, /*LoongArch 0000000000000000010011 0x13 BITREV.8B */
  OPC_BITREV_W = 0x14,  /*LoongArch 0000000000000000010100 0x14 BITREV.W */
  OPC_BITREV_D = 0x15,  /*LoongArch 0000000000000000010101 0x15 BITREV.D */
  OPC_SEH = 0x16,       /* LoongARCH: 0000000000000000010110 0x16 EXT.W.H */
  OPC_SEB = 0x17,       /* LoongARCH: 0000000000000000010111 0x17 EXT.W.B */

  /* three reg, op: 31:15, 14:10 rk, 9:5 rj, 4:0 rd */
  OPC_ADDW = 0x20,    /*LoongARCH: 00000000000100000 0x20 ADD.W*/
  OPC_ADD = 0x21,     /*LoongARCH: 00000000000100001 0x21 ADD.D*/
  OPC_SUBW = 0x22,    /*LoongARCH: 00000000000100010 0x22 SUB.W*/
  OPC_SUB = 0x23,     /*LoongArch 00000000000100011 0x23 SUB.D*/
  OPC_SLT = 0x24,     /*LoongARCH: 00000000000100100 0x24 SLT */
  OPC_SLTU = 0x25,    /*LoongARCH: 00000000000100101 0x25 SLTU*/
  OPC_MASKEQZ = 0x26, /*LoongArch: 00000000000100110 0x26 SELNEZ*/
  OPC_MASKNEZ = 0x27, /*LoongArch: 00000000000100111 0x27 SELEQZ*/
  OPC_NOR = 0x28,     /*LoongArch: 00000000000101000 0x28 NOR*/
  OPC_AND = 0x29,     /*LoongArch: 00000000000101001 0x29 AND*/
  OPC_OR = 0x2A,      /*LoongARCH: OR: 00000000000101010 0x2A OR*/
  OPC_XOR = 0x2B,     /*LoongARCH: XOR: 00000000000101011 0x2B XOR*/
  OPC_ORN = 0x2C,     /*LoongARCH: ORN: 00000000000101100 0x2C ORN*/
  OPC_ANDN = 0x2D,    /*LoongARCH: ANDN: 00000000000101101 0x2D ANDN*/
  OPC_SLLW = 0x2E,    /*LoongARCH: SLLW: 00000000000101110 0x2E SLL.W*/
  OPC_SRLW = 0x2F,    /*LoongARCH: ANDN: 00000000000101111 0x2F SRL.W*/
  OPC_SRAW = 0x30,    /*LoongARCH: ANDN: 00000000000110000 0x30 SRA.W*/
  OPC_SLL = 0x31,     /*Loonarch: 00000000000110001 0x31 SLL.D*/
  OPC_SRL = 0x32,     /*Loonarch: 00000000000110010 0x32 SRL.D*/
  OPC_SRA = 0x33,     /*Loonarch: 00000000000110011 0x33 SRA.D*/
  OPC_ROTRW = 0x36,   /*00000000000110110 ROTR.W*/
  OPC_ROTRD = 0x37,   /*00000000000110111 ROTR.D*/
  OPC_MULW = 0x38,    /*LoongArch: 00000000000111000 MUL.W*/
  OPC_MULHW = 0x39,   /*LoongArch: 00000000000111001 MULH.W*/
  OPC_MULHWU = 0x3A,  /*00000000000111010 MULHWU*/
  OPC_MUL = 0x3B,     /*LoongArch: 00000000000111011 MUL.D*/
  OPC_MULHD = 0x3C,   /*00000000000111100 MULH.D*/
  OPC_MULHDU = 0x3D,  /*00000000000111101 MULHDU*/
  OPC_DIVW = 0x40,    /*00000000001000000 DIVW*/
  OPC_MODW = 0x41,    /*00000000001000001 MODW*/
  OPC_DIVWU = 0x42,   /*00000000001000010 DIVWU */
  OPC_MODWU = 0x43,   /*00000000001000011 MODWU */
  OPC_DIVD = 0x44,    /*00000000001000100 DIVD */
  OPC_MODD = 0x45,    /*00000000001000101 MODD */
  OPC_DIVDU = 0x46,   /*00000000001000110 DIVU */
  OPC_MODDU = 0x47,   /*00000000001000111 MODDU */

  /* imm12+2reg, 31:22 op, 21:10 imm12, 9:5 rj, 4:0 rd */
  OPC_SLTI = 0x08,   /*LoongARCH: 0000001008 0x8 SLTI*/
  OPC_SLTIU = 0x09,  /*LoongARCH: 0000001001 0x9 SLTIU*/
  OPC_ADDIW = 0x0A,  /*LoongARCH: 0000001010 0xA ADDI.W*/
  OPC_ADDI = 0x0B,   /*LoongARCH: 0000001011  0xB ADDI.D*/
  OPC_LU52ID = 0x0C, /*LoongARCH: 0000001100 0xC LU52I.D */
  OPC_ANDI = 0x0D,   /*LoongARCH: 0000001101 0xD ANDI*/
  OPC_ORI = 0x0E,    /*LoongARCH: 0000001110 0xE ORI*/
  OPC_XORI = 0x0F,   /*LoongARCH: 0000001111 0xF XORI*/
  OPC_LB = 0xA0,     /*LoongArch: 0010100000 0xA0 LD.B*/
  OPC_LH = 0xA1,     /*LoongArch: 0010100001 0xA1 LD.H*/
  OPC_LW = 0xA2,     /*LoongARCH: 0010100010 0xA2 LD.W*/
  OPC_LD = 0xA3,     /*LoongARCH: 0010100011 0xA3 LD.D*/
  OPC_SB = 0xA4,     /*LoongArch: 0010100100 0xA4 ST.B*/
  OPC_SH = 0xA5,     /*LoongArch: 0010100101 0xA5 ST.H*/
  OPC_SW = 0xA6,     /*LoongARCH: 0010100110 0xA6 ST.W*/
  OPC_SD = 0xA7,     /*LoongARCH: 0010100111 0xA7 ST.D*/
  OPC_LBU = 0xA8,    /*LoongARCH: 0010101000 0xA8 LD.BU*/
  OPC_LHU = 0xA9,    /*LoongArch: 0010101001 0xA9 LD.HU*/
  OPC_LWU = 0xAA,    /*LoongArch: 0010101010 0xAA LD.WU*/
  OPC_PREF = 0xAB,   /*LoongArch: 0010101011 0xAB PREF*/
  OPC_FLDS = 0xAC,   /*LoongArch: 0010101100 0xAC FLD.S*/
  OPC_FSTS = 0xAD,   /*LoongArch: 0010101101 0xAD FST.S*/
  OPC_FLDD = 0xAE,   /*LoongArch: 0010101110 0xAE FLD.D*/
  OPC_FSTD = 0xAF,   /*LoongArch: 0010101111 0xAF FST.D*/

  /* branch with 21bit offset and one reg, 31:26 op, 25:10 offs[15:0], 9:5 rj,
     4:0 offs[20:16] */
  OPC_BEQZ = 0x10, /* LoongArch: 010000 0x10*/
  OPC_BNEZ = 0x11, /* LoongArch: 010001 0x11*/

  /* imm16+2reg, 31:26 op, 25:10 imm16, 9:5 rj, 4:0 rd */
  OPC_JIRL = 0x13, /* LoongArch 010011 0x13*/
  OPC_BEQ = 0x16,  /*LoongArch: 010110 0x16*/
  OPC_BNE = 0x17,  /*LoongArch: 010111 0x17*/
  OPC_BLT = 0x18,  /*LoongArch: 011000 0x18*/
  OPC_BGE = 0x19,  /*LoongArch: 011001 0x19*/
  OPC_BLTU = 0x1A, /*LoongArch: 011010 0x1A*/
  OPC_BGEU = 0x1B, /*LoongArch: 011011 0x1B*/

  /* branch with 26bit offset, 31:26 op, 25:10 offs[15:0], 9:0 offs[25:16] */
  OPC_B = 0x14,  /* LoongArch: 010100 => 0x14 instead of OPC_J*/
  OPC_BL = 0x15, /* LoongArch: 010101 => 0x15 instead of OPC_JAL*/

  /* imm5 + 2reg, 31:15 op, 14:10 imm5, 9:5 rj, 4:0 rd */
  OPC_SLLIW = 0x81,  /*LoongARCH: 00000000010000001 0x81 SLLI.W*/
  OPC_SRLIW = 0x89,  /*LoongARCH: 00000000010001001 0x89 SRLI.W*/
  OPC_SRAIW = 0x91,  /*LoongARCH: 00000000010010001 0x91 SRAI.W*/
  OPC_ROTRIW = 0x99, /*LoongARCH: 00000000010011001 0x99 ROTRI.W*/

  /* imm6 + 2reg, 31:16 op, 15:10 imm6, 9:5 rj, 4:0 rd */
  OPC_SLLI = 0x41,  /*LoongARCH: 0000000001000001 0x41 SLLI.D*/
  OPC_SRLI = 0x45,  /*LoongARCH: 0000000001000101 0x45 SRLI.D*/
  OPC_SRAI = 0x49,  /*LoongARCH: 0000000001001001 0x41 SRAI.D*/
  OPC_ROTRI = 0x4d, /*LoongARCH: 0000000001001101 0x45 ROTRI.D*/

  /* 31:21+bit15 op, 20:16 msb, 14:10 lsb, 9:5 rj, 4:0 rd */
  OPC_INS = (0x03 << 21) |
            (0 << 15), /* LoongARCH: BSTRINS.W BS00000000011 0x03 Bit15=0*/
  OPC_EXT = (0x03 << 21) |
            (1 << 15), /* LoongARCH: BSTRPICK.W BS00000000011 0x03 Bit15=1*/

  /* 31:22 op, 21:16 msb, 15:10 lsb, 9:5 rj, 4:0 rd */
  OPC_DINS = 0x02, /* LoongARCH: BSTRINS.D 000000010 0x02 */
  OPC_DEXT = 0x03, /* LoongARCH: BSTRPICK.D 000000011 0x03 */

  OPC_DBAR = 0x70E4, /* LoongARCH: DBAR 00111000011100100 */

  /* imm20 + 1reg, 31:25 op, 24:5 imm20, 4:0 rd */
  OPC_LUI = 0x0A,       /*LoongARCH: 0001010 0xA*/
  OPC_LU32I = 0x0B,     /*LoongARCH: 0001011 0xB*/
  OPC_PCADDI = 0x0C,    /*LoongARCH: 0001100 0xC*/
  OPC_PCALAU12I = 0x0D, /*LoongARCH: 0001101 0xD*/
  OPC_PCADDU12I = 0x0E, /*LoongARCH: 0001110 0xE*/
  OPC_AUIPC = OPC_PCADDU12I,
  OPC_PCADDU18I = 0x0F, /*LoongARCH: 0001111 0xF*/

  /* To avoid SP calculate error*/
  /* Different with Riscv, we need to use different Ops for data load/store*/
  ALIAS_PADD = sizeof(void *) == 4 ? OPC_ADDW : OPC_ADD,
  ALIAS_PADDI = sizeof(void *) == 4 ? OPC_ADDIW : OPC_ADDI,
} LoongarchInsn;

static struct tcg_region_state region;
/*
 * This is an array of struct tcg_region_tree's, with padding.
 * We use void * to simplify the computation of region_trees[i]; each
 * struct is found every tree_size bytes.
 */
static void *region_trees;
static size_t tree_size;
static TCGRegSet tcg_target_available_regs[TCG_TYPE_COUNT];
static TCGRegSet tcg_target_call_clobber_regs;

static void tcg_region_bounds(size_t curr_region, void **pstart, void **pend) {
  void *start, *end;

  start = region.start_aligned + curr_region * region.stride;
  end = start + region.size;

  if (curr_region == 0) {
    start = region.start;
  }
  if (curr_region == region.n - 1) {
    end = region.end;
  }

  *pstart = start;
  *pend = end;
}

static void tcg_region_assign(TCGContext *s, size_t curr_region) {
  void *start, *end;

  tcg_region_bounds(curr_region, &start, &end);

  s->code_gen_buffer = start;
  s->code_gen_ptr = start;
  s->code_gen_buffer_size = end - start;
  s->code_gen_highwater = end - TCG_HIGHWATER;
}

static bool tcg_region_alloc__locked(TCGContext *s) {
  if (region.current == region.n) {
    return true;
  }
  tcg_region_assign(s, region.current);
  region.current++;
  return false;
}

/*
 * Request a new region once the one in use has filled up.
 * Returns true on error.
 */
static bool tcg_region_alloc(TCGContext *s) {
  bool err;
  /* read the region size now; alloc__locked will overwrite it on success */
  size_t size_full = s->code_gen_buffer_size;

  qemu_mutex_lock(&region.lock);
  err = tcg_region_alloc__locked(s);
  if (!err) {
    region.agg_size_full += size_full - TCG_HIGHWATER;
  }
  qemu_mutex_unlock(&region.lock);
  return err;
}

static struct tcg_region_tree *tc_ptr_to_region_tree(void *p) {
  size_t region_idx;

  if (p < region.start_aligned) {
    region_idx = 0;
  } else {
    ptrdiff_t offset = p - region.start_aligned;

    if (offset > region.stride * (region.n - 1)) {
      region_idx = region.n - 1;
    } else {
      region_idx = offset / region.stride;
    }
  }
  return region_trees + region_idx * tree_size;
}

static inline TCGTemp *tcg_temp_alloc(TCGContext *s) {
  int n = s->nb_temps++;
  tcg_debug_assert(n < TCG_MAX_TEMPS);
  return memset(&s->temps[n], 0, sizeof(TCGTemp));
}

static inline TCGTemp *tcg_global_alloc(TCGContext *s) {
  TCGTemp *ts;

  tcg_debug_assert(s->nb_globals == s->nb_temps);
  s->nb_globals++;
  ts = tcg_temp_alloc(s);
  ts->temp_global = 1;

  return ts;
}

static TCGTemp *tcg_global_reg_new_internal(TCGContext *s, TCGType type,
                                            TCGReg reg, const char *name) {
  TCGTemp *ts;

  if (TCG_TARGET_REG_BITS == 32 && type != TCG_TYPE_I32) {
    tcg_abort();
  }

  ts = tcg_global_alloc(s);
  ts->base_type = type;
  ts->type = type;
  ts->fixed_reg = 1;
  ts->reg = reg;
  ts->name = name;
  tcg_regset_set_reg(s->reserved_regs, reg);

  return ts;
}

void tcg_set_frame(TCGContext *s, TCGReg reg, intptr_t start, intptr_t size) {
  s->frame_start = start;
  s->frame_end = start + size;
  s->frame_temp = tcg_global_reg_new_internal(s, TCG_TYPE_PTR, reg, "_frame");
}

/* Generate global QEMU prologue and epilogue code */
static void tcg_target_qemu_prologue(TCGContext *s) {
  int i;

#ifdef LOONGARCH_DEBUG
  printf("Start tcg_target_qemu_prologue.\n");
#endif

  tcg_set_frame(s, TCG_REG_SP, TCG_STATIC_CALL_ARGS_SIZE, TEMP_SIZE);

#ifdef CONFIG_LATX
  i = target_x86_to_mips_static_codes(s->code_ptr);
  s->code_ptr += i;
  s->code_gen_prologue = (void *)context_switch_bt_to_native;
  s->code_gen_epilogue = (void *)context_switch_native_to_bt;
#else
  /* TB prologue */
  tcg_out_opc_imm(s, ALIAS_PADDI, TCG_REG_SP, TCG_REG_SP, -FRAME_SIZE);
  for (i = 0; i < ARRAY_SIZE(tcg_target_callee_save_regs); i++) {
    tcg_out_st(s, TCG_TYPE_REG, tcg_target_callee_save_regs[i], TCG_REG_SP,
               SAVE_OFS + i * REG_SIZE);
  }

#ifndef CONFIG_SOFTMMU
  if (guest_base) {
    tcg_out_movi(s, TCG_TYPE_PTR, TCG_GUEST_BASE_REG, guest_base);
    tcg_regset_set_reg(s->reserved_regs, TCG_GUEST_BASE_REG);
  }
#endif

  /* Call generated code */
  tcg_out_mov(s, TCG_TYPE_PTR, TCG_AREG0, tcg_target_call_iarg_regs[0]);
  // LoongArch
  // tcg_out_opc_imm(s, OPC_JALR, TCG_REG_ZERO, tcg_target_call_iarg_regs[1],
  // 0);
  tcg_out_opc_jirl(s, TCG_REG_ZERO, tcg_target_call_iarg_regs[1], 0);
  /* Return path for goto_ptr. Set return value to 0 */
  s->code_gen_epilogue = s->code_ptr;
  tcg_out_mov(s, TCG_TYPE_REG, TCG_REG_A0, TCG_REG_ZERO);

  /* TB epilogue */
  tb_ret_addr = s->code_ptr;
  for (i = 0; i < ARRAY_SIZE(tcg_target_callee_save_regs); i++) {
    tcg_out_ld(s, TCG_TYPE_REG, tcg_target_callee_save_regs[i], TCG_REG_SP,
               SAVE_OFS + i * REG_SIZE);
  }

  tcg_out_opc_imm(s, ALIAS_PADDI, TCG_REG_SP, TCG_REG_SP, FRAME_SIZE);
  // LoongArch
  // tcg_out_opc_imm(s, OPC_JALR, TCG_REG_ZERO, TCG_REG_RA, 0);
  tcg_out_opc_jirl(s, TCG_REG_ZERO, TCG_REG_RA, 0);
#endif

#ifdef LOONGARCH_DEBUG
  printf("End tcg_target_qemu_prologue.\n");
#endif
}

/* The CIE and FDE header definitions will be common to all hosts.  */
typedef struct {
  uint32_t len __attribute__((aligned((sizeof(void *)))));
  uint32_t id;
  uint8_t version;
  char augmentation[1];
  uint8_t code_align;
  uint8_t data_align;
  uint8_t return_column;
} DebugFrameCIE;

typedef struct QEMU_PACKED {
  uint32_t len __attribute__((aligned((sizeof(void *)))));
  uint32_t cie_offset;
  uintptr_t func_start;
  uintptr_t func_len;
} DebugFrameFDEHeader;

typedef struct QEMU_PACKED {
  DebugFrameCIE cie;
  DebugFrameFDEHeader fde;
} DebugFrameHeader;

static const int tcg_target_callee_save_regs[] = {
    TCG_REG_S0, /* used for the global env (TCG_AREG0) */
    TCG_REG_S1, TCG_REG_S2, TCG_REG_S3, TCG_REG_S4, TCG_REG_S5,
    TCG_REG_S6, TCG_REG_S7, TCG_REG_S8, TCG_REG_RA, /* should be last for ABI
                                                       compliance */
};

typedef struct {
  DebugFrameHeader h;
  uint8_t fde_def_cfa[4];
  uint8_t fde_reg_ofs[ARRAY_SIZE(tcg_target_callee_save_regs) * 2];
} DebugFrame;

#define ELF_HOST_MACHINE EM_LOONGARCH

static const DebugFrame debug_frame = {
    .h.cie.len = sizeof(DebugFrameCIE) - 4, /* length after .len member */
    .h.cie.id = -1,
    .h.cie.version = 1,
    .h.cie.code_align = 1,
    .h.cie.data_align = -(TCG_TARGET_REG_BITS / 8) & 0x7f, /* sleb128 */
    .h.cie.return_column = TCG_REG_RA,

    /* Total FDE size does not include the "len" member.  */
    .h.fde.len = sizeof(DebugFrame) - offsetof(DebugFrame, h.fde.cie_offset),

    .fde_def_cfa = {12, TCG_REG_SP,             /* DW_CFA_def_cfa sp, ... */
                    (FRAME_SIZE & 0x7f) | 0x80, /* ... uleb128 FRAME_SIZE */
                    (FRAME_SIZE >> 7)},
    .fde_reg_ofs = {
        0x80 + 24, 9, /* DW_CFA_offset, s1,  -72 */
        0x80 + 25, 8, /* DW_CFA_offset, s2,  -64 */
        0x80 + 26, 7, /* DW_CFA_offset, s3,  -56 */
        0x80 + 27, 6, /* DW_CFA_offset, s4,  -48 */
        0x80 + 28, 5, /* DW_CFA_offset, s5,  -40 */
        0x80 + 29, 4, /* DW_CFA_offset, s6,  -32 */
        0x80 + 30, 3, /* DW_CFA_offset, s7,  -24 */
        0x80 + 31, 2, /* DW_CFA_offset, s8,  -16 */
        0x80 + 1, 1,  /* DW_CFA_offset, ra,  -8 */
    }};

void tcg_register_jit(void *buf, size_t buf_size) {
  tcg_register_jit_int(buf, buf_size, &debug_frame, sizeof(debug_frame));
}

static int32_t encode_imm12(uint32_t imm) { return (imm & 0xfff) << 10; }

static int32_t encode_i(LoongarchInsn opc, TCGReg rd, TCGReg rs1,
                        uint32_t imm) {
  return (opc << 22) | (rd & 0x1f) | (rs1 & 0x1f) << 5 | encode_imm12(imm);
}

static void tcg_out_nop_fill(tcg_insn_unit *p, int count) {
  int i;
  for (i = 0; i < count; ++i) {
    p[i] = encode_i(OPC_ANDI, TCG_REG_ZERO, TCG_REG_ZERO, 0);
  }
}

/*
 * Relocations
 */
static void reloc_pc_10_16_s2(tcg_insn_unit *pc, tcg_insn_unit *target) {
  intptr_t offs = (intptr_t)target - (intptr_t)pc;
#ifdef LOONGARCH_DEBUG
  printf("[reloc debug]: target = %p, pc = %p, off = 0x%lx, *pc = 0x%x\n",
         target, pc, offs, *pc);
#endif
  /* check 4-aligned */
  tcg_debug_assert(offs % 4 == 0);
  /* check 18-bit signed */
  offs >>= 2;
  tcg_debug_assert(((offs & ~0x7fffU) == 0) || ((offs & ~0x7fffU) == ~0x7fffU));
  /* (*PC) [25 ... 10] = offs [17 ... 2] */
  *pc = deposit32(*pc, 10, 16, offs & 0xffff);
#ifdef LOONGARCH_DEBUG
  printf("[reloc debug]: *pc = 0x%x\n", *pc);
#endif
}

static void reloc_pc_0_10_10_16_s2(tcg_insn_unit *pc, tcg_insn_unit *target) {
  intptr_t offs = (intptr_t)target - (intptr_t)pc;
  /* check 4-aligned */
  tcg_debug_assert(offs % 4 == 0);
  /* check 28-bit signed */
  offs >>= 2;
  tcg_debug_assert(((offs & ~0x1ffffffU) == 0) ||
                   ((offs & ~0x1ffffffU) == ~0x1ffffffU));
  /*
   * (*PC) [9 ... 0] = offs [27 ... 18]
   * (*PC) [25 ... 10] = offs [17 ... 2]
   */
  *pc = deposit32(*pc, 10, 16, offs & 0xffff);
  *pc = deposit32(*pc, 0, 10, (offs & 0x3ff0000) >> 16);
}

static bool patch_reloc(tcg_insn_unit *code_ptr, int type, intptr_t value,
                        intptr_t addend) {

  tcg_debug_assert(addend == 0);
  switch (type) {
  case R_LARCH_SOP_POP_32_S_10_16_S2:
    reloc_pc_10_16_s2(code_ptr, (tcg_insn_unit *)value);
    break;
  case R_LARCH_SOP_POP_32_S_0_10_10_16_S2:
    reloc_pc_0_10_10_16_s2(code_ptr, (tcg_insn_unit *)value);
    break;
  // FIXME Niugenen say xqm doesn't need this, but the code flow has
  // a different idea
  // case R_RISCV_CALL:
  // return reloc_call(code_ptr, (tcg_insn_unit *)value);
  default:
#ifdef LOONGARCH_DEBUG
    printf("[Fix me!!!]: This is called by tcg.c, type is %d\n", type);
#endif
    tcg_abort();
  }
  return true;
}

static int tcg_out_pool_finalize(TCGContext *s) {
  TCGLabelPoolData *p = s->pool_labels;
  TCGLabelPoolData *l = NULL;
  void *a;

  if (p == NULL) {
    return 0;
  }

  /* ??? Round up to qemu_icache_linesize, but then do not round
     again when allocating the next TranslationBlock structure.  */
  a = (void *)ROUND_UP((uintptr_t)s->code_ptr,
                       sizeof(tcg_target_ulong) * p->nlong);
  tcg_out_nop_fill(s->code_ptr, (tcg_insn_unit *)a - s->code_ptr);
  s->data_gen_ptr = a;

  for (; p != NULL; p = p->next) {
    size_t size = sizeof(tcg_target_ulong) * p->nlong;
    if (!l || l->nlong != p->nlong || memcmp(l->data, p->data, size)) {
      if (unlikely(a > s->code_gen_highwater)) {
        return -1;
      }
      memcpy(a, p->data, size);
      a += size;
      l = p;
    }
    if (!patch_reloc(p->label, p->rtype, (intptr_t)a - size, p->addend)) {
      return -2;
    }
  }

  s->code_ptr = a;
  return 0;
}

/*
 * Allocate TBs right before their corresponding translated code, making
 * sure that TBs and code are on different cache lines.
 */
TranslationBlock *tcg_tb_alloc(TCGContext *s) {
  uintptr_t align = qemu_icache_linesize;
  TranslationBlock *tb;
  void *next;

retry:
  tb = (void *)ROUND_UP((uintptr_t)s->code_gen_ptr, align);
  next = (void *)ROUND_UP((uintptr_t)(tb + 1), align);

  if (unlikely(next > s->code_gen_highwater)) {
    if (tcg_region_alloc(s)) {
      return NULL;
    }
    goto retry;
  }
  atomic_set(&s->code_gen_ptr, next);
  s->data_gen_ptr = NULL;
  return tb;
}

void tcg_tb_insert(TranslationBlock *tb) {
  struct tcg_region_tree *rt = tc_ptr_to_region_tree(tb->tc.ptr);

  qemu_mutex_lock(&rt->lock);
  g_tree_insert(rt->tree, &tb->tc, tb);
  qemu_mutex_unlock(&rt->lock);
}

void tcg_tb_remove(TranslationBlock *tb) {
  struct tcg_region_tree *rt = tc_ptr_to_region_tree(tb->tc.ptr);

  qemu_mutex_lock(&rt->lock);
  g_tree_remove(rt->tree, &tb->tc);
  qemu_mutex_unlock(&rt->lock);
}

/*
 * Find the TB 'tb' such that
 * tb->tc.ptr <= tc_ptr < tb->tc.ptr + tb->tc.size
 * Return NULL if not found.
 */
TranslationBlock *tcg_tb_lookup(uintptr_t tc_ptr) {
  struct tcg_region_tree *rt = tc_ptr_to_region_tree((void *)tc_ptr);
  TranslationBlock *tb;
  struct tb_tc s = {.ptr = (void *)tc_ptr};

  qemu_mutex_lock(&rt->lock);
  tb = g_tree_lookup(rt->tree, &s);
  qemu_mutex_unlock(&rt->lock);
  return tb;
}

void tcg_prologue_init(TCGContext *s) {
  size_t prologue_size, total_size;
  void *buf0, *buf1;

  /* Put the prologue at the beginning of code_gen_buffer.  */
  buf0 = s->code_gen_buffer;
  total_size = s->code_gen_buffer_size;
  s->code_ptr = buf0;
  s->code_buf = buf0;
  s->data_gen_ptr = NULL;
  s->code_gen_prologue = buf0;

  /* Compute a high-water mark, at which we voluntarily flush the buffer
     and start over.  The size here is arbitrary, significantly larger
     than we expect the code generation for any one opcode to require.  */
  s->code_gen_highwater = s->code_gen_buffer + (total_size - TCG_HIGHWATER);

#ifdef TCG_TARGET_NEED_POOL_LABELS
  s->pool_labels = NULL;
#endif

  /* Generate the prologue.  */
  tcg_target_qemu_prologue(s);

#ifdef TCG_TARGET_NEED_POOL_LABELS
  /* Allow the prologue to put e.g. guest_base into a pool entry.  */
  {
    int result = tcg_out_pool_finalize(s);
    tcg_debug_assert(result == 0);
  }
#endif

  buf1 = s->code_ptr;
  flush_icache_range((uintptr_t)buf0, (uintptr_t)buf1);

  /* Deduct the prologue from the buffer.  */
  prologue_size = tcg_current_code_size(s);
  s->code_gen_ptr = buf1;
  s->code_gen_buffer = buf1;
  s->code_buf = buf1;
  total_size -= prologue_size;
  s->code_gen_buffer_size = total_size;

  tcg_register_jit(s->code_gen_buffer, total_size);

#ifdef DEBUG_DISAS
  if (qemu_loglevel_mask(CPU_LOG_TB_OUT_ASM)) {
    qemu_log_lock();
    qemu_log("PROLOGUE: [size=%zu]\n", prologue_size);
    if (s->data_gen_ptr) {
      size_t code_size = s->data_gen_ptr - buf0;
      size_t data_size = prologue_size - code_size;
      size_t i;

      log_disas(buf0, code_size);

      for (i = 0; i < data_size; i += sizeof(tcg_target_ulong)) {
        if (sizeof(tcg_target_ulong) == 8) {
          qemu_log("0x%08" PRIxPTR ":  .quad  0x%016" PRIx64 "\n",
                   (uintptr_t)s->data_gen_ptr + i,
                   *(uint64_t *)(s->data_gen_ptr + i));
        } else {
          qemu_log("0x%08" PRIxPTR ":  .long  0x%08x\n",
                   (uintptr_t)s->data_gen_ptr + i,
                   *(uint32_t *)(s->data_gen_ptr + i));
        }
      }
    } else {
      log_disas(buf0, prologue_size);
    }
    qemu_log("\n");
    qemu_log_flush();
    qemu_log_unlock();
  }
#endif

  /* Assert that goto_ptr is implemented completely.  */
  if (TCG_TARGET_HAS_goto_ptr) {
    tcg_debug_assert(s->code_gen_epilogue != NULL);
  }
}

#ifdef ELF_HOST_MACHINE
/* In order to use this feature, the backend needs to do three things:

   (1) Define ELF_HOST_MACHINE to indicate both what value to
       put into the ELF image and to indicate support for the feature.

   (2) Define tcg_register_jit.  This should create a buffer containing
       the contents of a .debug_frame section that describes the post-
       prologue unwind info for the tcg machine.

   (3) Call tcg_register_jit_int, with the constructed .debug_frame.
*/

/* Begin GDB interface.  THE FOLLOWING MUST MATCH GDB DOCS.  */
typedef enum {
  JIT_NOACTION = 0,
  JIT_REGISTER_FN,
  JIT_UNREGISTER_FN
} jit_actions_t;

struct jit_code_entry {
  struct jit_code_entry *next_entry;
  struct jit_code_entry *prev_entry;
  const void *symfile_addr;
  uint64_t symfile_size;
};

struct jit_descriptor {
  uint32_t version;
  uint32_t action_flag;
  struct jit_code_entry *relevant_entry;
  struct jit_code_entry *first_entry;
};

void __jit_debug_register_code(void) __attribute__((noinline));
void __jit_debug_register_code(void) { asm(""); }

/* Must statically initialize the version, because GDB may check
   the version before we can set it.  */
struct jit_descriptor __jit_debug_descriptor = {1, 0, 0, 0};

/* End GDB interface.  */

static int find_string(const char *strtab, const char *str) {
  const char *p = strtab + 1;

  while (1) {
    if (strcmp(p, str) == 0) {
      return p - strtab;
    }
    p += strlen(p) + 1;
  }
}

static void tcg_register_jit_int(void *buf_ptr, size_t buf_size,
                                 const void *debug_frame,
                                 size_t debug_frame_size) {
  struct __attribute__((packed)) DebugInfo {
    uint32_t len;
    uint16_t version;
    uint32_t abbrev;
    uint8_t ptr_size;
    uint8_t cu_die;
    uint16_t cu_lang;
    uintptr_t cu_low_pc;
    uintptr_t cu_high_pc;
    uint8_t fn_die;
    char fn_name[16];
    uintptr_t fn_low_pc;
    uintptr_t fn_high_pc;
    uint8_t cu_eoc;
  };

  struct ElfImage {
    ElfW(Ehdr) ehdr;
    ElfW(Phdr) phdr;
    ElfW(Shdr) shdr[7];
    ElfW(Sym) sym[2];
    struct DebugInfo di;
    uint8_t da[24];
    char str[80];
  };

  struct ElfImage *img;

  static const struct ElfImage img_template = {
      .ehdr =
          {
              .e_ident[EI_MAG0] = ELFMAG0,
              .e_ident[EI_MAG1] = ELFMAG1,
              .e_ident[EI_MAG2] = ELFMAG2,
              .e_ident[EI_MAG3] = ELFMAG3,
              .e_ident[EI_CLASS] = ELF_CLASS,
              .e_ident[EI_DATA] = ELF_DATA,
              .e_ident[EI_VERSION] = EV_CURRENT,
              .e_type = ET_EXEC,
              .e_machine = ELF_HOST_MACHINE,
              .e_version = EV_CURRENT,
              .e_phoff = offsetof(struct ElfImage, phdr),
              .e_shoff = offsetof(struct ElfImage, shdr),
              .e_ehsize = sizeof(ElfW(Shdr)),
              .e_phentsize = sizeof(ElfW(Phdr)),
              .e_phnum = 1,
              .e_shentsize = sizeof(ElfW(Shdr)),
              .e_shnum = ARRAY_SIZE(img->shdr),
              .e_shstrndx = ARRAY_SIZE(img->shdr) - 1,
#ifdef ELF_HOST_FLAGS
              .e_flags = ELF_HOST_FLAGS,
#endif
#ifdef ELF_OSABI
              .e_ident[EI_OSABI] = ELF_OSABI,
#endif
          },
      .phdr =
          {
              .p_type = PT_LOAD,
              .p_flags = PF_X,
          },
      .shdr = {[0] = {.sh_type = SHT_NULL},
               /* Trick: The contents of code_gen_buffer are not present in
                  this fake ELF file; that got allocated elsewhere.  Therefore
                  we mark .text as SHT_NOBITS (similar to .bss) so that readers
                  will not look for contents.  We can record any address.  */
               [1] =
                   {
                       /* .text */
                       .sh_type = SHT_NOBITS,
                       .sh_flags = SHF_EXECINSTR | SHF_ALLOC,
                   },
               [2] =
                   {
                       /* .debug_info */
                       .sh_type = SHT_PROGBITS,
                       .sh_offset = offsetof(struct ElfImage, di),
                       .sh_size = sizeof(struct DebugInfo),
                   },
               [3] =
                   {
                       /* .debug_abbrev */
                       .sh_type = SHT_PROGBITS,
                       .sh_offset = offsetof(struct ElfImage, da),
                       .sh_size = sizeof(img->da),
                   },
               [4] =
                   {
                       /* .debug_frame */
                       .sh_type = SHT_PROGBITS,
                       .sh_offset = sizeof(struct ElfImage),
                   },
               [5] =
                   {
                       /* .symtab */
                       .sh_type = SHT_SYMTAB,
                       .sh_offset = offsetof(struct ElfImage, sym),
                       .sh_size = sizeof(img->sym),
                       .sh_info = 1,
                       .sh_link = ARRAY_SIZE(img->shdr) - 1,
                       .sh_entsize = sizeof(ElfW(Sym)),
                   },
               [6] =
                   {
                       /* .strtab */
                       .sh_type = SHT_STRTAB,
                       .sh_offset = offsetof(struct ElfImage, str),
                       .sh_size = sizeof(img->str),
                   }},
      .sym = {[1] =
                  {
                      /* code_gen_buffer */
                      .st_info = ELF_ST_INFO(STB_GLOBAL, STT_FUNC),
                      .st_shndx = 1,
                  }},
      .di = {.len = sizeof(struct DebugInfo) - 4,
             .version = 2,
             .ptr_size = sizeof(void *),
             .cu_die = 1,
             .cu_lang = 0x8001, /* DW_LANG_Mips_Assembler */
             .fn_die = 2,
             .fn_name = "code_gen_buffer"},
      .da =
          {
              1,         /* abbrev number (the cu) */
              0x11, 1,   /* DW_TAG_compile_unit, has children */
              0x13, 0x5, /* DW_AT_language, DW_FORM_data2 */
              0x11, 0x1, /* DW_AT_low_pc, DW_FORM_addr */
              0x12, 0x1, /* DW_AT_high_pc, DW_FORM_addr */
              0,    0,   /* end of abbrev */
              2,         /* abbrev number (the fn) */
              0x2e, 0,   /* DW_TAG_subprogram, no children */
              0x3,  0x8, /* DW_AT_name, DW_FORM_string */
              0x11, 0x1, /* DW_AT_low_pc, DW_FORM_addr */
              0x12, 0x1, /* DW_AT_high_pc, DW_FORM_addr */
              0,    0,   /* end of abbrev */
              0          /* no more abbrev */
          },
      .str = "\0"
             ".text\0"
             ".debug_info\0"
             ".debug_abbrev\0"
             ".debug_frame\0"
             ".symtab\0"
             ".strtab\0"
             "code_gen_buffer",
  };

  /* We only need a single jit entry; statically allocate it.  */
  static struct jit_code_entry one_entry;

  uintptr_t buf = (uintptr_t)buf_ptr;
  size_t img_size = sizeof(struct ElfImage) + debug_frame_size;
  DebugFrameHeader *dfh;

  img = g_malloc(img_size);
  *img = img_template;

  img->phdr.p_vaddr = buf;
  img->phdr.p_paddr = buf;
  img->phdr.p_memsz = buf_size;

  img->shdr[1].sh_name = find_string(img->str, ".text");
  img->shdr[1].sh_addr = buf;
  img->shdr[1].sh_size = buf_size;

  img->shdr[2].sh_name = find_string(img->str, ".debug_info");
  img->shdr[3].sh_name = find_string(img->str, ".debug_abbrev");

  img->shdr[4].sh_name = find_string(img->str, ".debug_frame");
  img->shdr[4].sh_size = debug_frame_size;

  img->shdr[5].sh_name = find_string(img->str, ".symtab");
  img->shdr[6].sh_name = find_string(img->str, ".strtab");

  img->sym[1].st_name = find_string(img->str, "code_gen_buffer");
  img->sym[1].st_value = buf;
  img->sym[1].st_size = buf_size;

  img->di.cu_low_pc = buf;
  img->di.cu_high_pc = buf + buf_size;
  img->di.fn_low_pc = buf;
  img->di.fn_high_pc = buf + buf_size;

  dfh = (DebugFrameHeader *)(img + 1);
  memcpy(dfh, debug_frame, debug_frame_size);
  dfh->fde.func_start = buf;
  dfh->fde.func_len = buf_size;

#ifdef DEBUG_JIT
  /* Enable this block to be able to debug the ELF image file creation.
     One can use readelf, objdump, or other inspection utilities.  */
  {
    FILE *f = fopen("/tmp/qemu.jit", "w+b");
    if (f) {
      if (fwrite(img, img_size, 1, f) != img_size) {
        /* Avoid stupid unused return value warning for fwrite.  */
      }
      fclose(f);
    }
  }
#endif

  one_entry.symfile_addr = img;
  one_entry.symfile_size = img_size;

  __jit_debug_descriptor.action_flag = JIT_REGISTER_FN;
  __jit_debug_descriptor.relevant_entry = &one_entry;
  __jit_debug_descriptor.first_entry = &one_entry;
  __jit_debug_register_code();
}
#else
/* No support for the feature.  Provide the entry point expected by exec.c,
   and implement the internal function we declared earlier.  */

static void tcg_register_jit_int(void *buf, size_t size,
                                 const void *debug_frame,
                                 size_t debug_frame_size) {}

void tcg_register_jit(void *buf, size_t buf_size) {}
#endif /* ELF_HOST_MACHINE */

static void tcg_region_tree_lock_all(void) {
  size_t i;

  for (i = 0; i < region.n; i++) {
    struct tcg_region_tree *rt = region_trees + i * tree_size;

    qemu_mutex_lock(&rt->lock);
  }
}

static void tcg_region_tree_unlock_all(void) {
  size_t i;

  for (i = 0; i < region.n; i++) {
    struct tcg_region_tree *rt = region_trees + i * tree_size;

    qemu_mutex_unlock(&rt->lock);
  }
}

void tcg_tb_foreach(GTraverseFunc func, gpointer user_data) {
  size_t i;

  tcg_region_tree_lock_all();
  for (i = 0; i < region.n; i++) {
    struct tcg_region_tree *rt = region_trees + i * tree_size;

    g_tree_foreach(rt->tree, func, user_data);
  }
  tcg_region_tree_unlock_all();
}

size_t tcg_nb_tbs(void) {
  size_t nb_tbs = 0;
  size_t i;

  tcg_region_tree_lock_all();
  for (i = 0; i < region.n; i++) {
    struct tcg_region_tree *rt = region_trees + i * tree_size;

    nb_tbs += g_tree_nnodes(rt->tree);
  }
  tcg_region_tree_unlock_all();
  return nb_tbs;
}

static void tcg_region_tree_reset_all(void) {
  size_t i;

  tcg_region_tree_lock_all();
  for (i = 0; i < region.n; i++) {
    struct tcg_region_tree *rt = region_trees + i * tree_size;

    /* Increment the refcount first so that destroy acts as a reset */
    g_tree_ref(rt->tree);
    g_tree_destroy(rt->tree);
  }
  tcg_region_tree_unlock_all();
}

/*
 * Perform a context's first region allocation.
 * This function does _not_ increment region.agg_size_full.
 */
static inline bool tcg_region_initial_alloc__locked(TCGContext *s) {
  return tcg_region_alloc__locked(s);
}

/* Call from a safe-work context */
void tcg_region_reset_all(void) {
  unsigned int n_ctxs = atomic_read(&n_tcg_ctxs);
  unsigned int i;

  qemu_mutex_lock(&region.lock);
  region.current = 0;
  region.agg_size_full = 0;

  for (i = 0; i < n_ctxs; i++) {
    TCGContext *s = atomic_read(&tcg_ctxs[i]);
    bool err = tcg_region_initial_alloc__locked(s);
    g_assert(!err);
  }
  qemu_mutex_unlock(&region.lock);

  tcg_region_tree_reset_all();
}

static const int tcg_target_reg_alloc_order[] = {
    /* Call saved registers */
    /* TCG_REG_S0 reservered for TCG_AREG0 */
    TCG_REG_S1,
    TCG_REG_S2,
    TCG_REG_S3,
    TCG_REG_S4,
    TCG_REG_S5,
    TCG_REG_S6,
    TCG_REG_S7,
    TCG_REG_S8,

    /* Call clobbered registers */
    TCG_REG_T0,
    TCG_REG_T1,
    TCG_REG_T2,
    TCG_REG_T3,
    TCG_REG_T4,
    TCG_REG_T5,
    TCG_REG_T6,

    /* Argument registers */
    TCG_REG_A0,
    TCG_REG_A1,
    TCG_REG_A2,
    TCG_REG_A3,
    TCG_REG_A4,
    TCG_REG_A5,
    TCG_REG_A6,
    TCG_REG_A7,
};

static int indirect_reg_alloc_order[ARRAY_SIZE(tcg_target_reg_alloc_order)];

static void tcg_target_init(TCGContext *s) {
  tcg_target_available_regs[TCG_TYPE_I32] = 0xffffffff;
  if (TCG_TARGET_REG_BITS == 64) {
    tcg_target_available_regs[TCG_TYPE_I64] = 0xffffffff;
  }

  tcg_target_call_clobber_regs = -1u;
  tcg_regset_reset_reg(tcg_target_call_clobber_regs, TCG_REG_S0);
  tcg_regset_reset_reg(tcg_target_call_clobber_regs, TCG_REG_S1);
  tcg_regset_reset_reg(tcg_target_call_clobber_regs, TCG_REG_S2);
  tcg_regset_reset_reg(tcg_target_call_clobber_regs, TCG_REG_S3);
  tcg_regset_reset_reg(tcg_target_call_clobber_regs, TCG_REG_S4);
  tcg_regset_reset_reg(tcg_target_call_clobber_regs, TCG_REG_S5);
  tcg_regset_reset_reg(tcg_target_call_clobber_regs, TCG_REG_S6);
  tcg_regset_reset_reg(tcg_target_call_clobber_regs, TCG_REG_S7);
  tcg_regset_reset_reg(tcg_target_call_clobber_regs, TCG_REG_S8);

  s->reserved_regs = 0;
  tcg_regset_set_reg(s->reserved_regs, TCG_REG_ZERO);
  tcg_regset_set_reg(s->reserved_regs, TCG_REG_TMP0);
  tcg_regset_set_reg(s->reserved_regs, TCG_REG_TMP1);
  tcg_regset_set_reg(s->reserved_regs, TCG_REG_TMP2);
  tcg_regset_set_reg(s->reserved_regs, TCG_REG_TP);
  tcg_regset_set_reg(s->reserved_regs, TCG_REG_SP);
  tcg_regset_set_reg(s->reserved_regs, TCG_REG_X);
  tcg_regset_set_reg(s->reserved_regs, TCG_REG_FP);
}

static void alloc_tcg_plugin_context(TCGContext *s) {}

void tcg_context_init(TCGContext *s) {
  int op, total_args, n, i;
  TCGOpDef *def;
  TCGArgConstraint *args_ct;
  int *sorted_args;
  TCGTemp *ts;

  memset(s, 0, sizeof(*s));
  s->nb_globals = 0;

  /* Count total number of arguments and allocate the corresponding
     space */
  total_args = 0;

  // FIXME why I still take care of tcg_op?
  // why we still need helper table?
#if 0
    for(op = 0; op < NB_OPS; op++) {
        def = &tcg_op_defs[op];
        n = def->nb_iargs + def->nb_oargs;
        total_args += n;
    }

    args_ct = g_malloc(sizeof(TCGArgConstraint) * total_args);
    sorted_args = g_malloc(sizeof(int) * total_args);

    for(op = 0; op < NB_OPS; op++) {
        def = &tcg_op_defs[op];
        def->args_ct = args_ct;
        def->sorted_args = sorted_args;
        n = def->nb_iargs + def->nb_oargs;
        sorted_args += n;
        args_ct += n;
    }

    /* Register helpers.  */
    /* Use g_direct_hash/equal for direct pointer comparisons on func.  */
    helper_table = g_hash_table_new(NULL, NULL);

    for (i = 0; i < ARRAY_SIZE(all_helpers); ++i) {
        g_hash_table_insert(helper_table, (gpointer)all_helpers[i].func,
                            (gpointer)&all_helpers[i]);
    }
#endif

  tcg_target_init(s);
  // FIXME more investigation later
  // process_op_defs(s);

  /* Reverse the order of the saved registers, assuming they're all at
     the start of tcg_target_reg_alloc_order.  */
  for (n = 0; n < ARRAY_SIZE(tcg_target_reg_alloc_order); ++n) {
    int r = tcg_target_reg_alloc_order[n];
    if (tcg_regset_test_reg(tcg_target_call_clobber_regs, r)) {
      break;
    }
  }
  for (i = 0; i < n; ++i) {
    indirect_reg_alloc_order[i] = tcg_target_reg_alloc_order[n - 1 - i];
  }
  for (; i < ARRAY_SIZE(tcg_target_reg_alloc_order); ++i) {
    indirect_reg_alloc_order[i] = tcg_target_reg_alloc_order[i];
  }

  alloc_tcg_plugin_context(s);

  tcg_ctx = s;
  /*
   * In user-mode we simply share the init context among threads, since we
   * use a single region. See the documentation tcg_region_init() for the
   * reasoning behind this.
   * In softmmu we will have at most max_cpus TCG threads.
   */
#ifdef CONFIG_USER_ONLY
  tcg_ctxs = &tcg_ctx;
  n_tcg_ctxs = 1;
#else
  // FIXME ...
  // MachineState *ms = MACHINE(qdev_get_machine());
  // unsigned int max_cpus = ms->smp.max_cpus;
  tcg_ctxs = g_new(TCGContext *, max_cpus);
#endif

  tcg_debug_assert(!tcg_regset_test_reg(s->reserved_regs, TCG_AREG0));
  ts = tcg_global_reg_new_internal(s, TCG_TYPE_PTR, TCG_AREG0, "env");
  cpu_env = temp_tcgv_ptr(ts);
}

#ifdef CONFIG_USER_ONLY
static size_t tcg_n_regions(void) { return 1; }
#else
/*
 * It is likely that some vCPUs will translate more code than others, so we
 * first try to set more regions than max_cpus, with those regions being of
 * reasonable size. If that's not possible we make do by evenly dividing
 * the code_gen_buffer among the vCPUs.
 */
static size_t tcg_n_regions(void) {
  size_t i;

  /* Use a single region if all we have is one vCPU thread */
#if !defined(CONFIG_USER_ONLY)
  unsigned int max_cpus = 1;

  // MachineState *ms = MACHINE(qdev_get_machine());
  // unsigned int max_cpus = ms->smp.max_cpus;
#endif
  if (max_cpus == 1 || !qemu_tcg_mttcg_enabled()) {
    return 1;
  }

  /* Try to have more regions than max_cpus, with each region being >= 2 MB */
  for (i = 8; i > 0; i--) {
    size_t regions_per_thread = i;
    size_t region_size;

    region_size = tcg_init_ctx.code_gen_buffer_size;
    region_size /= max_cpus * regions_per_thread;

    if (region_size >= 2 * 1024u * 1024) {
      return max_cpus * regions_per_thread;
    }
  }
  /* If we can't, then just allocate one region per vCPU thread */
  return max_cpus;
}
#endif

/* compare a pointer @ptr and a tb_tc @s */
static int ptr_cmp_tb_tc(const void *ptr, const struct tb_tc *s) {
  if (ptr >= s->ptr + s->size) {
    return 1;
  } else if (ptr < s->ptr) {
    return -1;
  }
  return 0;
}

static gint tb_tc_cmp(gconstpointer ap, gconstpointer bp) {
  const struct tb_tc *a = ap;
  const struct tb_tc *b = bp;

  /*
   * When both sizes are set, we know this isn't a lookup.
   * This is the most likely case: every TB must be inserted; lookups
   * are a lot less frequent.
   */
  if (likely(a->size && b->size)) {
    if (a->ptr > b->ptr) {
      return 1;
    } else if (a->ptr < b->ptr) {
      return -1;
    }
    /* a->ptr == b->ptr should happen only on deletions */
    g_assert(a->size == b->size);
    return 0;
  }
  /*
   * All lookups have either .size field set to 0.
   * From the glib sources we see that @ap is always the lookup key. However
   * the docs provide no guarantee, so we just mark this case as likely.
   */
  if (likely(a->size == 0)) {
    return ptr_cmp_tb_tc(a->ptr, b);
  }
  return ptr_cmp_tb_tc(b->ptr, a);
}

static void tcg_region_trees_init(void) {
  size_t i;

  tree_size = ROUND_UP(sizeof(struct tcg_region_tree), qemu_dcache_linesize);
  region_trees = qemu_memalign(qemu_dcache_linesize, region.n * tree_size);
  for (i = 0; i < region.n; i++) {
    struct tcg_region_tree *rt = region_trees + i * tree_size;

    qemu_mutex_init(&rt->lock);
    rt->tree = g_tree_new(tb_tc_cmp);
  }
}

/*
 * Initializes region partitioning.
 *
 * Called at init time from the parent thread (i.e. the one calling
 * tcg_context_init), after the target's TCG globals have been set.
 *
 * Region partitioning works by splitting code_gen_buffer into separate regions,
 * and then assigning regions to TCG threads so that the threads can translate
 * code in parallel without synchronization.
 *
 * In softmmu the number of TCG threads is bounded by max_cpus, so we use at
 * least max_cpus regions in MTTCG. In !MTTCG we use a single region.
 * Note that the TCG options from the command-line (i.e. -accel accel=tcg,[...])
 * must have been parsed before calling this function, since it calls
 * qemu_tcg_mttcg_enabled().
 *
 * In user-mode we use a single region.  Having multiple regions in user-mode
 * is not supported, because the number of vCPU threads (recall that each thread
 * spawned by the guest corresponds to a vCPU thread) is only bounded by the
 * OS, and usually this number is huge (tens of thousands is not uncommon).
 * Thus, given this large bound on the number of vCPU threads and the fact
 * that code_gen_buffer is allocated at compile-time, we cannot guarantee
 * that the availability of at least one region per vCPU thread.
 *
 * However, this user-mode limitation is unlikely to be a significant problem
 * in practice. Multi-threaded guests share most if not all of their translated
 * code, which makes parallel code generation less appealing than in softmmu.
 */
void tcg_region_init(void) {
  void *buf = tcg_init_ctx.code_gen_buffer;
  void *aligned;
  size_t size = tcg_init_ctx.code_gen_buffer_size;
  size_t page_size = qemu_real_host_page_size;
  size_t region_size;
  size_t n_regions;
  size_t i;

  n_regions = tcg_n_regions();

  /* The first region will be 'aligned - buf' bytes larger than the others */
  aligned = QEMU_ALIGN_PTR_UP(buf, page_size);
  g_assert(aligned < tcg_init_ctx.code_gen_buffer + size);
  /*
   * Make region_size a multiple of page_size, using aligned as the start.
   * As a result of this we might end up with a few extra pages at the end of
   * the buffer; we will assign those to the last region.
   */
  region_size = (size - (aligned - buf)) / n_regions;
  region_size = QEMU_ALIGN_DOWN(region_size, page_size);

  /* A region must have at least 2 pages; one code, one guard */
  g_assert(region_size >= 2 * page_size);

  /* init the region struct */
  qemu_mutex_init(&region.lock);
  region.n = n_regions;
  region.size = region_size - page_size;
  region.stride = region_size;
  region.start = buf;
  region.start_aligned = aligned;
  /* page-align the end, since its last page will be a guard page */
  region.end = QEMU_ALIGN_PTR_DOWN(buf + size, page_size);
  /* account for that last guard page */
  region.end -= page_size;

  /* set guard pages */
  for (i = 0; i < region.n; i++) {
    void *start, *end;
    int rc;

    tcg_region_bounds(i, &start, &end);
    rc = qemu_mprotect_none(end, page_size);
    g_assert(!rc);
  }

  tcg_region_trees_init();

  /* In user-mode we support only one ctx, so do the initial allocation now */
#ifdef CONFIG_USER_ONLY
  {
    bool err = tcg_region_initial_alloc__locked(tcg_ctx);

    g_assert(!err);
  }
#endif
}
