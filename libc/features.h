#ifndef FEATURES_H_OQWGZPCM
#define FEATURES_H_OQWGZPCM

#ifndef __STDC_VERSION__
#define __STDC_VERSION__ 201211L
#endif

#if __STDC_VERSION__ >= 199901L
#define __restrict restrict
#elif !defined(__GNUC__)
#define __restrict
#endif

#if __STDC_VERSION__ >= 199901L || defined(__cplusplus)
#define __inline inline
#elif !defined(__GNUC__)
#define __inline
#endif

#if __STDC_VERSION__ >= 201112L
#elif defined(__GNUC__)
#define _Noreturn __attribute__((__noreturn__))
#else
#define _Noreturn
#endif

#endif /* end of include guard: FEATURES_H_OQWGZPCM */
