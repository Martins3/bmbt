#ifndef NODEMASK_H_HBVICV0E
#define NODEMASK_H_HBVICV0E

#include <linux/numa.h>
#include <qemu/bitmap.h>

typedef struct {
  DECLARE_BITMAP(bits, MAX_NUMNODES);
} nodemask_t;

/* No static inline type checking - see Subtlety (1) above. */
#define node_isset(node, nodemask) test_bit((node), (nodemask).bits)

#define node_set(node, dst) __node_set((node), &(dst))

/**
 * NOTE: this function originally copied from Linux kernel, and the modifier
 * volatile is removed. I think it doesn't matter and more investigation is
 * needed.
 */
static inline __attribute__((always_inline)) void __node_set(int node,
                                                             nodemask_t *dstp) {
  set_bit(node, dstp->bits);
}

#define node_clear(node, dst) __node_clear((node), &(dst))
static inline void __node_clear(int node, nodemask_t *dstp) {
  clear_bit(node, dstp->bits);
}

#define for_each_node(node) for ((node) = 0; (node) < 1; (node)++)

#endif /* end of include guard: NODEMASK_H_HBVICV0E */
