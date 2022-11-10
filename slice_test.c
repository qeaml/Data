#include "slice.h"
#define QML_SLICE_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>

int sum_reduce_cb(void *acc, size_t idx, void *val) {
  // memory safety? what's that??
  *(int *)acc += *(int *)val;
  return 1;
}

int free_iter_cb(size_t idx, void* val) {
  free(val);
  return 1;
}

// this entire program can be rewritten with just an array
// but it's better than nothing
int main(int argc, char* argv[]) {
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
}
