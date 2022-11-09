/*
flexbuf.h
---------
Defines the public API of the flex_buf and an optional implementation. Every
function has a comment describing it.

To include the implementation with this header file, make sure to define
QML_FLEXBUF_IMPLEMENTATION beforehand:

  #define QML_FLEXBUF_IMPLEMENTATION
  #include "flexbuf.h"

Basic usage:

  flex_buf_t my_buf = buf_alloc(10);   // allocate a buffer of 10 characters
  buf_append_lit(&my_buf, "Hello");    // append the literal "Hello"
  buf_append(&my_buf, ' ');            // append a single space character
  buf_append_n(&my_buf, "worlddd", 5); // append 5 characters of "worlddd"
  buf_append(&my_buf, '!');
  char final_str[20];                  // allocate a final string on the stack
  buf_finalize(my_buf, final_str);     // copy the buffer's state to the string
  buf_free(my_buf);                    // deallocate the buffer
  printf(final_str); // Hello world!   // print the string

Customising behavior:

  // You can pick and choose which functions will be used for memory management.
  // These are expected to have the exact signatures of the stdlib functions
  // malloc, realloc and free respectively.
  #define QML_ALLOC my_alloc
  #define QML_REALLOC my_realloc
  #define QML_FREE my_free

  // If defined, functions that may grow the buffer will be allowed to allocate
  // the buffer it it has been already freed.
  #define QML_FLEXBUF_ALLOW_AUTO_ALLOC

  // If defined, the buf_finalize function will also free the underlying data
  // after copying to the output array.
  #define QML_FLEXBUF_FREE_ON_FINALIZE
  // or
  #define QML_FLEXBUF_FREE_ON_FINALISE

qeaml 9.11.2022
*/

#ifndef QML_FLEXBUF_DEFINED
#define QML_FLEXBUF_DEFINED

#include <stddef.h>

typedef struct flex_buf {
  size_t  size, cap;
    char *data;
} flex_buf_t;

// Allocate a buffer on the heap with size 0 and the given capacity.
flex_buf_t buf_alloc(size_t cap);
// Append a single character to the buffer, growing it if necessary.
void buf_append(flex_buf_t *buf, char c);
// Append n characters to the buffer, growing it if necessary.
void buf_append_n(flex_buf_t *buf, char *src, size_t amt);
// Append the NULL-terminated string to the buffer, growing it if necessary.
void buf_append_cstr(flex_buf_t *buf, char *str);
// Concatenate the other buffer to this buffer, growing it if necessary.
void buf_concat(flex_buf_t *buf, flex_buf_t other);
// Shrink the capacity of this buffer to it's current size and reallocate the
// underlying memory to this new, smaller capacity.
void buf_shrink(flex_buf_t *buf);
// Copy the buffer's current state to the output array with a NULL character
// at the end. If the correct macro is defined, this will also call buf_free.
void buf_finalize(flex_buf_t *buf, char *out);
// Free the buffer's underlying data and replace the now-invalid pointer with
// NULL.
void buf_free(flex_buf_t buf);

// UK-friendly
#define buf_finalise buf_finalize
#define buf_append_lit(buf, lit) buf_append_n(buf, lit, sizeof(lit))

#endif // QML_FLEXBUF_DEFINED

#ifdef QML_FLEXBUF_IMPLEMENTATION

#include <string.h>

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

flex_buf_t buf_alloc(size_t cap) {
  return (flex_buf_t){ 0, cap, (char *)QML_ALLOC(cap) };
}

inline void _buf_maybe_grow(flex_buf_t* buf, size_t amt) {
  #ifdef QML_FLEXBUF_ALLOW_AUTO_ALLOC
    if(buf->cap == 0 || buf->data == NULL) {
      *buf = buf_alloc(amt);
    }
  #endif

  if(buf->size + amt >= buf->cap) {
    buf->cap += buf->cap/2 + amt;
    buf->data = (char *)QML_REALLOC(buf->data, buf->cap);
  }
}

void buf_append(flex_buf_t *buf, char c) {
  _buf_maybe_grow(buf, 1);
  buf->data[buf->size++] = c;
}

void buf_append_n(flex_buf_t *buf, char *src, size_t amt) {
  _buf_maybe_grow(buf, amt);
  for(size_t i = 0; i < amt; i++)
    buf->data[buf->size++] = src[i];
}

void buf_append_cstr(flex_buf_t *buf, char *str) {
  size_t amt = strlen(str);
  buf_append_n(buf, str, amt);
}

void buf_shrink(flex_buf_t *buf) {
  buf->cap = buf->size + 1;
  buf->data = (char *)QML_REALLOC(buf->data, buf->cap);
}

void buf_finalize(flex_buf_t buf, char *out) {
  memcpy(out, buf.data, buf.size);
  out[buf.size] = 0;

  #if defined(QML_FLEXBUF_FREE_ON_FINALIZE) || defined(QML_FLEXBUF_FREE_ON_FINALISE)
    buf_free(buf);
  #endif
}

void buf_free(flex_buf_t buf) {
  if(buf.cap == 0 || buf.data == NULL)
    return;

  buf.size = 0;
  buf.cap = 0;
  QML_FREE(buf.data);
  buf.data = NULL;
}

#endif // QML_FLEXBUF_IMPLEMENTATION
