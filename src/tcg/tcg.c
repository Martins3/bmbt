#include "tcg.h"
#include "../../include/exec/cpu-all.h"
#include "../../include/exec/cpu-defs.h"
#include "../../include/exec/exec-all.h"
#include "../../include/exec/tb-hash.h"
#include "../../include/exec/tb-lookup.h"
#include "../../include/hw/boards.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/qemu/atomic.h"
#include "../../include/qemu/main-loop.h"
#include "../../include/qemu/osdep.h"
#include "../../include/qemu/thread.h"
#include "../../include/sysemu/replay.h"
#include "../../include/types.h"
#include "../i386/LATX/x86tomips-config.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define TCG_HIGHWATER 1024

#define CPU_TEMP_BUF_NLONGS 128

#include "glib_stub.h"
struct tcg_region_tree {
  QemuMutex lock;
  GTree *tree;
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

/* Generate global QEMU prologue and epilogue code */
static void tcg_target_qemu_prologue(TCGContext *s) {
  int i;

#ifdef LOONGARCH_DEBUG
  printf("Start tcg_target_qemu_prologue.\n");
#endif

#ifdef CONFIG_LATX
  i = target_x86_to_mips_static_codes(s->code_ptr);
  s->code_ptr += i;
#ifdef BMBT
  s->code_gen_prologue = (void *)context_switch_bt_to_native;
  s->code_gen_epilogue = (void *)context_switch_native_to_bt;
#endif
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

static int32_t encode_imm12(uint32_t imm) { return (imm & 0xfff) << 10; }

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

/*LoongArch for B and BL insn*/
static int32_t encode_uj(LoongarchInsn opc, uint32_t imm) {
  int32_t insn = 0;
  int32_t offset = 0;

  tcg_debug_assert((imm & 3) == 0);

  insn |= (opc << 26);
  offset |= ((imm >> 2) & 0xFFFF) << 10;
  offset |= ((imm >> 2) & 0x3FF0000) >> 16;
  insn |= offset;

  return insn;
}

void tb_target_set_jmp_target(uintptr_t tc_ptr, uintptr_t jmp_addr,
                              uintptr_t addr) {
  ptrdiff_t offset = addr - jmp_addr;
  tcg_insn_unit insn;
  if (offset == sextract64(offset, 0, 28)) {
    insn = encode_uj(OPC_B, offset & 0xFFFFFFF);
  } else {
    tcg_debug_assert(0);
  }
  /*
   * Update insn with new address.
   */
  atomic_set((uint32_t *)jmp_addr, insn);
  flush_icache_range(jmp_addr, jmp_addr + 4);
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
#ifdef BMBT
  s->code_gen_prologue = buf0;
#endif

  /* Compute a high-water mark, at which we voluntarily flush the buffer
     and start over.  The size here is arbitrary, significantly larger
     than we expect the code generation for any one opcode to require.  */
  s->code_gen_highwater = s->code_gen_buffer + (total_size - TCG_HIGHWATER);

  /* Generate the prologue.  */
  tcg_target_qemu_prologue(s);

  buf1 = s->code_ptr;
  flush_icache_range((uintptr_t)buf0, (uintptr_t)buf1);

  /* Deduct the prologue from the buffer.  */
  prologue_size = tcg_current_code_size(s);
  s->code_gen_ptr = buf1;
  s->code_gen_buffer = buf1;
  s->code_buf = buf1;
  total_size -= prologue_size;
  s->code_gen_buffer_size = total_size;

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

  // code_gen_epilogue is usuelss
#ifdef BMBT
  /* Assert that goto_ptr is implemented completely.  */
  if (TCG_TARGET_HAS_goto_ptr) {
    tcg_debug_assert(s->code_gen_epilogue != NULL);
  }
#endif
}

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

static void alloc_tcg_plugin_context(TCGContext *s) {}

void tcg_context_init(TCGContext *s) {
  memset(s, 0, sizeof(*s));

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
  MachineState *ms = qdev_get_machine();
  unsigned int max_cpus = ms->smp.max_cpus;
  tcg_ctxs = g_new(TCGContext *, max_cpus);
#endif

  tcg_debug_assert(!tcg_regset_test_reg(s->reserved_regs, TCG_AREG0));
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
  MachineState *ms = qdev_get_machine();
  unsigned int max_cpus = ms->smp.max_cpus;
  duck_check(max_cpus == 1);
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

/*
 * All TCG threads except the parent (i.e. the one that called tcg_context_init
 * and registered the target's TCG globals) must register with this function
 * before initiating translation.
 *
 * In user-mode we just point tcg_ctx to tcg_init_ctx. See the documentation
 * of tcg_region_init() for the reasoning behind this.
 *
 * In softmmu each caller registers its context in tcg_ctxs[]. Note that in
 * softmmu tcg_ctxs[] does not track tcg_ctx_init, since the initial context
 * is not used anymore for translation once this function is called.
 *
 * Not tracking tcg_init_ctx in tcg_ctxs[] in softmmu keeps code that iterates
 * over the array (e.g. tcg_code_size() the same for both softmmu and user-mode.
 */
void tcg_register_thread(void) {
  MachineState *ms = qdev_get_machine();
  TCGContext *s = g_malloc(sizeof(*s));
  unsigned int i, n;
  bool err;

  *s = tcg_init_ctx;

  /* Claim an entry in tcg_ctxs */
  n = atomic_fetch_inc(&n_tcg_ctxs);
  g_assert(n < ms->smp.max_cpus);
  atomic_set(&tcg_ctxs[n], s);

  if (n > 0) {
    alloc_tcg_plugin_context(s);
  }

  tcg_ctx = s;
  qemu_mutex_lock(&region.lock);
  err = tcg_region_initial_alloc__locked(tcg_ctx);
  g_assert(!err);
  qemu_mutex_unlock(&region.lock);
}
