#ifndef SEG_HELPER_H_MPE7LV6B
#define SEG_HELPER_H_MPE7LV6B

//#define DEBUG_PCALL

#ifdef DEBUG_PCALL
# define LOG_PCALL(...) qemu_log_mask(CPU_LOG_PCALL, ## __VA_ARGS__)
# define LOG_PCALL_STATE(cpu)                                  \
    log_cpu_state_mask(CPU_LOG_PCALL, (cpu), CPU_DUMP_CCOP)
#else
# define LOG_PCALL(...) do { } while (0)
# define LOG_PCALL_STATE(cpu) do { } while (0)
#endif


/*
 * TODO: Convert callers to compute cpu_mmu_index_kernel once
 * and use *_mmuidx_ra directly.
 */
#define cpu_ldub_kernel_ra(e, p, r) \
    cpu_ldub_mmuidx_ra(e, p, cpu_mmu_index_kernel(e), r)
#define cpu_lduw_kernel_ra(e, p, r) \
    cpu_lduw_mmuidx_ra(e, p, cpu_mmu_index_kernel(e), r)
#define cpu_ldl_kernel_ra(e, p, r) \
    cpu_ldl_mmuidx_ra(e, p, cpu_mmu_index_kernel(e), r)
#define cpu_ldq_kernel_ra(e, p, r) \
    cpu_ldq_mmuidx_ra(e, p, cpu_mmu_index_kernel(e), r)

#define cpu_stb_kernel_ra(e, p, v, r) \
    cpu_stb_mmuidx_ra(e, p, v, cpu_mmu_index_kernel(e), r)
#define cpu_stw_kernel_ra(e, p, v, r) \
    cpu_stw_mmuidx_ra(e, p, v, cpu_mmu_index_kernel(e), r)
#define cpu_stl_kernel_ra(e, p, v, r) \
    cpu_stl_mmuidx_ra(e, p, v, cpu_mmu_index_kernel(e), r)
#define cpu_stq_kernel_ra(e, p, v, r) \
    cpu_stq_mmuidx_ra(e, p, v, cpu_mmu_index_kernel(e), r)

#define cpu_ldub_kernel(e, p)    cpu_ldub_kernel_ra(e, p, 0)
#define cpu_lduw_kernel(e, p)    cpu_lduw_kernel_ra(e, p, 0)
#define cpu_ldl_kernel(e, p)     cpu_ldl_kernel_ra(e, p, 0)
#define cpu_ldq_kernel(e, p)     cpu_ldq_kernel_ra(e, p, 0)

#define cpu_stb_kernel(e, p, v)  cpu_stb_kernel_ra(e, p, v, 0)
#define cpu_stw_kernel(e, p, v)  cpu_stw_kernel_ra(e, p, v, 0)
#define cpu_stl_kernel(e, p, v)  cpu_stl_kernel_ra(e, p, v, 0)
#define cpu_stq_kernel(e, p, v)  cpu_stq_kernel_ra(e, p, v, 0)

#endif /* end of include guard: SEG_HELPER_H_MPE7LV6B */
