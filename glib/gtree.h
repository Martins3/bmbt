#ifndef GTREE_H_L4WEA9D5
#define GTREE_H_L4WEA9D5

#include "glibconfig.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _GTreeNode GTreeNode;
typedef struct _GTree GTree;

void g_tree_insert(GTree *tree, gpointer, gpointer);
gboolean g_tree_remove(GTree *tree, gconstpointer key);
gpointer g_tree_lookup(GTree *tree, gconstpointer key);
gint g_tree_nnodes(GTree *tree);
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

#endif /* end of include guard: GTREE_H_L4WEA9D5 */
