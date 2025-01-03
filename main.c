#include <stdlib.h>

#include "key.h"
#include "screen.h"

// https://viewsourcecode.org/snaptoken/kilo

// similar to pyright, clangd has no concept of workspace. files must be opened
// before they can be scanned.

// on definitions (c) and declarations (h): lsp_definitions goes to the .c file
// if it is open, otherwise it goes to the .h file. function doc will no longer
// be displayed.

// on program structure, organisation:
// - including .c from .c is frowned upon
// - best practice appears to be:
//   - .c: include the .h, and "children", define impls, don't declare structs
//   - .h: include "neighbours", declare the struct (there should only be one)
//
// https://stackoverflow.com/questions/232693
// https://stackoverflow.com/questions/10448047
// https://github.com/ObaraEmmanuel/NES/tree/master/src

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
