#ifndef SOFTFLOAT_TYPES_H_OPERWT2C
#define SOFTFLOAT_TYPES_H_OPERWT2C
#include "../types.h"

/* This 'flag' type must be able to hold at least 0 and 1. It should
 * probably be replaced with 'bool' but the uses would need to be audited
 * to check that they weren't accidentally relying on it being a larger type.
 */
typedef uint8_t flag;

typedef uint16_t float16;
typedef uint32_t float32;
typedef uint64_t float64;
/*
 * Software IEC/IEEE floating-point types.
 */

typedef uint16_t float16;
typedef uint32_t float32;
typedef uint64_t float64;
#define float16_val(x) (x)
#define float32_val(x) (x)
#define float64_val(x) (x)
#define make_float16(x) (x)
#define make_float32(x) (x)
#define make_float64(x) (x)
#define const_float16(x) (x)
#define const_float32(x) (x)
#define const_float64(x) (x)

typedef struct {
  uint64_t low;
  uint16_t high;
} floatx80;

/*
 * Floating Point Status. Individual architectures may maintain
 * several versions of float_status for different functions. The
 * correct status for the operation is then passed by reference to
 * most of the softfloat functions.
 */

typedef struct float_status {
  signed char float_detect_tininess;
  signed char float_rounding_mode;
  uint8_t float_exception_flags;
  signed char floatx80_rounding_precision;
  /* should denormalised results go to zero and set the inexact flag? */
  flag flush_to_zero;
  /* should denormalised inputs go to zero and set the input_denormal flag? */
  flag flush_inputs_to_zero;
  flag default_nan_mode;
  /* not always used -- see snan_bit_is_one() in softfloat-specialize.h */
  flag snan_bit_is_one;
} float_status;

/*
 *Software IEC/IEEE floating-point rounding mode.
 */

enum {
    float_round_nearest_even = 0,
    float_round_down         = 1,
    float_round_up           = 2,
    float_round_to_zero      = 3,
    float_round_ties_away    = 4,
    /* Not an IEEE rounding mode: round to the closest odd mantissa value */
    float_round_to_odd       = 5,
};

// FIXME put this to ./softfloat.h, these function is defined at fpu/softfloat.c
floatx80 float64_to_floatx80(float64, float_status *status);
float64 floatx80_to_float64(floatx80, float_status *status);


#define make_floatx80(exp, mant) ((floatx80) { mant, exp })

#endif /* end of include guard: SOFTFLOAT_TYPES_H_OPERWT2C */
