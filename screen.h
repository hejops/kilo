/* screen.c */
#include "buffer.h"           // re-export type
extern struct SCREEN SCREEN;  // re-export global var
void draw_rows(int n, struct STRING_BUFFER *buf);
void clear_screen(struct SCREEN *scr);
void disable_raw_mode(void);
void enable_raw_mode(struct SCREEN *scr);
void set_dimensions(struct SCREEN *scr);
