#ifndef DEBUGCON_H_V0AYNIWH
#define DEBUGCON_H_V0AYNIWH

void debugcon_puts(const char *s);
void backtrace(long *fp);

static inline long *read_fp() {
  long *x;
  asm volatile("move %0, $fp" : "=r"(x));
  return x;
}
#endif /* end of include guard: DEBUGCON_H_V0AYNIWH */
