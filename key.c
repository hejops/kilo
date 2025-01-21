#include "key.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "screen.h"
#include "util.h"

enum ReservedKey {
  // CURSOR_LEFT = 'h',
  // CURSOR_RIGHT = 'l',
  // CURSOR_UP = 'k',
  // CURSOR_DOWN = 'j',

  // these are unreachable from normal keyboard input
  CURSOR_LEFT = 1000,
  CURSOR_RIGHT,
  CURSOR_UP,
  CURSOR_DOWN,
};

// Read a single key and return it. For normal keyboard input, the integer
// value of the key returned should be in the range of a byte (0-255), but
// escape sequences (e.g. 'up') may be turned into 'special' keys that exceed
// this range.
int read_key(void) { /* {{{ */
  const int DEBUG = 0;

  char key = '\0';

  // read stdin, 1 char (byte) at a time (as configured in VMIN), every 0.1 s
  // (as configured in VTIME)
  if (read(STDIN_FILENO, &key, 1) == -1
      // we don't care about Cygwin
      // && errno != EAGAIN
  ) {
    panic("read");
  }

  if (DEBUG) {
    if (iscntrl(key)) {
      // escape sequences comprise 2 or more bytes
      // some bytes (e.g. 27 (Esc)) do not have a meaningful char representation
      printf("%d\r\n", key);
    } else {
      printf("%d (%c)\r\n", key, key);
    }
  }

  if (key == '\x1b') {  // this check is not the same is iscntrl!
    char seq[3];

    // try to read 2 more keys into seq, else bail and return Escape
    if (read(STDIN_FILENO, &seq[0], 1) != 1) {
      return '\x1b';
    }
    if (read(STDIN_FILENO, &seq[1], 1) != 1) {
      return '\x1b';
    }

    if (seq[0] == '[') {
      switch (seq[1]) {
        default:
          break;

        case 'A':  // the full sequence is therefore \x1b [ A
          return CURSOR_UP;
        case 'B':
          return CURSOR_DOWN;
        case 'C':
          return CURSOR_RIGHT;
        case 'D':
          return CURSOR_LEFT;
      }
    }

    return '\x1b';
  }

  return key;

} /* }}} */

void handle_key(int key) {
  switch (key) {
    default:  // should be declared first, apparently
      break;

    case KEY_QUIT:  // c-q closes the window in my window manager
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      printf("exited\n");
      exit(EXIT_SUCCESS);

    case CURSOR_DOWN:
    case 'j':
      SCREEN.cursor_row++;
      break;
    case CURSOR_UP:
    case 'k':
      SCREEN.cursor_row--;
      break;
    case CURSOR_RIGHT:
    case 'l':
      SCREEN.cursor_col++;
      break;
    case CURSOR_LEFT:
    case 'h':
      SCREEN.cursor_col--;
      break;
  }
}
