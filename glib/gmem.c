#include "gmem.h"
#include "glibconfig.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>

/* g_xx_malloc_xx is from gmem.c */

/**
 * g_try_malloc:
 * @n_bytes: number of bytes to allocate.
 *
 * Attempts to allocate @n_bytes, and returns %NULL on failure.
 * Contrast with g_malloc(), which aborts the program on failure.
 *
 * Returns: the allocated memory, or %NULL.
 */
gpointer g_try_malloc(gsize n_bytes) {
  gpointer mem;

  if (n_bytes) {
    mem = malloc(n_bytes);
  } else {
    mem = NULL;
  }

  return mem;
}

/**
 * g_try_malloc0:
 * @n_bytes: number of bytes to allocate
 *
 * Attempts to allocate @n_bytes, initialized to 0's, and returns %NULL on
 * failure. Contrast with g_malloc0(), which aborts the program on failure.
 *
 * Since: 2.8
 * Returns: the allocated memory, or %NULL
 */
gpointer g_try_malloc0(gsize n_bytes) {
  gpointer mem;

  if (n_bytes) {
    mem = calloc(1, n_bytes);
  } else {
    mem = NULL;
  }

  return mem;
}

/**
 * g_try_malloc_n:
 * @n_blocks: the number of blocks to allocate
 * @n_block_bytes: the size of each block in bytes
 *
 * This function is similar to g_try_malloc(), allocating (@n_blocks *
 * @n_block_bytes) bytes, but care is taken to detect possible overflow during
 * multiplication.
 *
 * Since: 2.24
 * Returns: the allocated memory, or %NULL.
 */
gpointer g_try_malloc_n(gsize n_blocks, gsize n_block_bytes) {
  if ((n_block_bytes) > 0 && (n_blocks) > (G_MAXSIZE / (n_block_bytes)))
    return NULL;

  return g_try_malloc(n_blocks * n_block_bytes);
}

/**
 * g_try_realloc:
 * @mem: (nullable): previously-allocated memory, or %NULL.
 * @n_bytes: number of bytes to allocate.
 *
 * Attempts to realloc @mem to a new size, @n_bytes, and returns %NULL
 * on failure. Contrast with g_realloc(), which aborts the program
 * on failure.
 *
 * If @mem is %NULL, behaves the same as g_try_malloc().
 *
 * Returns: the allocated memory, or %NULL.
 */
gpointer g_try_realloc(gpointer mem, gsize n_bytes) {
  gpointer newmem;

  if (n_bytes) {
    newmem = realloc(mem, n_bytes);
  } else {
    newmem = NULL;
    free(mem);
  }

  return newmem;
}

/**
 * g_malloc:
 * @n_bytes: the number of bytes to allocate
 *
 * Allocates @n_bytes bytes of memory.
 * If @n_bytes is 0 it returns %NULL.
 *
 * Returns: a pointer to the allocated memory
 */
gpointer g_malloc(gsize n_bytes) {
  if (n_bytes) {
    gpointer mem;

    mem = malloc(n_bytes);
    if (mem)
      return mem;
    // g_error ("%s: failed to allocate %"G_GSIZE_FORMAT" bytes",
    //          G_STRLOC, n_bytes);
    fprintf(stderr, "%s: failed to allocate %" G_GSIZE_FORMAT " bytes",
            G_STRLOC, n_bytes);
  }

  return NULL;
}

/**
 * g_malloc0:
 * @n_bytes: the number of bytes to allocate
 *
 * Allocates @n_bytes bytes of memory, initialized to 0's.
 * If @n_bytes is 0 it returns %NULL.
 *
 * Returns: a pointer to the allocated memory
 */
gpointer g_malloc0(gsize n_bytes) {
  if (n_bytes) {
    gpointer mem;

    mem = calloc(1, n_bytes);
    if (mem)
      return mem;

    // g_error ("%s: failed to allocate %"G_GSIZE_FORMAT" bytes",
    //          G_STRLOC, n_bytes);
    fprintf(stderr, "%s: failed to allocate %" G_GSIZE_FORMAT " bytes",
            G_STRLOC, n_bytes);
  }

  return NULL;
}

/**
 * g_malloc_n:
 * @n_blocks: the number of blocks to allocate
 * @n_block_bytes: the size of each block in bytes
 *
 * This function is similar to g_malloc(), allocating (@n_blocks *
 * @n_block_bytes) bytes, but care is taken to detect possible overflow during
 * multiplication.
 *
 * Since: 2.24
 * Returns: a pointer to the allocated memory
 */
