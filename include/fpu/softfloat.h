#ifndef SOFTFLOAT_H_DGVZIKLW
#define SOFTFLOAT_H_DGVZIKLW

#include "softfloat-types.h"

// TODO I don't want copy fpu/softfloat.c yet
int floatx80_compare(floatx80, floatx80, float_status *status);
int floatx80_compare_quiet(floatx80, floatx80, float_status *status);

/*----------------------------------------------------------------------------
| Software IEC/IEEE extended double-precision conversion routines.
*----------------------------------------------------------------------------*/
int32_t floatx80_to_int32(floatx80, float_status *status);
int32_t floatx80_to_int32_round_to_zero(floatx80, float_status *status);
int64_t floatx80_to_int64(floatx80, float_status *status);
int64_t floatx80_to_int64_round_to_zero(floatx80, float_status *status);
float32 floatx80_to_float32(floatx80, float_status *status);
float64 floatx80_to_float64(floatx80, float_status *status);
// float128 floatx80_to_float128(floatx80, float_status *status);

/*----------------------------------------------------------------------------
| Software IEC/IEEE extended double-precision operations.
*----------------------------------------------------------------------------*/
// floatx80 floatx80_round(floatx80 a, float_status *status);
// floatx80 floatx80_round_to_int(floatx80, float_status *status);
floatx80 floatx80_add(floatx80, floatx80, float_status *status);
floatx80 floatx80_sub(floatx80, floatx80, float_status *status);
floatx80 floatx80_mul(floatx80, floatx80, float_status *status);
floatx80 floatx80_div(floatx80, floatx80, float_status *status);
// floatx80 floatx80_rem(floatx80, floatx80, float_status *status);
// floatx80 floatx80_sqrt(floatx80, float_status *status);
// int floatx80_eq(floatx80, floatx80, float_status *status);
// int floatx80_le(floatx80, floatx80, float_status *status);
// int floatx80_lt(floatx80, floatx80, float_status *status);
// int floatx80_unordered(floatx80, floatx80, float_status *status);
// int floatx80_eq_quiet(floatx80, floatx80, float_status *status);
// int floatx80_le_quiet(floatx80, floatx80, float_status *status);
// int floatx80_lt_quiet(floatx80, floatx80, float_status *status);
// int floatx80_unordered_quiet(floatx80, floatx80, float_status *status);
// int floatx80_compare(floatx80, floatx80, float_status *status);
// int floatx80_compare_quiet(floatx80, floatx80, float_status *status);
// int floatx80_is_quiet_nan(floatx80, float_status *status);
// int floatx80_is_signaling_nan(floatx80, float_status *status);
// floatx80 floatx80_silence_nan(floatx80, float_status *status);
// floatx80 floatx80_scalbn(floatx80, int, float_status *status);


static inline floatx80 floatx80_abs(floatx80 a)
{
    a.high &= 0x7fff;
    return a;
}

static inline floatx80 floatx80_chs(floatx80 a)
{
    a.high ^= 0x8000;
    return a;
}

#define floatx80_zero make_floatx80(0x0000, 0x0000000000000000LL)
#define floatx80_one make_floatx80(0x3fff, 0x8000000000000000LL)
#define floatx80_ln2 make_floatx80(0x3ffe, 0xb17217f7d1cf79acLL)
#define floatx80_pi make_floatx80(0x4000, 0xc90fdaa22168c235LL)
#define floatx80_half make_floatx80(0x3ffe, 0x8000000000000000LL)
#endif /* end of include guard: SOFTFLOAT_H_DGVZIKLW */
