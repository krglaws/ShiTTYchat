
#include <ncurses.h>


static WINDOW *top = NULL;
static WINDOW *bot = NULL;


void init_ui()
{
  /* enter curses mode */
  initscr();
  //raw();
  refresh();

  /* calculate screen size */
  int height, width;
  getmaxyx(stdscr, height, width);
  int htop = ((height / 3) * 2) - 2;
  int hbot = (height / 3) - 2;
  width -= 2;

  /* create top window */
  top = newwin(htop, width, 1, 1);
  box(top, 0, 0);
  wrefresh(top);

  /* create bottom window */
  bot = newwin(hbot, width, htop + 2, 1);
  box(bot, 0, 0);
  wrefresh(bot);
}


void window_resize()
{
  /* For some reason, things work better if you refresh 
   * stdscr BEFORE making these changes.
   */
  refresh(); 

  /* calculate new screen size */
  int height, width;
  getmaxyx(stdscr, height, width);
  int htop = ((height / 3) * 2) - 2;
  int hbot = (height / 3) - 2;
  width -= 2;

  /* clear old top window border */
  wborder(top, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(top);

  /* resize and draw new border */
  wresize(top, htop, width);
  box(top, 0, 0);
  wrefresh(top);

  /* clear old bot window border */
  wborder(bot, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(bot);

  /* resize, move bot window up or down (relative to height of top window),
   * and draw new border
   */
  wresize(bot, hbot, width);
  mvwin(bot, htop + 2, 1);
  box(bot, 0, 0);
  wrefresh(bot);
}


int main()
{
  init_ui();

  char msg[64];

  while (1)
  {
    refresh();

    wgetstr(bot, msg);
    wrefresh(bot);

    wprintw(top, msg);
    wrefresh(top);
  }

  endwin();
}


