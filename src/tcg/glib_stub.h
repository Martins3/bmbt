#ifndef GLIB_STUB_H_78QYZX2K
#define GLIB_STUB_H_78QYZX2K

#include <stdbool.h>
#include <stdlib.h>

typedef struct GTree {
  struct GTree *left;
  struct GTree *right;
} GTree;

typedef void *gpointer;
typedef bool gboolean;

#define FALSE false
#define TRUE true

typedef int gint;
typedef const void *gconstpointer;

void g_tree_insert(GTree *, gpointer, gpointer);
void g_tree_remove(GTree *, void *);
void *g_tree_lookup(GTree *tree, gpointer key);
int g_tree_nnodes(GTree *tree);
GTree *g_tree_ref(GTree *tree);
void g_tree_destroy(GTree *tree);

typedef bool (*GTraverseFunc)(gpointer key, gpointer value, gpointer data);

void g_tree_foreach(GTree *tree, GTraverseFunc func, gpointer user_data);

typedef gint (*GCompareDataFunc)(gconstpointer a, gconstpointer b,
                                 gpointer user_data);
typedef void (*GDestroyNotify)(gpointer data);

GTree *g_tree_new_full(GCompareDataFunc key_compare_func,
                       gpointer key_compare_data,
                       GDestroyNotify key_destroy_func,
                       GDestroyNotify value_destroy_func);

typedef gint (*GCompareFunc)(gconstpointer a, gconstpointer b);
GTree *g_tree_new(GCompareFunc key_compare_func);

gpointer g_try_malloc0(int n_bytes);

#define g_new(type, num) (type *)NULL
#define g_try_new(type, num) (type *)NULL
#define g_free(type)                                                           \
  {}
// @todo implement it
#define g_assert(expr)                                                         \
  {}
#define g_assert_not_reached()                                                 \
  { exit(0); }

#define duck_check(x)                                                          \
  {                                                                            \
    if (!x) {                                                                  \
      g_assert_not_reached()                                                   \
    }                                                                          \
  }

#define g_new0(struct_type, n_structs) (struct_type *)NULL
#define g_renew(struct_type, mem, n_structs) (struct_type *)NULL

#define g_malloc(size) NULL

#undef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#undef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

gpointer g_memdup(gconstpointer mem, uint byte_size);

gpointer g_malloc0(gint n_bytes);

typedef struct _GList GList;

struct _GList {
  gpointer data;
  GList *next;
  GList *prev;
};

typedef struct QDict {
} QDict;

#endif /* end of include guard: GLIB_STUB_H_78QYZX2K */
