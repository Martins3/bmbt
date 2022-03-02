#include "tcg-target.h"
#include "latx-config.h"
#include "latx-options.h"
#include "../tcg.h"
#include <stddef.h>

/*
 * Loongarch Base ISA opcodes (IM)
 */

typedef enum {
  /* reg2 */
  OPC_CLZW = 0x05,
  OPC_CTZW = 0x07,
  OPC_CLZD = 0x09,
  OPC_CTZD = 0x0b,
  // OPC_CLZ = TCG_TARGET_REG_BITS == 64 ? OPC_CLZD : OPC_CLZW;
  // OPC_CTZ = TCG_TARGET_REG_BITS == 64 ? OPC_CTZD : OPC_CTZW;
  OPC_EXTWH = 0x16,
  OPC_EXTWB = 0x17,
  /* reg3 */
  OPC_ADDW = 0x20,
  OPC_ADDD = 0x21,
  OPC_ADD = TCG_TARGET_REG_BITS == 64 ? OPC_ADDD : OPC_ADDW,
  OPC_SUBW = 0x22,
  OPC_SUBD = 0x23,
  OPC_SUB = TCG_TARGET_REG_BITS == 64 ? OPC_SUBD : OPC_SUBW,
  OPC_SLT = 0x24,
  OPC_SLTU = 0x25,
  OPC_MASKEQZ = 0x26,
  OPC_MASKNEZ = 0x27,
  OPC_NOR = 0x28,
  OPC_AND = 0x29,
  OPC_OR = 0x2a,
  OPC_XOR = 0x2b,
  OPC_ORN = 0x2c,
  OPC_ANDN = 0x2d,
  OPC_SLLW = 0x2e,
  OPC_SRLW = 0x2f,
  OPC_SRAW = 0x30,
  OPC_SLLD = 0x31,
  OPC_SRLD = 0x32,
  OPC_SRAD = 0x33,
  OPC_SLL = TCG_TARGET_REG_BITS == 64 ? OPC_SLLD : OPC_SLLW,
  OPC_SRL = TCG_TARGET_REG_BITS == 64 ? OPC_SRLD : OPC_SRLW,
  OPC_SRA = TCG_TARGET_REG_BITS == 64 ? OPC_SRAD : OPC_SRAW,
  OPC_ROTRB = 0x34,
  OPC_ROTRH = 0x35,
  OPC_ROTRW = 0x36,
  OPC_ROTRD = 0x37,
  OPC_MULW = 0x38,
  OPC_MULHW = 0x39,
  OPC_MULHWU = 0x3a,
  OPC_MULD = 0x3b,
  OPC_MULHD = 0x3c,
  OPC_MULHDU = 0x3d,
  OPC_MUL = TCG_TARGET_REG_BITS == 64 ? OPC_MULD : OPC_MULW,
  OPC_MULH = TCG_TARGET_REG_BITS == 64 ? OPC_MULHD : OPC_MULHW,
  OPC_MULHU = TCG_TARGET_REG_BITS == 64 ? OPC_MULHDU : OPC_MULHWU,
  OPC_DIVW = 0x40,
  OPC_MODW = 0x41,
  OPC_DIVWU = 0x42,
  OPC_MODWU = 0x43,
  OPC_DIVD = 0x44,
  OPC_MODD = 0x45,
  OPC_DIVDU = 0x46,
  OPC_MODDU = 0x47,
  OPC_DIV = TCG_TARGET_REG_BITS == 64 ? OPC_DIVD : OPC_DIVW,
  OPC_MOD = TCG_TARGET_REG_BITS == 64 ? OPC_MODD : OPC_MODW,
  OPC_DIVU = TCG_TARGET_REG_BITS == 64 ? OPC_DIVDU : OPC_DIVWU,
  OPC_MODU = TCG_TARGET_REG_BITS == 64 ? OPC_MODDU : OPC_MODWU,
  /* reg2 + imm12 */
  OPC_SLTI = 0x8,
  OPC_SLTIU = 0x9,
  OPC_ADDIW = 0xa,
  OPC_ADDID = 0xb,
  OPC_ADDI = TCG_TARGET_REG_BITS == 64 ? OPC_ADDID : OPC_ADDIW,
  OPC_LU52I = 0xc,
  OPC_ANDI = 0xd,
  OPC_ORI = 0xe,
  OPC_XORI = 0xf,
  OPC_LDB = 0xa0,
  OPC_LDH = 0xa1,
  OPC_LDW = 0xa2,
  OPC_LDD = 0xa3,
  OPC_STB = 0xa4,
  OPC_STH = 0xa5,
  OPC_STW = 0xa6,
  OPC_STD = 0xa7,
  OPC_LDBU = 0xa8,
  OPC_LDHU = 0xa9,
  OPC_LDWU = 0xaa,
  /* reg2 + imm16 */
  OPC_JIRL = 0x13,
  OPC_BEQ = 0x16,
  OPC_BNE = 0x17,
  OPC_BLT = 0x18,
  OPC_BGE = 0x19,
  OPC_BLTU = 0x1a,
  OPC_BGEU = 0x1b,
  /* imm26 */
  OPC_B = 0x14,
  OPC_BL = 0x15,
  /* reg1 + imm20 */
  OPC_LU12I = 0xa,
  OPC_LU32I = 0xb,
  OPC_PCADDI = 0xc,
  OPC_PCALAU12I = 0xd,
  OPC_PCADDU12I = 0xe,
  OPC_PCADDU18I = 0xf,
  /* bit-op */
  OPC_SLLIW = 0x81,
  OPC_SLLID = 0x82,
  OPC_SLLI = TCG_TARGET_REG_BITS == 64 ? OPC_SLLID : OPC_SLLIW,
  OPC_SRLIW = 0x89,
  OPC_SRLID = 0x8a,
  OPC_SRLI = TCG_TARGET_REG_BITS == 64 ? OPC_SRLID : OPC_SRLIW,
  OPC_SRAIW = 0x91,
  OPC_SRAID = 0x92,
  OPC_SRAI = TCG_TARGET_REG_BITS == 64 ? OPC_SRAID : OPC_SRAIW,
  // OPC_ROTRID = 0x4d,
  // OPC_ROTRIW = 0x99,
  // OPC_ROTRI = TCG_TARGET_REG_BITS == 64 ? OPC_ROTRID : OPC_ROTRIW;

  OPC_DBAR = 0x70e4,
} LoongarchInsn;

