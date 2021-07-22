#ifndef LOG_H_DLREVUT7
#define LOG_H_DLREVUT7

#include "../hw/core/cpu.h"
#include "log-for-trace.h"

/* Return the number of characters emitted.  */
int qemu_log(const char *fmt, ...);
bool qemu_log_in_addr_range(uint64_t addr);

/* log only if a bit is set on the current loglevel mask:
 * @mask: bit to check in the mask
 * @fmt: printf-style format string
 * @args: optional arguments for format string
 */
#define qemu_log_mask(MASK, FMT, ...)                                          \
  do {                                                                         \
    if (unlikely(qemu_loglevel_mask(MASK))) {                                  \
      qemu_log(FMT, ##__VA_ARGS__);                                            \
    }                                                                          \
  } while (0)

#define qemu_log_mask_and_addr(MASK, ADDR, FMT, ...)                           \
  do {                                                                         \
        if (unlikely(qemu_loglevel_mask(MASK)) &&       \
                     qemu_log_in_addr_range(ADDR)) {    \
            qemu_log(FMT, ## __VA_ARGS__);              \
        }                                               \
  } while (0)

#define CPU_LOG_TB_OUT_ASM (1 << 0)
#define CPU_LOG_TB_IN_ASM (1 << 1)
#define CPU_LOG_TB_OP (1 << 2)
#define CPU_LOG_TB_OP_OPT (1 << 3)
#define CPU_LOG_INT (1 << 4)
#define CPU_LOG_EXEC (1 << 5)
#define CPU_LOG_PCALL (1 << 6)
#define CPU_LOG_TB_CPU (1 << 8)
#define CPU_LOG_RESET (1 << 9)
#define LOG_UNIMP (1 << 10)
#define LOG_GUEST_ERROR (1 << 11)
#define CPU_LOG_MMU (1 << 12)
#define CPU_LOG_TB_NOCHAIN (1 << 13)
#define CPU_LOG_PAGE (1 << 14)
/* LOG_TRACE (1 << 15) is defined in log-for-trace.h */
#define CPU_LOG_TB_OP_IND (1 << 16)
#define CPU_LOG_TB_FPU (1 << 17)
#define CPU_LOG_PLUGIN (1 << 18)

void log_cpu_state(CPUState *cpu, int flags);

/**
 * log_cpu_state_mask:
 * @mask: Mask when to log.
 * @cpu: The CPU whose state is to be logged.
 * @flags: Flags what to log.
 *
 * Logs the output of cpu_dump_state() if loglevel includes @mask.
 */
static inline void log_cpu_state_mask(int mask, CPUState *cpu, int flags) {
  if (qemu_loglevel & mask) {
    log_cpu_state(cpu, flags);
  }
}

#endif /* end of include guard: LOG_H_DLREVUT7 */
