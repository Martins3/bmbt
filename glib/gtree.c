#include "gtree.h"
#include "assert.h"
#include "gmem.h"
#include "gtestutils.h"
#include <malloc.h>
#include <stdlib.h>

#define MAX_GTREE_HEIGHT 40

/**
 * GTree:
 *
 * The GTree struct is an opaque data structure representing a
 * [balanced binary tree][glib-Balanced-Binary-Trees]. It should be
 * accessed only by using the following functions.
 */
struct _GTree {
  GTreeNode *root;
  GCompareDataFunc key_compare;
  GDestroyNotify key_destroy_func;
  GDestroyNotify value_destroy_func;
  gpointer key_compare_data;
  guint nnodes;
  gint ref_count;
};

struct _GTreeNode {
  gpointer key;     /* key for this node */
  gpointer value;   /* value stored at this node */
  GTreeNode *left;  /* left subtree */
  GTreeNode *right; /* right subtree */
  gint8 balance;    /* height (right) - height (left) */
  guint8 left_child;
  guint8 right_child;
};

static void g_tree_insert_internal(GTree *tree, gpointer key, gpointer value,
                                   gboolean replace);
static GTreeNode *g_tree_node_new(gpointer key, gpointer value);
static GTreeNode *g_tree_node_balance(GTreeNode *node);
static GTreeNode *g_tree_node_rotate_left(GTreeNode *node);
static GTreeNode *g_tree_node_rotate_right(GTreeNode *node);
static GTreeNode *g_tree_find_node(GTree *tree, gconstpointer key);
void g_tree_destroy(GTree *tree);
void g_tree_foreach(GTree *tree, GTraverseFunc func, gpointer user_data);

static GTreeNode *g_tree_node_rotate_left(GTreeNode *node) {
  GTreeNode *right;
  gint a_bal;
  gint b_bal;

  right = node->right;

  if (right->left_child)
    node->right = right->left;
  else {
    node->right_child = FALSE;
    right->left_child = TRUE;
  }
  right->left = node;

  a_bal = node->balance;
  b_bal = right->balance;

  if (b_bal <= 0) {
    if (a_bal >= 1)
      right->balance = b_bal - 1;
    else
      right->balance = a_bal + b_bal - 2;
    node->balance = a_bal - 1;
  } else {
    if (a_bal <= b_bal)
      right->balance = a_bal - 2;
    else
      right->balance = b_bal - 1;
    node->balance = a_bal - b_bal - 1;
  }

  return right;
}

static GTreeNode *g_tree_node_rotate_right(GTreeNode *node) {
  GTreeNode *left;
  gint a_bal;
  gint b_bal;

  left = node->left;

  if (left->right_child)
    node->left = left->right;
  else {
    node->left_child = FALSE;
    left->right_child = TRUE;
  }
  left->right = node;

  a_bal = node->balance;
  b_bal = left->balance;

  if (b_bal <= 0) {
    if (b_bal > a_bal)
      left->balance = b_bal + 1;
    else
      left->balance = a_bal + 2;
    node->balance = a_bal - b_bal + 1;
  } else {
    if (a_bal <= -1)
      left->balance = b_bal + 1;
    else
      left->balance = a_bal + b_bal + 2;
    node->balance = a_bal + 1;
  }

  return left;
}

static GTreeNode *g_tree_node_balance(GTreeNode *node) {
  if (node->balance < -1) {
    if (node->left->balance > 0)
      node->left = g_tree_node_rotate_left(node->left);
    node = g_tree_node_rotate_right(node);
  } else if (node->balance > 1) {
    if (node->right->balance < 0)
      node->right = g_tree_node_rotate_right(node->right);
    node = g_tree_node_rotate_left(node);
  }

  return node;
}

static GTreeNode *g_tree_node_new(gpointer key, gpointer value) {
  // the full g_slice_new function is complete and can't port almostly,
  // so use the simplified function.
  // you can find the full implement of g_slice_new in gslice.c.
  // GTreeNode *node = g_slice_new (GTreeNode);

  GTreeNode *node = g_slice_alloc(sizeof(GTreeNode));
  node->balance = 0;
  node->left = NULL;
  node->right = NULL;
  node->left_child = FALSE;
  node->right_child = FALSE;
  node->key = key;
  node->value = value;

  return node;
}

