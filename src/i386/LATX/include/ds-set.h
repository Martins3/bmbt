#ifndef _DS_SET_H_
#define _DS_SET_H_

#include <stdlib.h>

typedef struct ds_set {
    int size;
    int num;
    void *data;
} ds_set;

#define DSSET_INIT(set, elemnum, elemsize)                  \
    do {                                                    \
        set->data = (void *)malloc((elemnum) * (elemsize)); \
        set->size = (elemnum);                              \
        set->num = 0;                                       \
    } while (0)

#define DSSET_FINI(set)          \
    do {                         \
        if (set->data != NULL) { \
            free(set->data);     \
        }                        \
    } while (0)

#define DSSET_INSERT(set, type, value)                                         \
    do {                                                                       \
        if (set->num == set->size) {                                           \
            set->data = mm_realloc(set->data, 2 * (set->size) * sizeof(type)); \
            set->size *= 2;                                                    \
        }                                                                      \
        type *__d = (type *)set->data;                                         \
        int __idx = set->num;                                                  \
        __d[__idx] = (value);                                                  \
        set->num = __idx + 1;                                                  \
    } while (0)

#define DSSET_COUNT(ret, set, type, value)     \
    do {                                       \
        ret = 0;                               \
        type *__d = set->data;                 \
        int __i = 0;                           \
        for (__i = 0; __i < set->num; ++__i) { \
            if (__d[__i] == (value))           \
                ret = ret + 1;                 \
        }                                      \
    } while (0)

#define DSSET_ISSUBSET(ret, type, set1, set2)          \
    do {                                               \
        ret = 1;                                       \
        type *__d1 = (type *)set1->data;               \
        type *__d2 = (type *)set2->data;               \
        int __i1 = 0;                                  \
        int __i2 = 0;                                  \
        int __find_2_in_1 = 0;                         \
        for (__i2 = 0; __i2 < set2->num; ++__i2) {     \
            __find_2_in_1 = 0;                         \
            type __d = __d2[__i2];                     \
            for (__i1 = 0; __i1 < set1->num; ++__i1) { \
                if (__d == __d1[__i1]) {               \
                    __find_2_in_1 = 1;                 \
                    break;                             \
                }                                      \
            }                                          \
            if (!__find_2_in_1) {                      \
                ret = 0;                               \
                break;                                 \
            }                                          \
        }                                              \
    } while (0)

#endif
