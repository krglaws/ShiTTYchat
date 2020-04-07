
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ncurses.h>


void window_resize();


static WINDOW *top = NULL;
static WINDOW *bot = NULL;
int count = 0;

void init_ui()
{
  /* enter curses mode */
  initscr();
  //raw();
  refresh();

  /* calculate screen size */
  int height, width;
  getmaxyx(stdscr, height, width);
  count++;
  FILE *f = fopen("log.txt", "a");
  fprintf(f, "%d: height = %d, width = %d\n", count, height, width);
  fclose(f); 

  int htop = ((height / 3) * 2) - 2;
  int hbot = (height / 3) - 2;
  width = width - 2;

  /* create top window */
  top = newwin(htop, width, 1, 1);
  box(top, 0, 0);
  mvwprintw(top, htop/2, width/2, "H = %d, W = %d", htop, width);
  wrefresh(top);

  /* create bottom window */
  bot = newwin(hbot, width, htop + 2, 1);
  box(bot, 0, 0);
  mvwprintw(bot, hbot/2, width/2, "H = %d, W = %d", hbot, width);
  wrefresh(bot);

  /* set screen resize signal handler */
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = window_resize;
  sigaction(SIGWINCH, &sa, NULL);
}


void window_resize()
{
  /* For some reason, things work better if you refresh 
   * stdscr BEFORE making these changes.
   */
  refresh();
  wrefresh(top);
  wrefresh(bot);

  /* calculate new screen size */
  int height, width;
  getmaxyx(stdscr, height, width);

  count++;
  FILE *f = fopen("log.txt", "a");
  fprintf(f, "%d: height = %d, width = %d\n", count, height, width);
  fclose(f);

  int htop = ((height / 3) * 2) - 2;
  int hbot = (height / 3) - 2;
  width = width - 2;

  /* clear old top window border */
  wborder(top, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(top);

  /* resize and draw new border */
  wresize(top, htop, width);
  box(top, 0, 0);
  mvwprintw(top, htop/2, width/2, "H = %d, W = %d", htop, width);
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
  mvwprintw(bot, hbot/2, width/2, "H = %d, W = %d", hbot, width);
  wrefresh(bot);
}


int main()
{
  remove("log.txt");

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