static inline tcg_target_long sextreg(tcg_target_long val, int pos, int len) {
  if (TCG_TARGET_REG_BITS == 32) {
    return sextract32(val, pos, len);
  } else {
    return sextract64(val, pos, len);
  }
}

static int32_t encode_jbimm18(uint32_t imm) { return (imm << 8) & 0x3fffc00; }

static int32_t encode_djimm28(uint32_t imm) {
  return ((imm >> 18) & 0x3ff) | encode_jbimm18(imm);
}

/* Type-DJ (direct jump) */

static int32_t encode_dj(LoongarchInsn opc, uint32_t imm) {
  return encode_djimm28(imm) | opc << 26;
}

void tb_target_set_jmp_target(uintptr_t tc_ptr, uintptr_t jmp_rx,
                              uintptr_t jmp_rw, uintptr_t addr) {
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)
  if (option_large_code_cache) {
    ptrdiff_t jmp_offset = addr - jmp_rx;
    if (jmp_offset == sextreg(jmp_offset, 0, 28)) {
      tcg_insn_unit insn;
      insn = encode_dj(OPC_B, jmp_offset);
      qatomic_set((uint32_t *)jmp_rw, insn);
      flush_idcache_range(jmp_rx, jmp_rw, 4);
    } else {
      tcg_insn_unit i1, i2;
      jmp_offset >>= 2;
      int64_t upper, lower;
      upper = ((jmp_offset + (1 << 15)) >> 16) & 0xfffff;
      lower = (jmp_offset & 0xffff);
      /* $t0 -> temp reg, was not used at end of tb */
      i1 = 0x1e000000 | upper << 5 | TCG_REG_A0;       /* pcaddu18i */
      i2 = 0x4c000000 | lower << 10 | TCG_REG_A0 << 5; /* jirl */
      qatomic_set((uint32_t *)jmp_rw, i1);
      qatomic_set((uint32_t *)(jmp_rw + 4), i2);
      flush_idcache_range(jmp_rx, jmp_rw, 8);
    }
  } else {
#endif
    ptrdiff_t offset = addr - jmp_rx;
    tcg_insn_unit insn;

    if (offset == sextreg(offset, 0, 28)) {
      insn = encode_dj(OPC_B, offset);
      qatomic_set((uint32_t *)jmp_rw, insn);
      flush_idcache_range(jmp_rx, jmp_rw, 4);
    } else {
      /* patch far jmp, two insns were preserved */
      tcg_insn_unit i1, i2;
      offset >>= 2;
      int64_t upper, lower;
      upper = ((offset + (1 << 15)) >> 16) & 0xfffff;
      lower = (offset & 0xffff);
      /* $a0 -> temp reg, was not used at end of tb */
      i1 = 0x1e000000 | upper << 5 | TCG_REG_A0;       /* pcaddu18i */
      i2 = 0x4c000000 | lower << 10 | TCG_REG_A0 << 5; /* jirl */
      qatomic_set((uint32_t *)jmp_rw, i1);
      qatomic_set((uint32_t *)(jmp_rw + 4), i2);
      flush_idcache_range(jmp_rx, jmp_rw, 8);
    }
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)
  }
#endif
}

/* Generate global QEMU prologue and epilogue code */
void tcg_target_qemu_prologue(TCGContext *s) {

  int inst_nr = 0;
#ifdef CONFIG_SOFTMMU
  inst_nr = target_latxs_static_codes(s->code_ptr);
  s->code_ptr += inst_nr;
  return;
#endif
}
