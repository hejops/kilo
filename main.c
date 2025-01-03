#include <stdlib.h>

#include "key.h"
#include "screen.h"

// https://viewsourcecode.org/snaptoken/kilo

// similar to pyright, clangd has no concept of workspace. files must be opened
// before they can be scanned.

// functions must be declared/included before they can be used
int main(void) {
  // init

  enable_raw_mode(&SCREEN);
  set_dimensions(&SCREEN);
  clear_screen(&SCREEN);

  while (1) {
    char key = read_key();
    handle_key(key);
  };

  // clear_screen(&SCREEN);
  return EXIT_SUCCESS;
}
