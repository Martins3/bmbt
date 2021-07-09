#ifndef GLIB_STUB_H_78QYZX2K
#define GLIB_STUB_H_78QYZX2K
// FIXME I know the glib will be removed eventually
// but #include <glib-2.0/glib/gtree.h> doesn't work
// so create a empty stub for it.
//
// I will remvoe this with glib, but the glib will be replace later.

#include <stdbool.h>

typedef struct GTree {
  struct GTree *left;
  struct GTree *right;
} GTree;

typedef void *gpointer;
typedef bool gboolean;

#define FALSE false
#define TRUE true

typedef int gint;
typedef void *gconstpointer;

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

#endif /* end of include guard: GLIB_STUB_H_78QYZX2K */
