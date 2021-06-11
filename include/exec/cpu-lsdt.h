#ifndef CPU_LSDT_H_LY8H0INC
#define CPU_LSDT_H_LY8H0INC

#include "cpu-all.h"

typedef target_ulong abi_ptr;

// FIXME take care of this
// they are defined in cputlb.c
//
// code are copied from 6.0 version kernel
// so, take care of what I am doing
uint32_t cpu_lduw_le_data_ra(CPUArchState *env, abi_ptr ptr, uintptr_t ra);
int cpu_ldsw_le_data_ra(CPUArchState *env, abi_ptr ptr, uintptr_t ra);
uint32_t cpu_ldl_le_data_ra(CPUArchState *env, abi_ptr ptr, uintptr_t ra);
uint64_t cpu_ldq_le_data_ra(CPUArchState *env, abi_ptr ptr, uintptr_t ra);

void cpu_stw_le_data(CPUArchState *env, abi_ptr ptr, uint32_t val);
void cpu_stl_le_data(CPUArchState *env, abi_ptr ptr, uint32_t val);
void cpu_stq_le_data(CPUArchState *env, abi_ptr ptr, uint64_t val);

void cpu_stw_le_data(CPUArchState *env, abi_ptr ptr, uint32_t val);
void cpu_stl_le_data(CPUArchState *env, abi_ptr ptr, uint32_t val);
void cpu_stq_le_data(CPUArchState *env, abi_ptr ptr, uint64_t val);

void cpu_stw_le_data_ra(CPUArchState *env, abi_ptr ptr,
                        uint32_t val, uintptr_t ra);
void cpu_stl_le_data_ra(CPUArchState *env, abi_ptr ptr,
                        uint32_t val, uintptr_t ra);
void cpu_stq_le_data_ra(CPUArchState *env, abi_ptr ptr,
                        uint64_t val, uintptr_t ra);

# define cpu_lduw_data        cpu_lduw_le_data
# define cpu_ldsw_data        cpu_ldsw_le_data
# define cpu_ldl_data         cpu_ldl_le_data
# define cpu_ldq_data         cpu_ldq_le_data
# define cpu_lduw_data_ra     cpu_lduw_le_data_ra
# define cpu_ldsw_data_ra     cpu_ldsw_le_data_ra
# define cpu_ldl_data_ra      cpu_ldl_le_data_ra
# define cpu_ldq_data_ra      cpu_ldq_le_data_ra
# define cpu_lduw_mmuidx_ra   cpu_lduw_le_mmuidx_ra
# define cpu_ldsw_mmuidx_ra   cpu_ldsw_le_mmuidx_ra
# define cpu_ldl_mmuidx_ra    cpu_ldl_le_mmuidx_ra
# define cpu_ldq_mmuidx_ra    cpu_ldq_le_mmuidx_ra
# define cpu_stw_data         cpu_stw_le_data
# define cpu_stl_data         cpu_stl_le_data
# define cpu_stq_data         cpu_stq_le_data
# define cpu_stw_data_ra      cpu_stw_le_data_ra
# define cpu_stl_data_ra      cpu_stl_le_data_ra
# define cpu_stq_data_ra      cpu_stq_le_data_ra
# define cpu_stw_mmuidx_ra    cpu_stw_le_mmuidx_ra
# define cpu_stl_mmuidx_ra    cpu_stl_le_mmuidx_ra
# define cpu_stq_mmuidx_ra    cpu_stq_le_mmuidx_ra

uint32_t cpu_ldub_code(CPUArchState *env, abi_ptr addr);
uint32_t cpu_lduw_code(CPUArchState *env, abi_ptr addr);
uint32_t cpu_ldl_code(CPUArchState *env, abi_ptr addr);
uint64_t cpu_ldq_code(CPUArchState *env, abi_ptr addr);

static inline int cpu_ldsb_code(CPUArchState *env, abi_ptr addr)
{
    return (int8_t)cpu_ldub_code(env, addr);
}

static inline int cpu_ldsw_code(CPUArchState *env, abi_ptr addr)
{
    return (int16_t)cpu_lduw_code(env, addr);
}

uint32_t cpu_ldub_mmuidx_ra(CPUArchState *env, abi_ptr addr,
                            int mmu_idx, uintptr_t ra);
int cpu_ldsb_mmuidx_ra(CPUArchState *env, abi_ptr addr,
                       int mmu_idx, uintptr_t ra);

uint32_t cpu_lduw_be_mmuidx_ra(CPUArchState *env, abi_ptr addr,
                               int mmu_idx, uintptr_t ra);
int cpu_ldsw_be_mmuidx_ra(CPUArchState *env, abi_ptr addr,
                          int mmu_idx, uintptr_t ra);
uint32_t cpu_ldl_be_mmuidx_ra(CPUArchState *env, abi_ptr addr,
                              int mmu_idx, uintptr_t ra);
uint64_t cpu_ldq_be_mmuidx_ra(CPUArchState *env, abi_ptr addr,
                              int mmu_idx, uintptr_t ra);

uint32_t cpu_lduw_le_mmuidx_ra(CPUArchState *env, abi_ptr addr,
                               int mmu_idx, uintptr_t ra);
int cpu_ldsw_le_mmuidx_ra(CPUArchState *env, abi_ptr addr,
                          int mmu_idx, uintptr_t ra);
uint32_t cpu_ldl_le_mmuidx_ra(CPUArchState *env, abi_ptr addr,
                              int mmu_idx, uintptr_t ra);
uint64_t cpu_ldq_le_mmuidx_ra(CPUArchState *env, abi_ptr addr,
                              int mmu_idx, uintptr_t ra);

void cpu_stb_mmuidx_ra(CPUArchState *env, abi_ptr addr, uint32_t val,
                       int mmu_idx, uintptr_t retaddr);

void cpu_stw_be_mmuidx_ra(CPUArchState *env, abi_ptr addr, uint32_t val,
                          int mmu_idx, uintptr_t retaddr);
void cpu_stl_be_mmuidx_ra(CPUArchState *env, abi_ptr addr, uint32_t val,
                          int mmu_idx, uintptr_t retaddr);
void cpu_stq_be_mmuidx_ra(CPUArchState *env, abi_ptr addr, uint64_t val,
                          int mmu_idx, uintptr_t retaddr);

void cpu_stw_le_mmuidx_ra(CPUArchState *env, abi_ptr addr, uint32_t val,
                          int mmu_idx, uintptr_t retaddr);
void cpu_stl_le_mmuidx_ra(CPUArchState *env, abi_ptr addr, uint32_t val,
                          int mmu_idx, uintptr_t retaddr);
void cpu_stq_le_mmuidx_ra(CPUArchState *env, abi_ptr addr, uint64_t val,
                          int mmu_idx, uintptr_t retaddr);

#endif /* end of include guard: CPU_LSDT_H_LY8H0INC */
