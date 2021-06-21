#ifndef BITMAP_H_ENRPTXFD
#define BITMAP_H_ENRPTXFD

#include <stdlib.h>
#include "bitops.h"


static inline unsigned long *bitmap_try_new(long nbits)
{
  // FIXME fix later
    // long len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
    // return g_try_malloc0(len);
    return 0;
}


static inline unsigned long *bitmap_new(long nbits)
{
    unsigned long *ptr = bitmap_try_new(nbits);
    if (ptr == NULL) {
        abort();
    }
    return ptr;
}

// FIXME fix later, I'm fucking tired
void bitmap_set(unsigned long *map, long i, long len);

#define DECLARE_BITMAP(name,bits)                  \
        unsigned long name[BITS_TO_LONGS(bits)]

#endif /* end of include guard: BITMAP_H_ENRPTXFD */
