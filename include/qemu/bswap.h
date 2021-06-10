#ifndef BSWAP_H_POC0IZMR
#define BSWAP_H_POC0IZMR

#include "../types.h"
#include "../fpu/softfloat-types.h"

typedef union {
    float32 f;
    uint32_t l;
} CPU_FloatU;

typedef union {
    float64 d;
#if defined(HOST_WORDS_BIGENDIAN)
    struct {
        uint32_t upper;
        uint32_t lower;
    } l;
#else
    struct {
        uint32_t lower;
        uint32_t upper;
    } l;
#endif
    uint64_t ll;
} CPU_DoubleU;

typedef union {
     floatx80 d;
     struct {
         uint64_t lower;
         uint16_t upper;
     } l;
} CPU_LDoubleU;


#endif /* end of include guard: BSWAP_H_POC0IZMR */
