#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct STRING_BUFFER {
  char* contents;
  int length;
};

struct STRING_BUFFER buf_new(void) {
  // C always returns a copy, never a move
  struct STRING_BUFFER buf = {.contents = NULL, .length = 0};
  // printf("%p ", &buf);
  return buf;
}

// if buf has not been initialised, this function segfaults.
void buf_append(struct STRING_BUFFER* buf, const char* str, int len) {
  // realloc may or may not move the existing ptr
  char* new_addr = realloc(buf->contents, (unsigned)(buf->length + len));
  if (new_addr == NULL) {
    return;
  }

  // copy the new string to the end of the current one
  memcpy(&new_addr[buf->length], str, (unsigned)len);

  // update the current contents
  buf->contents = new_addr;
  buf->length += len;
}

void buf_write(struct STRING_BUFFER buf) {
  write(STDOUT_FILENO, buf.contents, (unsigned)buf.length);
}

void buf_free(struct STRING_BUFFER* buf) { free(buf->contents); }
