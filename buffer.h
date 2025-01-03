/* buffer.c */
struct STRING_BUFFER {
  char *contents;
  int length;
};

struct STRING_BUFFER buf_new(void);
void buf_append(struct STRING_BUFFER *buf, const char *str, int len);
void buf_write(struct STRING_BUFFER buf);
void buf_free(struct STRING_BUFFER *buf);
