#pragma once

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "screen.c"

// switch/case only accepts compile-time constants
// char mod_ctrl(char c) { return c & 0x1f; }
//
// 0x1f = 0b0001_1111
// this clears the upper 3 bits; the uppermost (7th, 0-indexed) bit is
// actually irrelevant (?) in ascii, which only uses 7 bits.
// https://old.reddit.com/r/learnprogramming/comments/6vyoxj/_/dm400ar/
#define mod_ctrl(k) (k & 0x1f)

#define KEY_QUIT mod_ctrl('x')

const int DEBUG = 0;

// read a single key (byte) and return it
char read_key(void) { /* {{{ */
  char key = '\0';

  // read stdin, 1 char (byte) at a time (as configured in VMIN), every 0.1 s
  // (as configured in VTIME)
  if (read(STDIN_FILENO, &key, 1) == -1
      // we don't care about Cygwin
      // && errno != EAGAIN
  ) {
    panic("read");
  };

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
      // c-q closes the window in my window manager
    case KEY_QUIT:
      printf("exited\n");
      exit(EXIT_SUCCESS);
      break;
  };
}
