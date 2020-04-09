
#include <string.h>
#include <signal.h>
#include <ncurses.h>


const char *text = "All Gaul is divided into three parts, one of which the Belgae inhabit, the Aquitani another, those who in their own language are called Celts, in our Gauls, the third. All these differ from each other in language, customs and laws. The river Garonne separates the Gauls from the Aquitani; the Marne and the Seine separate them from the Belgae.\nOf all these, the Belgae are the bravest, because they are furthest from the civilization and refinement of [our] Province, and merchants least frequently resort to them, and import those things which tend to effeminate the mind; and they are the nearest to the Germans, who dwell beyond the Rhine, with whom they are continually waging war; for which reason the Helvetii also surpass the rest of the Gauls in valor, as they contend with the Germans in almost daily battles, when they either repel them from their own territories, or themselves wage war on their frontiers.\nOne part of these, which it has been said that the Gauls occupy, takes its beginning at the river Rhone; it is bounded by the river Garonne, the ocean, and the territories of the Belgae; it borders, too, on the side of the Sequani and the Helvetii, upon the river Rhine, and stretches toward the north. The Belgae rises from the extreme frontier of Gaul, extend to the lower part of the river Rhine; and look toward the north and the rising sun. Aquitania extends from the river Garonne to the Pyrenaean mountains and to that part of the ocean which is near Spain: it looks between the setting of the sun, and the north star.\n";


void handle_resize(WINDOW *win)
{
  refresh();

  int h, w;
  getmaxyx(stdscr, h, w);

  wclear(win);
  wrefresh(win);

  wresize(win, h-2, w-2);
  box(win, 0, 0);

  refresh();
  wrefresh(win);
}


void display_from_line(WINDOW* win, unsigned line)
{
  /* get window dimensions */
  int h, w;
  getmaxyx(win, h, w);
  h -= 2;
  w -= 2;

  /* find line in massive text string */
  char* start = text;
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


int main()
{
  initscr();
  noecho();
  raw();
  keypad(stdscr, TRUE);

  refresh();

  int h, w;
  getmaxyx(stdscr, h, w);
  WINDOW *win = newwin(h-1, w-1, 1, 1);
  box(win, 0, 0);
  wrefresh(win);
  refresh();

  int c;
  unsigned line = 0;

  display_from_line(win, line);

  while ((c = getch()) != SIGQUIT)
  {
    refresh();

    switch(c)
    {
    case KEY_RESIZE:
      handle_resize(win);
      display_from_line(win, line);  
      break;

    case KEY_UP:
      line -= (line == 0 ? 0 : 1);
      display_from_line(win, line);
      break;

    case KEY_DOWN:
      line += 1;
      display_from_line(win, line);
      break;
    }
  }

  endwin();
}



