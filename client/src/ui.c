
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include <ui.h>


void terminal_resize(WINDOW *top, WINDOW *bot)
{
  /* get new terminal dimensions */
  int height, htop, hbot, width;
  getmaxyx(stdscr, height, width);
  htop = ((height / 3) * 2) - 2;
  hbot = (height / 3) - 2;
  width -= 2;

  /* clear old borders */
  wclear(top);
  wrefresh(top);
  wclear(bot);
  wrefresh(bot);

  /* resize and draw new border */
  wresize(top, htop, width);
  box(top, 0, 0);
  wrefresh(top);

  /* resize, move bot window up or down
         * (relative to height of top window),
   * and draw new border
   */
  wresize(bot, hbot, width);
  mvwin(bot, htop + 3, 1);
  box(bot, 0, 0);
  wrefresh(bot);
}


void init_windows(WINDOW **top, WINDOW **bot)
{
  /* receive all signals as characters,
     no buffering input */
  raw();

  /* don't show uncooked input */
  noecho();

  /* refresh stdscr before sub windows */
  refresh();

  /* no new lines when enter key is pressed */
  nonl();

  /* get terminal window dimensions */
  int height, htop, hbot, width;
  getmaxyx(stdscr, height, width);
  htop = ((height / 3) * 2) - 2;
  hbot = (height / 3) - 2;
  width -= 2;

  /* create top sub window */
  *top = newwin(htop, width, 1, 1);
  box(*top, 0, 0);
  wrefresh(*top);

  /* create bottom sub window */
  *bot = newwin(hbot, width, htop + 3, 1);
  box(*bot, 0, 0);
  wmove(*bot, 1, 1);
  wrefresh(*bot);

  /* use keypad on all windows */
  keypad(stdscr, TRUE);
  keypad(*top, TRUE);
  keypad(*bot, TRUE);
}


void display_from_line(WINDOW* win, unsigned line, const char *text)
{
  /* get window dimensions */
  int h, w;
  getmaxyx(win, h, w);
  h -= 2;
  w -= 2;

  /* find line in massive text string */
  const char * start = text;
  int currline = 0;
  int chars = 0;

  while (*start)
  {
    /* have we reached target line? */
    if (currline == line)
    {
      break;
    }

    /* check for new line char */
    if (*start == '\n')
    {
      currline++;
      chars = 0;
    }

    /* otherwise increment char counter */
    else
    {
      chars++;

      /* check if we have filled the current line */
      if (chars == w)
      {
        chars = 0;
        currline++;
      }
    }

    /* next char */
    start++;
  }

  char line_buff[w+1];
  line_buff[w] = 0;

  for (int i = 1; i <= h; i++)
  {
    /* clear line */
    memset(line_buff, ' ', w);

    /* fill in chars for this line */
    int charnum = 0;
    while (*start && charnum < w)
    {
      if (*start == '\n')
      {
        start++;
        break;
      }

      line_buff[charnum++] = *(start++);
    }

    mvwprintw(win, i, 1, line_buff);
  }

  wrefresh(win);
}


int move_cursor(WINDOW *win, int len, int index, int key)
{
  int h, w;
  getmaxyx(win, h, w);
  h -= 2;
  w -= 2;

  switch (key)
  {
  case KEY_LEFT:
    index--;
    break;
  case KEY_RIGHT:
    index++;
    break;
  case KEY_UP:
    index -= w;
    break;
  case KEY_DOWN:
    index += w;
    break;
  }

  if (index > len) index = len;
  else if (index < 0) index = 0;

  int y = index / w;
  int x = index % w;

  wmove(win, y+1, x+1);

  wrefresh(win);

  return index;
}

