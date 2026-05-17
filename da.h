#ifndef DA_H_
#define DA_H_

// Lots of things in this header file were copied from nob.h, though
// most of these things were additionally simplified.
// https://github.com/tsoding/nob.h/blob/main/nob.h

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define TARE_DEF static inline

#ifndef DA_INIT_CAPACITY
#define DA_INIT_CAPACITY 64
#endif // DA_INIT_CAPACITY

#ifndef da_append
#define da_append(da, item)                                             \
  do {                                                                  \
    if ((da)->capacity < (da)->count + 1) {                             \
      if ((da)->capacity == 0) {                                        \
        (da)->capacity = DA_INIT_CAPACITY;                              \
      }                                                                 \
      while ((da)->capacity < (da)->count + 1) {                        \
        (da)->capacity *= 2;                                            \
      }                                                                 \
      (da)->items = realloc((da)->items, (da)->capacity * sizeof(*(da)->items)); \
      assert((da)->items != NULL && "allocation failed");               \
    }                                                                   \
    (da)->items[(da)->count++] = (item);                                \
  } while (0)
#endif // da_append

#define da_reserve(da, expected_capacity)                               \
  do {                                                                  \
    if ((expected_capacity) > (da)->capacity) {                         \
      if ((da)->capacity == 0) {                                        \
        (da)->capacity = DA_INIT_CAPACITY;                              \
      }                                                                 \
      while ((expected_capacity) > (da)->capacity) {                    \
        (da)->capacity *= 2;                                            \
      }                                                                 \
      (da)->items = realloc((da)->items, (da)->capacity * sizeof(*(da)->items)); \
      assert((da)->items != NULL && "allocation failed");               \
    }                                                                   \
  } while (0)

#define da_append_many(da, new_items, new_items_count)                  \
  do {                                                                  \
    da_reserve((da), (da)->count + (new_items_count));                  \
    memcpy((da)->items + (da)->count, (new_items), (new_items_count)*sizeof(*(da)->items)); \
    (da)->count += (new_items_count);                                   \
  } while (0)

#endif // DA_H_
