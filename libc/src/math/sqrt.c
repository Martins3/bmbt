#include <errno.h>
static inline double __ieee754_sqrt(double x) {
  double z;
  __asm__("fsqrt.d %0,%1" : "=f"(z) : "f"(x));
  return z;
}

double sqrt(double x) {
  if (__builtin_expect((__builtin_isless(x, 0.0)), 0))
    (errno = (EDOM));
  return __ieee754_sqrt(x);
}
