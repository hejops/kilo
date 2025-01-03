#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "buffer.c"
#include "util.c"

// for consistency, we only use the word 'screen', not 'window', or 'editor'.

struct SCREEN {  // struct declaration and initialisation
  // note: fields can be declared as immutable, but we don't do this, simply to
  // satisfy compiler

  // the initial state of termios. this is never modified: enable_raw_mode
  // makes a copy of this and modifies it to set the editor state, while
  // disable_raw_mode simply reverts to this state.
  // https://www.mankier.com/3/termios
  struct termios termios_mode;

  int rows;
  int columns;
} SCREEN;

void draw_rows(int n, struct STRING_BUFFER* b) {
  for (int row = 0; row < n - 1; row++) {
    buf_append(b, "~\r\n", 3);
  }
  buf_append(b, "~", 1);
}

void clear_screen(struct SCREEN* s) {
  struct STRING_BUFFER buf = {.contents = NULL, .length = 0};

  // // C always returns a copy, never a move
  // struct STRING_BUFFER buf = buf_new();
  // printf("%p ", &buf);

  // note: there is no way to check whether a var has been initialised (without
  // segfaulting)
  // https://stackoverflow.com/a/36490974

  // \x1b = 27 = Esc

  // [2J = erase all lines
  // https://vt100.net/docs/vt100-ug/chapter3.html#ED
  // if n < len(string), probably UB
  buf_append(&buf, "\x1b[2J", 4);

  // [row;colH = place cursor at row,col
  // with no args, row=col=1 (1-indexed)
  // https://vt100.net/docs/vt100-ug/chapter3.html#CUP
  buf_append(&buf, "\x1b[H", 3);

  draw_rows(s->rows, &buf);

  buf_append(&buf, "\x1b[H", 3);  // move cursor back to home

  buf_write(&buf);
  buf_free(&buf);
}

// restore the initial state of termios on exit. this prevents the user from
// needing to manually call `reset` on exit
//
// because this function is called by atexit, it cannot accept any args.
void disable_raw_mode(void) {
  // TCSAFLUSH ensures the rest of the (unread) chars are not sent back to the
  // parent process
  // https://www.gnu.org/software/libc/manual/html_node/Mode-Functions.html#index-TCSAFLUSH
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &SCREEN.termios_mode) == -1) {
    panic("tcsetattr");
  };
}

void enable_raw_mode(struct SCREEN* scr) { /* {{{ */
  // fails if stdin is passed to the program, or if a file descriptor is passed
  // (./main <main.c)
  if (tcgetattr(STDIN_FILENO, &scr->termios_mode) == -1) {
    panic("tcgetattr");
  };
  atexit(disable_raw_mode);  // defer-ish

  struct termios raw = scr->termios_mode;  // assignment without ptr = copy

  // note: terminal bindings still intercept stdin

  // WARN: since all flags are just a sequence of bits, there is no
  // type-checking. (un)setting a bit in the wrong flag may have unexpected
  // results.

  raw.c_lflag &= ~(

      // disable printing of stdin, just as in password input. terminals that
      // pick this up will display the lock icon.
      (unsigned)ECHO |

      // readchar; "input is available immediately (without the user having to
      // type a line-delimiter character), no input processing is performed, and
      // line editing is disabled"
      // https://www.mankier.com/3/termios#Description-Canonical_and_noncanonical_mode
      ICANON |

      // ignore signals (e.g. SIGINT ctrl-c)
      ISIG |

      // disable input processing (ctrl-v)
      IEXTEN

  );

  raw.c_iflag &= ~(

      // ignore XON/XOFF (ctrl-s/q)
      // https://en.wikipedia.org/wiki/Software_flow_control
      (unsigned)IXON |

      // disable translation of \r (ctrl-m) to \n (ctrl-j)
      ICRNL |

      // misc; should not be relevant to modern terminal emulators
      // https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html#miscellaneous-flags
      BRKINT | INPCK | ISTRIP

  );

  // disable output processing; i.e., disable translation of \n to \r\n.
  // this means we need to insert \r's ourselves.
  raw.c_oflag &= ~((unsigned)OPOST);

  raw.c_cflag |= ~((unsigned)CS8);

  // Minimum number of characters for noncanonical read (MIN).
  // when 0, read() returns as soon as there is any input to be read.
  raw.c_cc[VMIN] = 0;

  // Timeout in deciseconds for noncanonical read (TIME).
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    panic("tcsetattr");
  };

} /* }}} */

void set_dimensions(struct SCREEN* scr) { /* {{{ */
  struct winsize wsz;
  // note: TIOCGWINSZ is indirectly included via asm/termbits.h
  // https://man.archlinux.org/man/core/man-pages/TIOCGWINSZ.2const.en
  // https://stackoverflow.com/q/67098208
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsz) != -1 && wsz.ws_col > 0) {
    scr->rows = wsz.ws_row;
    scr->columns = wsz.ws_col;
    return;
  }

  // fallback: determine cursor position manually

  // unlike H, C+B ensures cursor remains within the bounds
  if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
    panic("write (move cursor)");
  };

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
    panic("write (query cursor position)");
  };

  const int bufsize = 32;
  char buf[bufsize];
  for (unsigned int i = 0; i < sizeof(buf); i++) {
    // raw output: ^[[43;170R
    if (read(STDOUT_FILENO, &buf[i], 1) != 1 || buf[i] == 'R') {
      buf[i] = '\0';
      break;
    }
  }

  // note: sscanf is marked as unsafe. sscanf_s is microsoft-specific. strtol is
  // probably the way to go, but this may involve strtok
  // https://stackoverflow.com/a/9245829

  // indexing a string with &s[n] (located at mem addr x) retrieves the
  // substring starting at mem addr x+n, and ending at the null byte.
  if (sscanf(&buf[2], "%d;%d", &(scr->rows), &(scr->columns)) == -1) {
    panic("sscanf cursor position");
  };
} /* }}} */
