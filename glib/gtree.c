#include "gtree.h"
#include "gatomic.h"
#include "gmem.h"
#include "gmessages.h"
#include "gslice.h"
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

static GTreeNode *g_tree_insert_internal(GTree *tree, gpointer key,
                                         gpointer value, gboolean replace);

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
  GTreeNode *node = g_slice_new(GTreeNode);

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
static GTreeNode *g_tree_insert_internal(GTree *tree, gpointer key,
                                         gpointer value, gboolean replace) {
  GTreeNode *node, *retnode;
  GTreeNode *path[MAX_GTREE_HEIGHT];
  int idx;

  g_return_val_if_fail(tree != NULL, NULL);

  if (!tree->root) {
    tree->root = g_tree_node_new(key, value);
    tree->nnodes++;
    return tree->root;
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

      return node;
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

        retnode = child;
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

        retnode = child;
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

  return retnode;
}

/**
 * g_tree_insert_node:
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
 * The cost of maintaining a balanced tree while inserting new key/value
 * result in a O(n log(n)) operation where most of the other operations
 * are O(log(n)).
 *
 * Returns: (transfer none): the inserted (or set) node.
 *
 * Since: 2.68
 */
GTreeNode *g_tree_insert_node(GTree *tree, gpointer key, gpointer value) {
  GTreeNode *node;

  g_return_val_if_fail(tree != NULL, NULL);

  node = g_tree_insert_internal(tree, key, value, FALSE);

#ifdef G_TREE_DEBUG
  g_tree_node_check(tree->root);
#endif

  return node;
}

/**
 * g_tree_insert:
 * @tree: a #GTree
 * @key: the key to insert
 * @value: the value corresponding to the key
 *
 * Inserts a key/value pair into a #GTree.
 *
 * Inserts a new key and value into a #GTree as g_tree_insert_node() does,
 * only this function does not return the inserted or set node.
 */
void g_tree_insert(GTree *tree, gpointer key, gpointer value) {
  g_tree_insert_node(tree, key, value);
}

/**
 * g_tree_node_first:
 * @tree: a #GTree
 *
 * Returns the first in-order node of the tree, or %NULL
 * for an empty tree.
 *
 * Returns: (nullable) (transfer none): the first node in the tree
 *
 * Since: 2.68
 */
GTreeNode *g_tree_node_first(GTree *tree) {
  GTreeNode *tmp;

  g_return_val_if_fail(tree != NULL, NULL);

  if (!tree->root)
    return NULL;

  tmp = tree->root;

  while (tmp->left_child)
    tmp = tmp->left;

  return tmp;
}

/**
 * g_tree_node_last:
 * @tree: a #GTree
 *
 * Returns the last in-order node of the tree, or %NULL
 * for an empty tree.
 *
 * Returns: (nullable) (transfer none): the last node in the tree
 *
 * Since: 2.68
 */
GTreeNode *g_tree_node_last(GTree *tree) {
  GTreeNode *tmp;

  g_return_val_if_fail(tree != NULL, NULL);

  if (!tree->root)
    return NULL;

  tmp = tree->root;

  while (tmp->right_child)
    tmp = tmp->right;

  return tmp;
}

/**
 * g_tree_node_previous
 * @node: a #GTree node
 *
 * Returns the previous in-order node of the tree, or %NULL
 * if the passed node was already the first one.
 *
 * Returns: (nullable) (transfer none): the previous node in the tree
 *
 * Since: 2.68
 */
GTreeNode *g_tree_node_previous(GTreeNode *node) {
  GTreeNode *tmp;

  g_return_val_if_fail(node != NULL, NULL);

  tmp = node->left;

  if (node->left_child)
    while (tmp->right_child)
      tmp = tmp->right;

  return tmp;
}

/**
 * g_tree_node_next
 * @node: a #GTree node
 *
 * Returns the next in-order node of the tree, or %NULL
 * if the passed node was already the last one.
 *
 * Returns: (nullable) (transfer none): the next node in the tree
 *
 * Since: 2.68
 */
GTreeNode *g_tree_node_next(GTreeNode *node) {
  GTreeNode *tmp;

  g_return_val_if_fail(node != NULL, NULL);

  tmp = node->right;

  if (node->right_child)
    while (tmp->left_child)
      tmp = tmp->left;

  return tmp;
}

/**
 * g_tree_remove_all:
 * @tree: a #GTree
 *
 * Removes all nodes from a #GTree and destroys their keys and values,
 * then resets the #GTree’s root to %NULL.
 *
 * Since: 2.70
 */
void g_tree_remove_all(GTree *tree) {
  GTreeNode *node;
  GTreeNode *next;

  g_return_if_fail(tree != NULL);

  node = g_tree_node_first(tree);

  while (node) {
    next = g_tree_node_next(node);

    if (tree->key_destroy_func)
      tree->key_destroy_func(node->key);
    if (tree->value_destroy_func)
      tree->value_destroy_func(node->value);
    g_slice_free(GTreeNode, node);

#ifdef G_TREE_DEBUG
    g_assert(tree->nnodes > 0);
    tree->nnodes--;
#endif

    node = next;
  }

#ifdef G_TREE_DEBUG
  g_assert(tree->nnodes == 0);
#endif

  tree->root = NULL;
#ifndef G_TREE_DEBUG
  tree->nnodes = 0;
#endif
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
 * g_tree_lookup_node:
 * @tree: a #GTree
 * @key: the key to look up
 *
 * Gets the tree node corresponding to the given key. Since a #GTree is
 * automatically balanced as key/value pairs are added, key lookup
 * is O(log n) (where n is the number of key/value pairs in the tree).
 *
 * Returns: (nullable) (transfer none): the tree node corresponding to
 *          the key, or %NULL if the key was not found
 *
 * Since: 2.68
 */
GTreeNode *g_tree_lookup_node(GTree *tree, gconstpointer key) {
  g_return_val_if_fail(tree != NULL, NULL);

  return g_tree_find_node(tree, key);
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

  node = g_tree_lookup_node(tree, key);

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
  g_return_val_if_fail(tree != NULL, 0);

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
  g_return_val_if_fail(tree != NULL, NULL);

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
  g_return_if_fail(tree != NULL);

  if (g_atomic_int_dec_and_test(&tree->ref_count)) {
    g_tree_remove_all(tree);
    g_slice_free(GTree, tree);
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
  g_return_if_fail(tree != NULL);

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

  g_return_if_fail(tree != NULL);

  if (!tree->root)
    return;

  node = g_tree_node_first(tree);

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

  g_return_val_if_fail(key_compare_func != NULL, NULL);

  tree = g_slice_new(GTree);
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
  g_return_val_if_fail(key_compare_func != NULL, NULL);

  return g_tree_new_full((GCompareDataFunc)key_compare_func, NULL, NULL, NULL);
}

/* internal remove routine */
static gboolean g_tree_remove_internal(GTree *tree, gconstpointer key,
                                       gboolean steal) {
  GTreeNode *node, *parent, *balance;
  GTreeNode *path[MAX_GTREE_HEIGHT];
  int idx;
  gboolean left_node;

  g_return_val_if_fail(tree != NULL, FALSE);

  if (!tree->root)
    return FALSE;

  idx = 0;
  path[idx++] = NULL;
  node = tree->root;

  while (1) {
    int cmp = tree->key_compare(key, node->key, tree->key_compare_data);

    if (cmp == 0)
      break;
    else if (cmp < 0) {
      if (!node->left_child)
        return FALSE;

      path[idx++] = node;
      node = node->left;
    } else {
      if (!node->right_child)
        return FALSE;

      path[idx++] = node;
      node = node->right;
    }
  }

  /* The following code is almost equal to g_tree_remove_node,
   * except that we do not have to call g_tree_node_parent.
   */
  balance = parent = path[--idx];
  g_assert(!parent || parent->left == node || parent->right == node);
  left_node = (parent && node == parent->left);

  if (!node->left_child) {
    if (!node->right_child) {
      if (!parent)
        tree->root = NULL;
      else if (left_node) {
        parent->left_child = FALSE;
        parent->left = node->left;
        parent->balance += 1;
      } else {
        parent->right_child = FALSE;
        parent->right = node->right;
        parent->balance -= 1;
      }
    } else /* node has a right child */
    {
      GTreeNode *tmp = g_tree_node_next(node);
      tmp->left = node->left;

      if (!parent)
        tree->root = node->right;
      else if (left_node) {
        parent->left = node->right;
        parent->balance += 1;
      } else {
        parent->right = node->right;
        parent->balance -= 1;
      }
    }
  } else /* node has a left child */
  {
    if (!node->right_child) {
      GTreeNode *tmp = g_tree_node_previous(node);
      tmp->right = node->right;

      if (parent == NULL)
        tree->root = node->left;
      else if (left_node) {
        parent->left = node->left;
        parent->balance += 1;
      } else {
        parent->right = node->left;
        parent->balance -= 1;
      }
    } else /* node has a both children (pant, pant!) */
    {
      GTreeNode *prev = node->left;
      GTreeNode *next = node->right;
      GTreeNode *nextp = node;
      int old_idx = idx + 1;
      idx++;

      /* path[idx] == parent */
      /* find the immediately next node (and its parent) */
      while (next->left_child) {
        path[++idx] = nextp = next;
        next = next->left;
      }

      path[old_idx] = next;
      balance = path[idx];

      /* remove 'next' from the tree */
      if (nextp != node) {
        if (next->right_child)
          nextp->left = next->right;
        else
          nextp->left_child = FALSE;
        nextp->balance += 1;

        next->right_child = TRUE;
        next->right = node->right;
      } else
        node->balance -= 1;

      /* set the prev to point to the right place */
      while (prev->right_child)
        prev = prev->right;
      prev->right = next;

      /* prepare 'next' to replace 'node' */
      next->left_child = TRUE;
      next->left = node->left;
      next->balance = node->balance;

      if (!parent)
        tree->root = next;
      else if (left_node)
        parent->left = next;
      else
        parent->right = next;
    }
  }

  /* restore balance */
  if (balance)
    while (1) {
      GTreeNode *bparent = path[--idx];
      g_assert(!bparent || bparent->left == balance ||
               bparent->right == balance);
      left_node = (bparent && balance == bparent->left);

      if (balance->balance < -1 || balance->balance > 1) {
        balance = g_tree_node_balance(balance);
        if (!bparent)
          tree->root = balance;
        else if (left_node)
          bparent->left = balance;
        else
          bparent->right = balance;
      }

      if (balance->balance != 0 || !bparent)
        break;

      if (left_node)
        bparent->balance += 1;
      else
        bparent->balance -= 1;

      balance = bparent;
    }

  if (!steal) {
    if (tree->key_destroy_func)
      tree->key_destroy_func(node->key);
    if (tree->value_destroy_func)
      tree->value_destroy_func(node->value);
  }

  g_slice_free(GTreeNode, node);

  tree->nnodes--;

  return TRUE;
}

/**
 * g_tree_remove:
 * @tree: a #GTree
 * @key: the key to remove
 *
 * Removes a key/value pair from a #GTree.
 *
 * If the #GTree was created using g_tree_new_full(), the key and value
 * are freed using the supplied destroy functions, otherwise you have to
 * make sure that any dynamically allocated values are freed yourself.
 * If the key does not exist in the #GTree, the function does nothing.
 *
 * The cost of maintaining a balanced tree while removing a key/value
 * result in a O(n log(n)) operation where most of the other operations
 * are O(log(n)).
 *
 * Returns: %TRUE if the key was found (prior to 2.8, this function
 *     returned nothing)
 */
gboolean g_tree_remove(GTree *tree, gconstpointer key) {
  gboolean removed;

  g_return_val_if_fail(tree != NULL, FALSE);
  removed = g_tree_remove_internal(tree, key, FALSE);

#ifdef G_TREE_DEBUG
  g_tree_node_check(tree->root);
#endif

  return removed;
}
