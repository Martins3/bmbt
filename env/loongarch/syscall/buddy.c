#include "buddy.h"
#include "internal.h"
#include <asm/page.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define NODE_UNUSED 0
#define NODE_USED 1
#define NODE_SPLIT 2
#define NODE_FULL 3

struct buddy {
  int level;
  long base;
  uint8_t tree[1];
};

static long alloc_pages(long pages) {
  return kernel_mmap(0, pages << PAGE_SHIFT, 0, 0, -1, 0, 0);
}

struct buddy *buddy_new(int level) {
  int size = 1 << level;
  int buddy_size = sizeof(struct buddy) + sizeof(uint8_t) * (size * 2 - 2);
  int buddy_size_page = (buddy_size + PAGE_SIZE - 1) / PAGE_SIZE;
  kern_printf("buddy system need %d pages \n", (buddy_size_page + size));
  struct buddy *self = (void *)alloc_pages(buddy_size_page + size);
  self->level = level;
  self->base = (long)self + PAGE_SIZE * buddy_size_page;
  memset(self->tree, NODE_UNUSED, size * 2 - 1);
  return self;
}

long buddy_base(struct buddy *self) { return self->base; }

/* void buddy_delete(struct buddy *self) { free(self); } */

static inline int is_pow_of_2(uint32_t x) { return !(x & (x - 1)); }

static inline uint32_t next_pow_of_2(uint32_t x) {
  if (is_pow_of_2(x))
    return x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

static inline int _index_offset(int index, int level, int max_level) {
  return ((index + 1) - (1 << level)) << (max_level - level);
}

static void _mark_parent(struct buddy *self, int index) {
  for (;;) {
    int buddy = index - 1 + (index & 1) * 2;
    if (buddy > 0 &&
        (self->tree[buddy] == NODE_USED || self->tree[buddy] == NODE_FULL)) {
      index = (index + 1) / 2 - 1;
      self->tree[index] = NODE_FULL;
    } else {
      return;
    }
  }
}

int buddy_alloc(struct buddy *self, int s) {
  int size;
  if (s == 0) {
    size = 1;
  } else {
    size = (int)next_pow_of_2(s);
  }
  int length = 1 << self->level;

  if (size > length)
    return -1;

  int index = 0;
  int level = 0;

  while (index >= 0) {
    if (size == length) {
      if (self->tree[index] == NODE_UNUSED) {
        self->tree[index] = NODE_USED;
        _mark_parent(self, index);
        return _index_offset(index, level, self->level);
      }
    } else {
      // size < length
      switch (self->tree[index]) {
      case NODE_USED:
      case NODE_FULL:
        break;
      case NODE_UNUSED:
        // split first
        self->tree[index] = NODE_SPLIT;
        self->tree[index * 2 + 1] = NODE_UNUSED;
        self->tree[index * 2 + 2] = NODE_UNUSED;
      default:
        index = index * 2 + 1;
        length /= 2;
        level++;
        continue;
      }
    }
    if (index & 1) {
      ++index;
      continue;
    }
    for (;;) {
      level--;
      length *= 2;
      index = (index + 1) / 2 - 1;
      if (index < 0)
        return -1;
      if (index & 1) {
        ++index;
        break;
      }
    }
  }

  return -1;
}

static void _combine(struct buddy *self, int index) {
  for (;;) {
    int buddy = index - 1 + (index & 1) * 2;
    if (buddy < 0 || self->tree[buddy] != NODE_UNUSED) {
      self->tree[index] = NODE_UNUSED;
      while (((index = (index + 1) / 2 - 1) >= 0) &&
             self->tree[index] == NODE_FULL) {
        self->tree[index] = NODE_SPLIT;
      }
      return;
    }
    index = (index + 1) / 2 - 1;
  }
}

void buddy_free(struct buddy *self, int offset) {
  kern_assert(offset < (1 << self->level));
  int left = 0;
  int length = 1 << self->level;
  int index = 0;

  for (;;) {
    switch (self->tree[index]) {
    case NODE_USED:
      kern_assert(offset == left);
      _combine(self, index);
      return;
    case NODE_UNUSED:
      kern_assert(0);
      return;
    default:
      length /= 2;
      if (offset < left + length) {
        index = index * 2 + 1;
      } else {
        left += length;
        index = index * 2 + 2;
      }
      break;
    }
  }
}

int buddy_size(struct buddy *self, int offset) {
  kern_assert(offset < (1 << self->level));
  int left = 0;
  int length = 1 << self->level;
  int index = 0;

  for (;;) {
    switch (self->tree[index]) {
    case NODE_USED:
      kern_assert(offset == left);
      return length;
    case NODE_UNUSED:
      kern_assert(0);
      return length;
    default:
      length /= 2;
      if (offset < left + length) {
        index = index * 2 + 1;
      } else {
        left += length;
        index = index * 2 + 2;
      }
      break;
    }
  }
}

static void _dump(struct buddy *self, int index, int level) {
  switch (self->tree[index]) {
  case NODE_UNUSED:
    kern_printf("(%d:%d)", _index_offset(index, level, self->level),
                1 << (self->level - level));
    break;
  case NODE_USED:
    kern_printf("[%d:%d]", _index_offset(index, level, self->level),
                1 << (self->level - level));
    break;
  case NODE_FULL:
    kern_printf("{");
    _dump(self, index * 2 + 1, level + 1);
    _dump(self, index * 2 + 2, level + 1);
    kern_printf("}");
    break;
  default:
    kern_printf("(");
    _dump(self, index * 2 + 1, level + 1);
    _dump(self, index * 2 + 2, level + 1);
    kern_printf(")");
    break;
  }
}

void buddy_dump(struct buddy *self) {
  _dump(self, 0, 0);
  kern_printf("\n");
}
