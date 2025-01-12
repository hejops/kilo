#include "key.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "screen.h"
#include "util.h"

// read a single key (byte) and return it
char read_key(void) { /* {{{ */
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

  return key;
} /* }}} */

void handle_key(char key) {
  switch (key) {
    default:  // should be declared first, apparently
      break;

      // c-q closes the window in my window manager
    case KEY_QUIT:
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      printf("exited\n");
      exit(EXIT_SUCCESS);

    case 'j':
      SCREEN.cursor_row++;
      break;
    case 'k':
      SCREEN.cursor_row--;
      break;
    case 'l':
      SCREEN.cursor_row++;
      break;
    case 'h':
      SCREEN.cursor_row--;
      break;
  }
}
