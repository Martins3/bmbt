#ifndef LOG_FOR_TRACE_H_YGFATEDC
#define LOG_FOR_TRACE_H_YGFATEDC

// FIXME a quick fix, review this file later
/* Private global variable, don't use */
extern int qemu_loglevel;

#define LOG_TRACE          (1 << 15)

/* Returns true if a bit is set in the current loglevel mask */
static inline bool qemu_loglevel_mask(int mask)
{
    return (qemu_loglevel & mask) != 0;
}

#endif /* end of include guard: LOG_FOR_TRACE_H_YGFATEDC */
