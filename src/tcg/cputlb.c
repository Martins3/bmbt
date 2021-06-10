#include "../../include/exec/cpu-defs.h"
#include "../../include/exec/memop.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/types.h"
#include "tcg.h"

tb_page_addr_t get_page_addr_code(CPUArchState *env, target_ulong addr) {
  // TODO
  // return get_page_addr_code_hostp(env, addr, NULL);
 return 0;
}

// FIXME __attribute__ is simplified, need more investigation
static inline void  __attribute__((always_inline))
store_helper(CPUArchState *env, target_ulong addr, uint64_t val,
             TCGMemOpIdx oi, uintptr_t retaddr, MemOp op)
{
  
}

void helper_ret_stb_mmu(CPUArchState *env, target_ulong addr, uint8_t val,
                        TCGMemOpIdx oi, uintptr_t retaddr) {
  store_helper(env, addr, val, oi, retaddr, MO_UB);
}
#ifdef CONFIG_X86toMIPS
void xtm_helper_ret_stb_mmu(CPUArchState *env, target_ulong addr, uint8_t val,
                            TCGMemOpIdx oi) {
  store_helper(env, addr, val, oi, GETPC(), MO_UB);
}
#endif

void helper_le_stw_mmu(CPUArchState *env, target_ulong addr, uint16_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr) {
  store_helper(env, addr, val, oi, retaddr, MO_LEUW);
}
#ifdef CONFIG_X86toMIPS
void xtm_helper_le_stw_mmu(CPUArchState *env, target_ulong addr, uint16_t val,
                           TCGMemOpIdx oi) {
  store_helper(env, addr, val, oi, GETPC(), MO_LEUW);
}
#endif

void helper_be_stw_mmu(CPUArchState *env, target_ulong addr, uint16_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr) {
  store_helper(env, addr, val, oi, retaddr, MO_BEUW);
}

void helper_le_stl_mmu(CPUArchState *env, target_ulong addr, uint32_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr) {
  store_helper(env, addr, val, oi, retaddr, MO_LEUL);
}
#ifdef CONFIG_X86toMIPS
void xtm_helper_le_stl_mmu(CPUArchState *env, target_ulong addr, uint32_t val,
                           TCGMemOpIdx oi) {
  store_helper(env, addr, val, oi, GETPC(), MO_LEUL);
}
#endif

void helper_be_stl_mmu(CPUArchState *env, target_ulong addr, uint32_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr) {
  store_helper(env, addr, val, oi, retaddr, MO_BEUL);
}

void helper_le_stq_mmu(CPUArchState *env, target_ulong addr, uint64_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr) {
  store_helper(env, addr, val, oi, retaddr, MO_LEQ);
}
#ifdef CONFIG_X86toMIPS
void xtm_helper_le_stq_mmu(CPUArchState *env, target_ulong addr, uint64_t val,
                           TCGMemOpIdx oi) {
  store_helper(env, addr, val, oi, GETPC(), MO_LEQ);
}
#endif

void helper_be_stq_mmu(CPUArchState *env, target_ulong addr, uint64_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr) {
  store_helper(env, addr, val, oi, retaddr, MO_BEQ);
}
