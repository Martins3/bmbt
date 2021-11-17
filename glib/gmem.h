#ifndef GMEM_H_JB5SEPOA
#define GMEM_H_JB5SEPOA

#include "gatomic.h"
#include "glibconfig.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * g_new:
 * @struct_type: the type of the elements to allocate
 * @n_structs: the number of elements to allocate
 *
 * Allocates @n_structs elements of type @struct_type.
 * The returned pointer is cast to a pointer to the given type.
 * If @n_structs is 0 it returns %NULL.
 * Care is taken to avoid overflow when calculating the size of the allocated
 * block.
 *
 * Since the returned pointer is already casted to the right type,
 * it is normally unnecessary to cast it explicitly, and doing
 * so might hide memory allocation errors.
 *
 * Returns: a pointer to the allocated memory, cast to a pointer to @struct_type
 */
#define g_new(struct_type, n_structs) _G_NEW(struct_type, n_structs, malloc)
/**
 * g_try_new:
 * @struct_type: the type of the elements to allocate
 * @n_structs: the number of elements to allocate
 *
 * Attempts to allocate @n_structs elements of type @struct_type, and returns
 * %NULL on failure. Contrast with g_new(), which aborts the program on failure.
 * The returned pointer is cast to a pointer to the given type.
 * The function returns %NULL when @n_structs is 0 of if an overflow occurs.
 *
 * Since: 2.8
 * Returns: a pointer to the allocated memory, cast to a pointer to @struct_type
 */
#define g_try_new(struct_type, n_structs)                                      \
  _G_NEW(struct_type, n_structs, try_malloc)

/**
 * g_new0:
 * @struct_type: the type of the elements to allocate.
 * @n_structs: the number of elements to allocate.
 *
 * Allocates @n_structs elements of type @struct_type, initialized to 0's.
 * The returned pointer is cast to a pointer to the given type.
 * If @n_structs is 0 it returns %NULL.
 * Care is taken to avoid overflow when calculating the size of the allocated
 * block.
 *
 * Since the returned pointer is already casted to the right type,
 * it is normally unnecessary to cast it explicitly, and doing
 * so might hide memory allocation errors.
 *
 * Returns: a pointer to the allocated memory, cast to a pointer to
 * @struct_type.
 */
#define g_new0(struct_type, n_structs) _G_NEW(struct_type, n_structs, malloc0)
/**
 * g_renew:
 * @struct_type: the type of the elements to allocate
 * @mem: the currently allocated memory
 * @n_structs: the number of elements to allocate
 *
 * Reallocates the memory pointed to by @mem, so that it now has space for
 * @n_structs elements of type @struct_type. It returns the new address of
 * the memory, which may have been moved.
 * Care is taken to avoid overflow when calculating the size of the allocated
 * block.
 *
 * Returns: a pointer to the new allocated memory, cast to a pointer to
 * @struct_type
 */
#define g_renew(struct_type, mem, n_structs)                                   \
  _G_RENEW(struct_type, mem, n_structs, realloc)

void g_free(gpointer mem);
gpointer g_malloc(gsize n_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
gpointer g_malloc0(gsize n_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
gpointer g_realloc(gpointer mem, gsize n_bytes) G_GNUC_WARN_UNUSED_RESULT;
gpointer g_try_malloc(gsize n_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
gpointer g_try_malloc0(gsize n_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
gpointer g_try_realloc(gpointer mem, gsize n_bytes) G_GNUC_WARN_UNUSED_RESULT;
gpointer g_malloc_n(gsize n_blocks, gsize n_block_bytes) G_GNUC_MALLOC
    G_GNUC_ALLOC_SIZE2(1, 2);
gpointer g_malloc0_n(gsize n_blocks, gsize n_block_bytes) G_GNUC_MALLOC
    G_GNUC_ALLOC_SIZE2(1, 2);
gpointer g_realloc_n(gpointer mem, gsize n_blocks,
                     gsize n_block_bytes) G_GNUC_WARN_UNUSED_RESULT;
gpointer g_try_malloc_n(gsize n_blocks, gsize n_block_bytes) G_GNUC_MALLOC
    G_GNUC_ALLOC_SIZE2(1, 2);
gpointer g_memdup(gconstpointer mem, guint byte_size) G_GNUC_ALLOC_SIZE(2);
gpointer g_slice_alloc(gsize block_size) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);

/* Optimise: avoid the call to the (slower) _n function if we can
 * determine at compile-time that no overflow happens.
 */
#if defined(__GNUC__) && (__GNUC__ >= 2) && defined(__OPTIMIZE__)
#define _G_NEW(struct_type, n_structs, func)                                   \
  (struct_type *)(G_GNUC_EXTENSION({                                           \
    gsize __n = (gsize)(n_structs);                                            \
    gsize __s = sizeof(struct_type);                                           \
    gpointer __p;                                                              \
    if (__s == 1)                                                              \
      __p = g_##func(__n);                                                     \
    else if (__builtin_constant_p(__n) &&                                      \
             (__s == 0 || __n <= G_MAXSIZE / __s))                             \
      __p = g_##func(__n * __s);                                               \
    else                                                                       \
      __p = g_##func##_n(__n, __s);                                            \
    __p;                                                                       \
  }))
#define _G_RENEW(struct_type, mem, n_structs, func)                            \
  (struct_type *)(G_GNUC_EXTENSION({                                           \
    gsize __n = (gsize)(n_structs);                                            \
    gsize __s = sizeof(struct_type);                                           \
    gpointer __p = (gpointer)(mem);                                            \
    if (__s == 1)                                                              \
      __p = g_##func(__p, __n);                                                \
    else if (__builtin_constant_p(__n) &&                                      \
             (__s == 0 || __n <= G_MAXSIZE / __s))                             \
      __p = g_##func(__p, __n * __s);                                          \
    else                                                                       \
      __p = g_##func##_n(__p, __n, __s);                                       \
    __p;                                                                       \
  }))

#else

/* Unoptimised version: always call the _n() function. */

#define _G_NEW(struct_type, n_structs, func)                                   \
  ((struct_type *)g_##func##_n((n_structs), sizeof(struct_type)))
#define _G_RENEW(struct_type, mem, n_structs, func)                            \
  ((struct_type *)g_##func##_n(mem, (n_structs), sizeof(struct_type)))

#endif

#endif /* end of include guard: GMEM_H_JB5SEPOA */
