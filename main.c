#include <stdlib.h>

#include "key.h"
#include "screen.h"

// https://viewsourcecode.org/snaptoken/kilo

// similar to pyright, clangd has no concept of workspace. files must be opened
// before they can be scanned.

// on definitions (.c) and declarations (.h): lsp_definitions goes to the .c
// file if it is open, otherwise it goes to the .h file. function doc will no
// longer be displayed.

// on program structure, organisation:
// - including .c from .c is frowned upon
// - best practice appears to be:
//   - .c: include the .h, and "children", define impls and global vars, don't
//   declare structs
//   - .h: include "neighbours", declare the struct (there should only be one),
//   re-export global vars (from .c)
//
// https://stackoverflow.com/questions/232693
// https://stackoverflow.com/questions/10448047
// https://github.com/ObaraEmmanuel/NES/tree/master/src
// https://websites.umich.edu/~eecs381/handouts/CHeaderFileGuidelines.pdf
// https://softwareengineering.stackexchange.com/a/167751

// functions must be declared/included before they can be used
int main(void) {
  // init

  scr_raw_mode(&SCREEN);
  SCREEN.cursor_row = 0;
  SCREEN.cursor_col = 0;
  scr_set_dims(&SCREEN);
  scr_clear(&SCREEN);

  while (1) {
    char key = read_key();
    handle_key(key);
    scr_clear(&SCREEN);
  };

  // clear_screen(&SCREEN);
  return EXIT_SUCCESS;
}
