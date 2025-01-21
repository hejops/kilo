/* key.c */

// switch/case only accepts compile-time constants, so macro is required
// char mod_ctrl(char c) { return c & 0x1f; }
//
// 0x1f = 0b0001_1111
// this clears the upper 3 bits; the uppermost (7th, 0-indexed) bit is
// actually irrelevant (?) in ascii, which only uses 7 bits.
// https://old.reddit.com/r/learnprogramming/comments/6vyoxj/_/dm400ar/
#define mod_ctrl(k) (k & 0x1f)

#define KEY_QUIT mod_ctrl('x')

int read_key(void);
void handle_key(int key);
