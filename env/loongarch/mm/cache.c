#include <asm/setup.h>
#include <linux/compiler_types.h>

/* Cache operations. */
void local_flush_icache_range(unsigned long start, unsigned long end) {
  asm volatile("\tibar 0\n" ::);
}

void cpu_cache_init(void) {
  // TMP_TODO : 去更新一下关于 cache 的信息
}

void cache_error_setup(void) {
  extern void except_vec_cex(void);
  set_merr_handler(0x0, (void *)except_vec_cex, 0x80);
}
