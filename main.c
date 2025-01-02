#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "key.c"
#include "screen.c"

// https://viewsourcecode.org/snaptoken/kilo

// functions must be declared/included before they can be used
int main(void) {
  enable_raw_mode(&EDITOR);
  set_window_dimensions(&EDITOR);
  clear_screen();

  while (1) {
    char c = read_key();
    handle_key(c);
  };

  return EXIT_SUCCESS;
}
