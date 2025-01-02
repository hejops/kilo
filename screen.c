#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct EDITOR {                 // struct declaration and initialisation
  struct termios termios_mode;  // https://www.mankier.com/3/termios
  int rows;
  int columns;
} EDITOR;

void draw_rows(void) {
  for (int row = 0; row < EDITOR.rows; row++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

void clear_screen(void) {
  // \x1b = 27 = Esc

  // [2J = erase all lines
  // https://vt100.net/docs/vt100-ug/chapter3.html#ED
  // if n < len(string), probably UB
  write(STDOUT_FILENO, "\x1b[2J", 4);

  // [row;colH = place cursor at row,col
  // with no args, row=col=1 (1-indexed)
  // https://vt100.net/docs/vt100-ug/chapter3.html#CUP
  write(STDOUT_FILENO, "\x1b[H", 3);

  draw_rows();

  write(STDOUT_FILENO, "\x1b[H", 3);  // move cursor back to home
}

// clear the screen, raise error, and exit. fallible stdlib functions always
// return an int (-1) on failure
void panic(const char* msg) {
  // const is not required, but it indicates that the string is readonly

  clear_screen();

  // perror vs fprintf: https://stackoverflow.com/a/12102357
  // perror relies on global `errno` variable (defined in errno.h, but always
  // implicitly written to by POSIX syscalls)
  perror(msg);

  exit(EXIT_FAILURE);
}

void disable_raw_mode(void) { /* {{{ */
  // if termios is not restored, user must manually call `reset` on exit

  // TCSAFLUSH ensures the rest of the (unread) chars are not sent back to the
  // parent process
  // https://www.gnu.org/software/libc/manual/html_node/Mode-Functions.html#index-TCSAFLUSH
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &EDITOR.termios_mode) == -1) {
    panic("tcsetattr");
  };
} /* }}} */

void enable_raw_mode(struct EDITOR* e) { /* {{{ */
  // fails if stdin is passed to the program, or if a file descriptor is passed
  // (./main <file)
  if (tcgetattr(STDIN_FILENO, &e->termios_mode) == -1) {
    panic("tcgetattr");
  };
  atexit(disable_raw_mode);  // defer-ish

  struct termios raw = e->termios_mode;  // assignment without ptr = copy

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

void set_window_dimensions(struct EDITOR* e) {
  // struct winsize ws;
  // if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1 && ws.ws_col > 0) {
  //   e->rows = ws.ws_row;
  //   e->columns = ws.ws_col;
  //   return;
  // }

  // fallback: determine cursor position manually

  // unlike H, C+B ensures cursor remains within the bounds
  if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
    panic("write");
  };

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
    panic("write");
  };

  char buf[32];
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
  if (sscanf(&buf[2], "%d;%d", &(e->rows), &(e->columns)) == -1) {
    panic("sscanf");
  };
}
