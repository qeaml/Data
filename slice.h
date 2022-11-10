/*
slice.h
-------
Defines the public API for a simple, untyped slice and a basic implementation.

To include the implementation with this header file, define QML_SLICE_IMPLEMENTATION

  #define QML_SLICE_IMPLEMENTATION
  #include "slice.h"

Basic usage:

  // allocate a slice for 100 integers
  slice_t my_slice = slice_alloc(100);
  // allocate 100 integers and place them in the slice
  for(int i = 0; i < 100; i++) {
    int* some_int = malloc(sizeof(int));
    *some_int = i;
    slice_append(&my_slice, some_int);
  }
  int sum = 0;
  // use slice_reduce to calculate their sum
  slice_reduce(&my_slice, &sum, sum_reduce_cb);
  // print out result
  printf("Sum of integers 0-99 is %d.\n", sum);
  // use slice_iter to free all the allocated memory
  slice_iter(&my_slice, free_iter_cb);

Customising behavior:

  // You can pick and choose which functions will be used for memory management.
  // These are expected to have the exact signatures of the stdlib functions
  // malloc, realloc and free respectively.
  #define QML_ALLOC my_alloc
  #define QML_REALLOC my_realloc
  #define QML_FREE my_free

  // If defined, functions that may grow the slice will be allowed to allocate
  // the slice it it has been freed.
  #define QML_SLICE_ALLOW_AUTO_ALLOC

*/

#ifndef QML_SLICE_DEFINED
#define QML_SLICE_DEFINED

#include <stddef.h>

typedef struct slice {
  size_t   len, cap;
    void **data;
} slice_t;

typedef int(slice_iter_cb_t)(size_t idx, void *value);
typedef int(slice_reduce_cb_t)(void *acc, size_t idx, void *value);

// Allocate a slice on the heap with length 0 and the given capacity.
slice_t slice_alloc(size_t cap);
// Append a pointer to the end of the slice, expanding it if necessary.
void slice_append(slice_t *slice, void *value);
// Tries to get the value at the given index, this will return NULL if the index
// is out of bounds or the slice is not valid.
void *slice_get(slice_t *slice, size_t idx);
// Sets the given index to the given value, expanding the slice if there is not
// enough allocated space and filling the newly allocated space with NULL.
void slice_set(slice_t *slice, size_t idx, void *value);
// Iterates through every element of the slice, calling the given callback on
// each (index, pointer) pair. If the callback returns 0, the iteration will be
// stopped.
void slice_iter(slice_t *slice, slice_iter_cb_t cb);
// Iterates all of the slice's elements similarily to slice_iter. Also provides
// an 'accumulator' argument that is also passed to the callback with each
// (index, pointer) pair. If the callback returns 0, the iteration will stop
// here as well.
void slice_reduce(slice_t *slice, void *acc, slice_reduce_cb_t cb);
// Shrinks the memory allocated for the slice to the slice's length with the
// additional overhead in case extra memory will be needed. The overhead may be
// 0 to only shrink.
void slice_shrink(slice_t *slice, size_t overhead);
// Frees the slice's allocated memory and sets it as invalid.
void slice_free(slice_t *slice);

#endif

#define QML_SLICE_IMPLEMENTATION
#define QML_SLICE_ALLOW_AUTO_ALLOC

#ifdef QML_SLICE_IMPLEMENTATION

#ifndef QML_ALLOC
#include <stdlib.h>
#define QML_ALLOC malloc
#endif

#ifndef QML_REALLOC
#include <stdlib.h>
#define QML_REALLOC realloc
#endif

#ifndef QML_FREE
#include <stdlib.h>
#define QML_FREE free
#endif

slice_t slice_alloc(size_t cap) {
  if(cap == 0)
    cap = 1;
  return (slice_t){ 0, cap, (void**)QML_ALLOC(sizeof(void*)*cap) };
}

void _slice_maybe_grow(slice_t *slice, size_t amt) {
  #ifdef QML_SLICE_ALLOW_AUTO_ALLOC
    if(slice->cap == 0 || slice->data == NULL) {
      *slice = slice_alloc(amt);
    }
  #endif

  if(slice->len + amt >= slice->cap) {
    slice->cap += slice->cap/2 + amt;
    slice->data = (void**)QML_REALLOC(slice->data, sizeof(void*)*slice->cap);
  }
}

void slice_append(slice_t *slice, void *value) {
  _slice_maybe_grow(slice, 1);
  slice->data[slice->len++] = value;
}

void *slice_get(slice_t *slice, size_t idx) {
  if(slice->cap == 0 || slice->data == NULL)
    return NULL;
  if(idx >= slice->len)
    return NULL;
  return slice->data[idx];
}

void slice_set(slice_t *slice, size_t idx, void *value) {
  if(idx >= slice->cap)
    _slice_maybe_grow(slice, idx - slice->cap);
  if(idx >= slice->len) {
    for(size_t i = slice->len; i < idx; i++)
      slice->data[i] = NULL;
    slice->len = idx+1;
  }
  slice->data[idx] = value;
}

void slice_iter(slice_t *slice, slice_iter_cb_t cb) {
  for(size_t i = 0; i < slice->len; i++)
    if(!cb(i, slice->data[i]))
      break;
}

void slice_reduce(slice_t *slice, void *acc, slice_reduce_cb_t cb) {
  for(size_t i = 0; i < slice->len; i++)
    if(!cb(acc, i, slice->data[i]))
      break;
}

void slice_shrink(slice_t *slice, size_t overhead) {
  slice->cap = slice->len + overhead;
  slice->data = (void **)QML_REALLOC(slice->data, sizeof(void*)*slice->cap);
}

void slice_free(slice_t *slice) {
  if(slice->cap == 0 || slice->data == NULL)
    return;

  slice->len = 0;
  slice->cap = 0;
  QML_FREE(slice->data);
  slice->data = NULL;
}

#endif
