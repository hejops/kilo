#include <stdlib.h>

#include "key.c"
#include "screen.c"

// https://viewsourcecode.org/snaptoken/kilo

// functions must be declared/included before they can be used
int main(void) {
  enable_raw_mode(&EDITOR);
  set_window_dimensions(&EDITOR);
  clear_screen();

  while (1) {
    char key = read_key();
    handle_key(key);
  };

  return EXIT_SUCCESS;
}
