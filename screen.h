/* screen.c */
#include <termios.h>

#include "buffer.h"  // re-export type

// declare, initialise (done in .c), re-export
// https://old.reddit.com/r/C_Programming/comments/q9of5i/_/hgyr78x/
// https://en.cppreference.com/w/c/language/storage_duration
struct SCREEN {
  // note: fields can be declared as immutable, but we don't do this, simply to
  // satisfy compiler

  // the initial state of termios. this is never modified: enable_raw_mode
  // makes a copy of this and modifies it to set the editor state, while
  // disable_raw_mode simply reverts to this state.
  // https://www.mankier.com/3/termios
  struct termios termios_mode;

  int rows;
  int columns;
};

extern struct SCREEN SCREEN;

void draw_rows(int n, struct STRING_BUFFER *buf);
void clear_screen(struct SCREEN *scr);
void disable_raw_mode(void);
void enable_raw_mode(struct SCREEN *scr);
void set_dimensions(struct SCREEN *scr);
