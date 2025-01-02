#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

// https://viewsourcecode.org/snaptoken/kilo

// https://www.mankier.com/3/termios
struct termios termios_mode;

// functions must be declared before they can be used

// error and exit. fallible stdlib functions always return an int (-1) on
// failure
void die(const char* msg) {
  // const is not required, but it indicates that the string is readonly

  // perror vs fprintf: https://stackoverflow.com/a/12102357
  // perror relies on global `errno` variable (defined in errno.h, but always
  // implicitly written to by POSIX syscalls)
  perror(msg);

  exit(EXIT_FAILURE);
}

void disable_raw_mode(void) {
  // if termios is not restored, user must manually call `reset` on exit

  // TCSAFLUSH ensures the rest of the (unread) chars are not sent back to the
  // parent process
  // https://www.gnu.org/software/libc/manual/html_node/Mode-Functions.html#index-TCSAFLUSH
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_mode) == -1) {
    die("tcsetattr");
  };
}

void enable_raw_mode(void) { /* {{{ */
  // fails if stdin is passed to the program, or if a file descriptor is passed
  // (./main <file)
  if (tcgetattr(STDIN_FILENO, &termios_mode) == -1) {
    die("tcgetattr");
  };
  atexit(disable_raw_mode);  // defer-ish

  struct termios raw = termios_mode;  // assignment without ptr = copy

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
    die("tcsetattr");
  };

} /* }}} */

int main(void) {
  enable_raw_mode();

  while (1) {
    char c = '\0';  // nulling c is presumably better for memory safety

    // read stdin, 1 char (byte) at a time (as configured in VMIN), every 0.1 s
    // (as configured in VTIME)
    if (read(STDIN_FILENO, &c, 1) == -1
        // we don't care about Cygwin
        // && errno != EAGAIN
    ) {
      die("read");
    };

    if (iscntrl(c)) {
      // escape sequences comprise 2 or more bytes
      // some bytes (e.g. 27 (Esc)) do not have a meaningful char representation
      printf("%d\r\n", c);
    } else {
      printf("%d (%c)\r\n", c, c);
    }

    if (c == 'q') {
      break;
    };
  };

  return EXIT_SUCCESS;
}
