#ifndef GSLICE_H_C3XFEJZB
#define GSLICE_H_C3XFEJZB

#include "gmem.h"
gpointer g_slice_alloc(gsize block_size) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
#define g_slice_new(type) ((type *)g_slice_alloc(sizeof(type)))

void g_slice_free1(gsize mem_size, gpointer mem_block);

#define g_slice_free(type, mem)                                                \
  G_STMT_START {                                                               \
    if (1)                                                                     \
      g_slice_free1(sizeof(type), (mem));                                      \
    else                                                                       \
      (void)((type *)0 == (mem));                                              \
  }                                                                            \
  G_STMT_END

#endif /* end of include guard: GSLICE_H_C3XFEJZB */