/* internal insert routine */
static void g_tree_insert_internal(GTree *tree, gpointer key, gpointer value,
                                   gboolean replace) {
  GTreeNode *node;
  GTreeNode *path[MAX_GTREE_HEIGHT];
  int idx;

  assert(tree != NULL);

  if (!tree->root) {
    tree->root = g_tree_node_new(key, value);
    tree->nnodes++;
    return;
  }

  idx = 0;
  path[idx++] = NULL;
  node = tree->root;

  while (1) {
    int cmp = tree->key_compare(key, node->key, tree->key_compare_data);

    if (cmp == 0) {
      if (tree->value_destroy_func)
        tree->value_destroy_func(node->value);

      node->value = value;

      if (replace) {
        if (tree->key_destroy_func)
          tree->key_destroy_func(node->key);

        node->key = key;
      } else {
        /* free the passed key */
        if (tree->key_destroy_func)
          tree->key_destroy_func(key);
      }

      return;
    } else if (cmp < 0) {
      if (node->left_child) {
        path[idx++] = node;
        node = node->left;
      } else {
        GTreeNode *child = g_tree_node_new(key, value);

        child->left = node->left;
        child->right = node;
        node->left = child;
        node->left_child = TRUE;
        node->balance -= 1;

        tree->nnodes++;

        break;
      }
    } else {
      if (node->right_child) {
        path[idx++] = node;
        node = node->right;
      } else {
        GTreeNode *child = g_tree_node_new(key, value);

        child->right = node->right;
        child->left = node;
        node->right = child;
        node->right_child = TRUE;
        node->balance += 1;

        tree->nnodes++;

        break;
      }
    }
  }

  /* Restore balance. This is the goodness of a non-recursive
   * implementation, when we are done with balancing we 'break'
   * the loop and we are done.
   */
  while (1) {
    GTreeNode *bparent = path[--idx];
    gboolean left_node = (bparent && node == bparent->left);
    g_assert(!bparent || bparent->left == node || bparent->right == node);

    if (node->balance < -1 || node->balance > 1) {
      node = g_tree_node_balance(node);
      if (bparent == NULL)
        tree->root = node;
      else if (left_node)
        bparent->left = node;
      else
        bparent->right = node;
    }

    if (node->balance == 0 || bparent == NULL)
      break;

    if (left_node)
      bparent->balance -= 1;
    else
      bparent->balance += 1;

    node = bparent;
  }
}

/**
 * g_tree_insert:
 * @tree: a #GTree
 * @key: the key to insert
 * @value: the value corresponding to the key
 *
 * Inserts a key/value pair into a #GTree.
 *
 * If the given key already exists in the #GTree its corresponding value
 * is set to the new value. If you supplied a @value_destroy_func when
 * creating the #GTree, the old value is freed using that function. If
 * you supplied a @key_destroy_func when creating the #GTree, the passed
 * key is freed using that function.
 *
 * The tree is automatically 'balanced' as new key/value pairs are added,
 * so that the distance from the root to every leaf is as small as possible.
 */
void g_tree_insert(GTree *tree, gpointer key, gpointer value) {
  assert(tree != NULL);

  g_tree_insert_internal(tree, key, value, FALSE);
}

static inline GTreeNode *g_tree_first_node(GTree *tree) {
  GTreeNode *tmp;

  if (!tree->root)
    return NULL;

  tmp = tree->root;

  while (tmp->left_child)
    tmp = tmp->left;

  return tmp;
}

static inline GTreeNode *g_tree_node_next(GTreeNode *node) {
  GTreeNode *tmp;

  tmp = node->right;

  if (node->right_child)
    while (tmp->left_child)
      tmp = tmp->left;

  return tmp;
}

static void g_tree_remove_all(GTree *tree) {
  GTreeNode *node;
  GTreeNode *next;

  assert(tree != NULL);

  node = g_tree_first_node(tree);

  while (node) {
    next = g_tree_node_next(node);

    if (tree->key_destroy_func)
      tree->key_destroy_func(node->key);
    if (tree->value_destroy_func)
      tree->value_destroy_func(node->value);
    // g_slice_free (GTreeNode, node);
    // CAUTION!!! there maybe a bug!!!
    if (!node) {
      return;
    } else {
      g_free(node);
    }
    node = next;
  }

  tree->root = NULL;
  tree->nnodes = 0;
}

static GTreeNode *g_tree_find_node(GTree *tree, gconstpointer key) {
  GTreeNode *node;
  gint cmp;

  node = tree->root;
  if (!node)
    return NULL;

  while (1) {
    cmp = tree->key_compare(key, node->key, tree->key_compare_data);
    if (cmp == 0)
      return node;
    else if (cmp < 0) {
      if (!node->left_child)
        return NULL;

      node = node->left;
    } else {
      if (!node->right_child)
        return NULL;

      node = node->right;
    }
  }
}

/**
 * g_tree_lookup:
 * @tree: a #GTree
 * @key: the key to look up
 *
 * Gets the value corresponding to the given key. Since a #GTree is
 * automatically balanced as key/value pairs are added, key lookup
 * is O(log n) (where n is the number of key/value pairs in the tree).
 *
 * Returns: the value corresponding to the key, or %NULL
 *     if the key was not found
 */
gpointer g_tree_lookup(GTree *tree, gconstpointer key) {
  GTreeNode *node;

  assert(tree != NULL);

  node = g_tree_find_node(tree, key);

  return node ? node->value : NULL;
}

/**
 * g_tree_nnodes:
 * @tree: a #GTree
 *
 * Gets the number of nodes in a #GTree.
 *
 * Returns: the number of nodes in @tree
 */
