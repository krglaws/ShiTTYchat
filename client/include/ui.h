
#ifndef _UI_H_
#define _UI_H_

#define CTRL(x) ((x) & (0x1f))

void terminal_resize(WINDOW *top, WINDOW *bot);

void init_windows(WINDOW **top, WINDOW **bot);

void display_from_line(WINDOW *win, unsigned line, const char *text);

int move_cursor(WINDOW *win, int len,  int index, int key);

#endif

