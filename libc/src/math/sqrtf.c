#include <errno.h>
static inline float __ieee754_sqrtf(float x) {
  float z;
  __asm__("fsqrt.s %0,%1" : "=f"(z) : "f"(x));
  return z;
}

float sqrtf(float x) {
  if (__builtin_expect((__builtin_isless(x, 0.0f)), 0))
    (errno = (EDOM));
  return __ieee754_sqrtf(x);
}
