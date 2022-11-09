#define QML_FLEXBUF_IMPLEMENTATION
#define QML_FLEXBUF_FREE_ON_FINALIZE
#include "flex_buf.h"
#include <stddef.h>
#include <stdio.h>

int main(int argc, char** argv) {
  flex_buf_t buf = buf_alloc(argc*10);
  buf_append_lit(&buf, "Command line:\n");
  for(size_t i = 0; i < argc-1; i++) {
    buf_append_cstr(&buf, argv[i]);
    buf_append(&buf, ' ');
  }
  buf_append_cstr(&buf, argv[argc-1]);
  char final[buf.size+1];
  buf_finalize(&buf, final);
  printf("%s\n", final);
  return 0;
}
