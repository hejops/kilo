#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// clear the screen, raise error, and exit. fallible stdlib functions always
// return an int (-1) on failure
void panic(const char* msg) {
  // const is not required, but it indicates that the string is readonly

  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  // perror vs fprintf: https://stackoverflow.com/a/12102357
  // perror relies on global `errno` variable (defined in errno.h, but always
  // implicitly written to by POSIX syscalls)
  perror(msg);

  exit(EXIT_FAILURE);
}
