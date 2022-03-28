#ifndef HAMT_MISC_H_MHDHCNLV
#define HAMT_MISC_H_MHDHCNLV

#include <stdint.h>
#include <stdlib.h>
// #include <larchintrin.h>
#include "../../../include/qemu/queue.h"
#include "internal.h"

#define DIRECT_MAPPING(x) (x + 0x9800000000000000)
#define REVDIRECT_MAPPING(x) (x - 0x9800000000000000)

#define INVTLB_CURRENT_GFALSE 0x3

#define PROT_RWX (PROT_READ | PROT_WRITE | PROT_EXEC)

#define TARGET_ADDR_SIZE (1UL << 32)

#define LB_MASK 0xffffffffffffff00
#define LH_MASK 0xffffffffffff0000
#define LW_MASK 0xffffffff00000000

#define MAX_PTE_PAGES 512
#define PTE_PAGE_SIZE 4096

#define MAX_ASID 1024

#define ASID_VERSION_MASK 0xffffffffff00
#define ASID_VALUE_MASK 0xff
#define GET_ASID_VERSION(asid) ((asid & ASID_VERSION_MASK) >> 8)
#define GET_ASID_VALUE(asid) (asid & ASID_VALUE_MASK)
#define FORM_NEW_ASID(version, value)                                          \
  ((version << 8) | (value & ASID_VALUE_MASK))

struct pte_page {
  QLIST_ENTRY(pte_page) entry;
  uint64_t page;
};

struct pgtable_head {

  QLIST_ENTRY(pgtable_head) entry;

  uint64_t cr3_value;

  uint16_t asid;
};

struct htable_ele {

  QLIST_HEAD(, pgtable_head) pgtables_list;

  int pgtable_num;
};

/*
 * memory access type:
 *     LD.{B[U]/H[U]/W[U]/D}, ST.{B/H/W/D}
 *     LDX.{B[U]/H[U]/W[U]/D}, STX.{B/H/W/D}
 */
typedef enum {
  OPC_LD_B = 0b0010100000 << 22,
  OPC_LD_H = 0b0010100001 << 22,
  OPC_LD_W = 0b0010100010 << 22,
  OPC_LD_D = 0b0010100011 << 22,
  OPC_ST_B = 0b0010100100 << 22,
  OPC_ST_H = 0b0010100101 << 22,
  OPC_ST_W = 0b0010100110 << 22,
  OPC_ST_D = 0b0010100111 << 22,
  OPC_LD_BU = 0b0010101000 << 22,
  OPC_LD_HU = 0b0010101001 << 22,
  OPC_LD_WU = 0b0010101010 << 22,
  /*
      OPC_LDX_B    = 0b00111000000000 << 18,
      OPC_LDX_H    = 0b00111000000001 << 18,
      OPC_LDX_W    = 0b00111000000010 << 18,
      OPC_LDX_D    = 0b00111000000011 << 18,
      OPC_STX_B    = 0b00111000000100 << 18,
      OPC_STX_H    = 0b00111000000101 << 18,
      OPC_STX_W    = 0b00111000000110 << 18,
      OPC_STX_D    = 0b00111000000111 << 18,
      OPC_LDX_BU   = 0b00111000001000 << 18,
      OPC_LDX_HU   = 0b00111000001001 << 18,
      OPC_LDX_WU   = 0b00111000001010 << 18,
  */
} ld_st_inst;

// void enable_x86vm_hamt(void);
void hamt_init();

#endif /* end of include guard: HAMT_MISC_H_MHDHCNLV */
