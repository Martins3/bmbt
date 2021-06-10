#ifndef BITOPS_H_Z29X3LYD
#define BITOPS_H_Z29X3LYD

#define MAKE_64BIT_MASK(shift, length) \
    (((~0ULL) >> (64 - (length))) << (shift))

#endif /* end of include guard: BITOPS_H_Z29X3LYD */
