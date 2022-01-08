#ifndef LIBC_H_VVRJ7LJM
#define LIBC_H_VVRJ7LJM

#ifndef USE_UEFI_LIBC
#include <features.h>

#define __NEED_float_t
#define __NEED_double_t
#include <bits/alltypes.h>

#if 100 * __GNUC__ + __GNUC_MINOR__ >= 303
#define NAN __builtin_nanf("")
#define INFINITY __builtin_inff()
#else
#define NAN (0.0f / 0.0f)
#define INFINITY 1e5000f
#endif

#define FP_NAN 0
#define FP_INFINITE 1
#define FP_ZERO 2
#define FP_SUBNORMAL 3
#define FP_NORMAL 4
#else
typedef float float_t;
typedef double double_t;
#endif

int __fpclassify(double);
int __fpclassifyf(float);
int __fpclassifyl(long double);

static __inline unsigned __FLOAT_BITS(float __f) {
  union {
    float __f;
    unsigned __i;
  } __u;
  __u.__f = __f;
  return __u.__i;
}
static __inline unsigned long long __DOUBLE_BITS(double __f) {
  union {
    double __f;
    unsigned long long __i;
  } __u;
  __u.__f = __f;
  return __u.__i;
}

#ifndef USE_UEFI_LIBC
#define fpclassify(x)                                                          \
  (sizeof(x) == sizeof(float)                                                  \
       ? __fpclassifyf(x)                                                      \
       : sizeof(x) == sizeof(double) ? __fpclassify(x) : __fpclassifyl(x))

#define isinf(x)                                                               \
  (sizeof(x) == sizeof(float)                                                  \
       ? (__FLOAT_BITS(x) & 0x7fffffff) == 0x7f800000                          \
       : sizeof(x) == sizeof(double)                                           \
             ? (__DOUBLE_BITS(x) & -1ULL >> 1) == 0x7ffULL << 52               \
             : __fpclassifyl(x) == FP_INFINITE)

#define isnan(x)                                                               \
  (sizeof(x) == sizeof(float)                                                  \
       ? (__FLOAT_BITS(x) & 0x7fffffff) > 0x7f800000                           \
       : sizeof(x) == sizeof(double)                                           \
             ? (__DOUBLE_BITS(x) & -1ULL >> 1) > 0x7ffULL << 52                \
             : __fpclassifyl(x) == FP_NAN)

#define isnormal(x)                                                            \
  (sizeof(x) == sizeof(float)                                                  \
       ? ((__FLOAT_BITS(x) + 0x00800000) & 0x7fffffff) >= 0x01000000           \
       : sizeof(x) == sizeof(double)                                           \
             ? ((__DOUBLE_BITS(x) + (1ULL << 52)) & -1ULL >> 1) >= 1ULL << 53  \
             : __fpclassifyl(x) == FP_NORMAL)

#define isfinite(x)                                                            \
  (sizeof(x) == sizeof(float)                                                  \
       ? (__FLOAT_BITS(x) & 0x7fffffff) < 0x7f800000                           \
       : sizeof(x) == sizeof(double)                                           \
             ? (__DOUBLE_BITS(x) & -1ULL >> 1) < 0x7ffULL << 52                \
             : __fpclassifyl(x) > FP_INFINITE)
#endif

int __signbit(double);
int __signbitf(float);
int __signbitl(long double);

#define signbit(x)                                                             \
  (sizeof(x) == sizeof(float)                                                  \
       ? (int)(__FLOAT_BITS(x) >> 31)                                          \
       : sizeof(x) == sizeof(double) ? (int)(__DOUBLE_BITS(x) >> 63)           \
                                     : __signbitl(x))

#define isunordered(x, y) (isnan((x)) ? ((void)(y), 1) : isnan((y)))

#define __ISREL_DEF(rel, op, type)                                             \
  static __inline int __is##rel(type __x, type __y) {                          \
    return !isunordered(__x, __y) && __x op __y;                               \
  }

__ISREL_DEF(lessf, <, float_t)
__ISREL_DEF(less, <, double_t)
__ISREL_DEF(lessl, <, long double)
__ISREL_DEF(lessequalf, <=, float_t)
__ISREL_DEF(lessequal, <=, double_t)
__ISREL_DEF(lessequall, <=, long double)
__ISREL_DEF(lessgreaterf, !=, float_t)
__ISREL_DEF(lessgreater, !=, double_t)
__ISREL_DEF(lessgreaterl, !=, long double)
__ISREL_DEF(greaterf, >, float_t)
__ISREL_DEF(greater, >, double_t)
__ISREL_DEF(greaterl, >, long double)
__ISREL_DEF(greaterequalf, >=, float_t)
__ISREL_DEF(greaterequal, >=, double_t)
__ISREL_DEF(greaterequall, >=, long double)

#define __tg_pred_2(x, y, p)                                                   \
  (sizeof((x) + (y)) == sizeof(float)                                          \
       ? p##f(x, y)                                                            \
       : sizeof((x) + (y)) == sizeof(double) ? p(x, y) : p##l(x, y))

#define isless(x, y) __tg_pred_2(x, y, __isless)
#define islessequal(x, y) __tg_pred_2(x, y, __islessequal)
#define islessgreater(x, y) __tg_pred_2(x, y, __islessgreater)
#define isgreater(x, y) __tg_pred_2(x, y, __isgreater)
#define isgreaterequal(x, y) __tg_pred_2(x, y, __isgreaterequal)

double fabs(double);
float fabsf(float x);
double fma(double, double, double);
float fmaf(float, float, float);

float sqrtf(float);
double sqrt(double);

// bmbt: used by fpu_helper.c
double pow(double, double);
double tan(double);
double rint(double);
double floor(double);
double ceil(double);

// bmbt: used by tr_farith.h
double log(double);
double log10(double);

// bmbt: used by internal functions
long double frexpl(long double, int *);
double scalbn(double, int);

#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define M_PI 3.14159265358979323846 /* pi */
#endif

#endif /* end of include guard: LIBC_H_VVRJ7LJM */
