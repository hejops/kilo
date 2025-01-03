#include <stdlib.h>

#include "key.c"
#include "screen.c"

// https://viewsourcecode.org/snaptoken/kilo

// functions must be declared/included before they can be used
int main(void) {
  // init
  enable_raw_mode(&SCREEN);
  set_dimensions(&SCREEN);
  clear_screen(&SCREEN);

  // draw_rows(SCREEN.rows);
  // write(STDOUT_FILENO, "\x1b[H", 3);  // move cursor back to home

  while (1) {
    char key = read_key();
    handle_key(key);
  };

  // clear_screen(&SCREEN);
  return EXIT_SUCCESS;
}
