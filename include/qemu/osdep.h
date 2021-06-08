#ifndef OSDEP_H_DXJTBG8M
#define OSDEP_H_DXJTBG8M



/* Round number up to multiple. Requires that d be a power of 2 (see
 * QEMU_ALIGN_UP for a safer but slower version on arbitrary
 * numbers); works even if d is a smaller type than n.  */
#ifndef ROUND_UP
#define ROUND_UP(n, d) (((n) + (d) - 1) & -(0 ? (n) : (d)))
#endif

#endif /* end of include guard: OSDEP_H_DXJTBG8M */
