
#include <ncurses.h>


int main()
{
  int height, width;
  WINDOW *top, *bot;

  initscr();
  raw();

  getmaxyx(stdscr, height, width);

  top = newwin((height/2)-2, width-2, 1, 1);
  box(top, 0, 0);
  //wborder(top, ' ', '|', '-', '-', '.', '.', '.', '.');

  bot = newwin((height/2)-2, width-2, (height/2)+1, 1);
  box(bot, 0, 0);
  //wborder(bot, '|', '|', '-', '-', '.', '.', '.', '.');

  wrefresh(top);
  wrefresh(bot);
  //refresh();

  int ch;
  do
  {
    ch = getch();

    if (ch == KEY_RESIZE)
    {
      getmaxyx(stdscr, height, width);

      // clear old borders
      wborder(top, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
      wborder(bot, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');

      // resize windows
      wresize(top, (height/2)-2, width-2);
      wresize(bot, (height/2)-2, width-2);

      // adjust bottom window position
      mvwin(bot, (height/2)+1, 1);

      // redraw borders
      box(top, 0, 0);
      box(bot, 0, 0);

      wrefresh(top);
      wrefresh(bot);
      //refresh();
    }

  } while(ch != 'Q');

  endwin();
}


