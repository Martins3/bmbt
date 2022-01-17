/* Cache operations. */
void local_flush_icache_range(unsigned long start, unsigned long end) {
  asm volatile("\tibar 0\n" ::);
}

void cpu_cache_init(void) {
  // TMP_TODO : 去更新一下关于 cache 的信息
}
