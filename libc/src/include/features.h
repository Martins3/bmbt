#ifndef FEATURES_H_MHZMO5SF
#define FEATURES_H_MHZMO5SF

#include "../../include/features.h"

#define weak __attribute__((__weak__))
#define hidden __attribute__((__visibility__("hidden")))
#define weak_alias(old, new)                                                   \
  extern __typeof(old) new __attribute__((__weak__, __alias__(#old)))

#endif /* end of include guard: FEATURES_H_MHZMO5SF */
