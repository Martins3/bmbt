#include "gslice.h"
// [interface 50]
gpointer g_slice_alloc(gsize block_size) { return g_malloc(block_size); }
void g_slice_free1(gsize mem_size, gpointer mem_block) { g_free(mem_block); }