gpointer g_malloc_n(gsize n_blocks, gsize n_block_bytes) {
  if ((n_block_bytes) > 0 && (n_blocks) > (G_MAXSIZE / (n_block_bytes))) {
    fprintf(stderr,
            "%s: overflow allocating %" G_GSIZE_FORMAT "*%" G_GSIZE_FORMAT
            " bytes",
            G_STRLOC, n_blocks, n_block_bytes);
  }

  return g_malloc(n_blocks * n_block_bytes);
}

/**
 * g_malloc0_n:
 * @n_blocks: the number of blocks to allocate
 * @n_block_bytes: the size of each block in bytes
 *
 * This function is similar to g_malloc0(), allocating (@n_blocks *
 * @n_block_bytes) bytes, but care is taken to detect possible overflow during
 * multiplication.
 *
 * Since: 2.24
 * Returns: a pointer to the allocated memory
 */
gpointer g_malloc0_n(gsize n_blocks, gsize n_block_bytes) {
  if ((n_block_bytes) > 0 && (n_blocks) > (G_MAXSIZE / (n_block_bytes))) {
    fprintf(stderr,
            "%s: overflow allocating %" G_GSIZE_FORMAT "*%" G_GSIZE_FORMAT
            " bytes",
            G_STRLOC, n_blocks, n_block_bytes);
  }

  return g_malloc0(n_blocks * n_block_bytes);
}

/**
 * g_realloc:
 * @mem: (nullable): the memory to reallocate
 * @n_bytes: new size of the memory in bytes
 *
 * Reallocates the memory pointed to by @mem, so that it now has space for
 * @n_bytes bytes of memory. It returns the new address of the memory, which may
 * have been moved. @mem may be %NULL, in which case it's considered to
 * have zero-length. @n_bytes may be 0, in which case %NULL will be returned
 * and @mem will be freed unless it is %NULL.
 *
 * Returns: the new address of the allocated memory
 */
gpointer g_realloc(gpointer mem, gsize n_bytes) {
  gpointer newmem;

  if (n_bytes) {
    newmem = realloc(mem, n_bytes);
    if (newmem)
      return newmem;

    fprintf(stderr, "%s: failed to allocate %" G_GSIZE_FORMAT " bytes",
            G_STRLOC, n_bytes);
  }
  free(mem);

  return NULL;
}

/**
 * g_realloc_n:
 * @mem: (nullable): the memory to reallocate
 * @n_blocks: the number of blocks to allocate
 * @n_block_bytes: the size of each block in bytes
 *
 * This function is similar to g_realloc(), allocating (@n_blocks *
 * @n_block_bytes) bytes, but care is taken to detect possible overflow during
 * multiplication.
 *
 * Since: 2.24
 * Returns: the new address of the allocated memory
 */
gpointer g_realloc_n(gpointer mem, gsize n_blocks, gsize n_block_bytes) {
  if ((n_block_bytes) > 0 && (n_blocks) > (G_MAXSIZE / (n_block_bytes))) {
    fprintf(stderr,
            "%s: overflow allocating %" G_GSIZE_FORMAT "*%" G_GSIZE_FORMAT
            " bytes",
            G_STRLOC, n_blocks, n_block_bytes);
  }

  return g_realloc(mem, n_blocks * n_block_bytes);
}

/**
 * g_free:
 * @mem: (nullable): the memory to free
 *
 * Frees the memory pointed to by @mem.
 *
 * If @mem is %NULL it simply returns, so there is no need to check @mem
 * against %NULL before calling this function.
 */
void g_free(gpointer mem) { free(mem); }

/* g_memdup is from gstrfuncs.c */

/**
 * g_memdup:
 * @mem: the memory to copy.
 * @byte_size: the number of bytes to copy.
 *
 * Allocates @byte_size bytes of memory, and copies @byte_size bytes into it
 * from @mem. If @mem is %NULL it returns %NULL.
 *
 * Returns: a pointer to the newly-allocated copy of the memory, or %NULL if
 * @mem is %NULL.
 */
gpointer g_memdup(gconstpointer mem, guint byte_size) {
  gpointer new_mem;

  if (mem && byte_size != 0) {
    new_mem = g_malloc(byte_size);
    memcpy(new_mem, mem, byte_size);
  } else
    new_mem = NULL;

  return new_mem;
}

gpointer g_slice_alloc(gsize block_size) { return g_malloc(block_size); }