gint g_tree_nnodes(GTree *tree) {
  assert(tree != NULL);

  return tree->nnodes;
}

/**
 * g_tree_ref:
 * @tree: a #GTree
 *
 * Increments the reference count of @tree by one.
 *
 * It is safe to call this function from any thread.
 *
 * Returns: the passed in #GTree
 *
 * Since: 2.22
 */
GTree *g_tree_ref(GTree *tree) {
  assert(tree != NULL);

  g_atomic_int_inc(&tree->ref_count);

  return tree;
}

/**
 * g_tree_unref:
 * @tree: a #GTree
 *
 * Decrements the reference count of @tree by one.
 * If the reference count drops to 0, all keys and values will
 * be destroyed (if destroy functions were specified) and all
 * memory allocated by @tree will be released.
 *
 * It is safe to call this function from any thread.
 *
 * Since: 2.22
 */
void g_tree_unref(GTree *tree) {
  assert(tree != NULL);

  if (g_atomic_int_dec_and_test(&tree->ref_count)) {
    g_tree_remove_all(tree);
    // g_slice_free (GTree, tree);
    // CAUTION!!! there maybe a bug!!!
    if (!tree) {
      return;
    } else {
      g_free(tree);
    }
  }
}

/**
 * g_tree_destroy:
 * @tree: a #GTree
 *
 * Removes all keys and values from the #GTree and decreases its
 * reference count by one. If keys and/or values are dynamically
 * allocated, you should either free them first or create the #GTree
 * using g_tree_new_full(). In the latter case the destroy functions
 * you supplied will be called on all keys and values before destroying
 * the #GTree.
 */
void g_tree_destroy(GTree *tree) {
  assert(tree != NULL);

  g_tree_remove_all(tree);
  g_tree_unref(tree);
}

/**
 * g_tree_foreach:
 * @tree: a #GTree
 * @func: the function to call for each node visited.
 *     If this function returns %TRUE, the traversal is stopped.
 * @user_data: user data to pass to the function
 *
 * Calls the given function for each of the key/value pairs in the #GTree.
 * The function is passed the key and value of each pair, and the given
 * @data parameter. The tree is traversed in sorted order.
 *
 * The tree may not be modified while iterating over it (you can't
 * add/remove items). To remove all items matching a predicate, you need
 * to add each item to a list in your #GTraverseFunc as you walk over
 * the tree, then walk the list and remove each item.
 */
void g_tree_foreach(GTree *tree, GTraverseFunc func, gpointer user_data) {
  GTreeNode *node;

  assert(tree != NULL);

  if (!tree->root)
    return;

  node = g_tree_first_node(tree);

  while (node) {
    if ((*func)(node->key, node->value, user_data))
      break;

    node = g_tree_node_next(node);
  }
}

/**
 * g_tree_new_full:
 * @key_compare_func: qsort()-style comparison function
 * @key_compare_data: data to pass to comparison function
 * @key_destroy_func: a function to free the memory allocated for the key
 *   used when removing the entry from the #GTree or %NULL if you don't
 *   want to supply such a function
 * @value_destroy_func: a function to free the memory allocated for the
 *   value used when removing the entry from the #GTree or %NULL if you
 *   don't want to supply such a function
 *
 * Creates a new #GTree like g_tree_new() and allows to specify functions
 * to free the memory allocated for the key and value that get called when
 * removing the entry from the #GTree.
 *
 * Returns: a newly allocated #GTree
 */
GTree *g_tree_new_full(GCompareDataFunc key_compare_func,
                       gpointer key_compare_data,
                       GDestroyNotify key_destroy_func,
                       GDestroyNotify value_destroy_func) {
  GTree *tree;

  assert(key_compare_func != NULL);

  // the full g_slice_new function is complete and can't port almostly,
  // so use the simplified function.
  // you can find the full implement of g_slice_new in gslice.c.
  // tree = g_slice_new (GTree);

  tree = g_slice_alloc(sizeof(GTree));
  tree->root = NULL;
  tree->key_compare = key_compare_func;
  tree->key_destroy_func = key_destroy_func;
  tree->value_destroy_func = value_destroy_func;
  tree->key_compare_data = key_compare_data;
  tree->nnodes = 0;
  tree->ref_count = 1;

  return tree;
}

/**
 * g_tree_new:
 * @key_compare_func: the function used to order the nodes in the #GTree.
 *   It should return values similar to the standard strcmp() function -
 *   0 if the two arguments are equal, a negative value if the first argument
 *   comes before the second, or a positive value if the first argument comes
 *   after the second.
 *
 * Creates a new #GTree.
 *
 * Returns: a newly allocated #GTree
 */
GTree *g_tree_new(GCompareFunc key_compare_func) {
  assert(key_compare_func != NULL);

  return g_tree_new_full((GCompareDataFunc)key_compare_func, NULL, NULL, NULL);
}
