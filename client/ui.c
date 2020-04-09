
#include <string.h>
#include <ncurses.h>


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
	mvwprintw(top, htop/2, width/2 - 5, "H = %d, W = %d", htop, width);
	wrefresh(top);

	/* resize, move bot window up or down (relative to height of top window),
	 * and draw new border
	 */
	wresize(bot, hbot, width);
	mvwin(bot, htop + 3, 1);
	box(bot, 0, 0);
	mvwprintw(bot, hbot/2, width/2 - 5, "H = %d, W = %d", hbot, width);
	wrefresh(bot);

	refresh();
}


void init_windows(WINDOW *top, WINDOW *bot)
{
	/* receive all signals as characters,
	   no buffering input */
	raw();

	/* don't show uncooked input */
	noecho();

	refresh();

	/* get terminal window dimensions */
	int height, htop, hbot, width;
	getmaxyx(stdscr, height, width);
	htop = ((height / 3) * 2) - 2;
	hbot = (height / 3) - 2;
	width -= 2;

	/* create top sub window */
	box(top, 0, 0);
	mvwin(top, 1, 1);
	wresize(top, htop, width);
	mvwprintw(top, htop/2, (width/2) - 5, "H = %d, W = %d", htop, width);
	wrefresh(top);

	/* create bottom sub window */
	box(bot, 0, 0);
	mvwin(bot, htop + 3, 1);
	wresize(bot, hbot, width);
	mvwprintw(bot, hbot/2, (width/2) - 5, "H = %d, W = %d", htop, width);
	wrefresh(bot);
}


void wipe_window(WINDOW *win)
{
	int height, width;
	getmaxyx(win, height, width);

	for (int r = 1; r < height - 1; r++)
	{
		/* prepare blank line with null terminator */
		char blank_line[width - 1];
		memset(blank_line, ' ', width - 2);
		blank_line[width-2] = 0;

		/* print line to current row */
		mvwprintw(win, r, 1, blank_line);
	}
}


void new_line()
{
	static int nl = 0;
	wipe_window(top);
	mvwprintw(top, 1, 1, "new line %d", ++nl);
	wrefresh(top);
}


void submit_text(int len)
{
	if (len == 0) return;

	char text[len + 1];
	text[len] = 0;

	int height, width;
	getmaxyx(height, width);

	width -= 2;

	int read_chars = 0;
	while (read_chars < len)
	{
		/* don't read border chars */
		int row = (read_chars / width) + 1;
		int col = (read_chars % width) + 1;

		/* read next char */
		text[read_chars++] = A_CHARTEXT & mvwinch(bot, row, col);
	}
}


void write_buff_to_win(WINDOW* win, char* buff)
{
	int height, width, len, curx, cury;

	getmaxyx(win, height, width);
	height -= 2;
	width -= 2;

	curx =  

	len = strlen(buff);

}


int main()
{
	/* start ncurses */
	initscr();

	/* set up sub windows */
	WINDOW *top, bot;
	top = newwin(0, 0, 0, 0);
	bot = newwin(0, 0, 0, 0);
	init_windows(top, bot);

	int maxlen = 128;
	char text[maxlen+1];
	memset(text, 0, maxlen+1);

	int c, len = 0;
	while ((c = getch()) != SIGQUIT)
	{
		if (c == KEY_RESIZE)
		{
			terminal_resize(top, bot);
		}
		else if (c == '\n')
		{
			submit_text(len);
			wipe_window(bot);
			len = 0;
		}

		/*
		handle arrow keys, back space
		*/

		else if (len == maxlen) continue;
		else
		{

			int height, width, y, x;
			getmaxyx(bot, height, width);
			height -= 2;
			width -= 2;

			if (len = 0)
			{
				x = y = 1;
			}

			
			mvwaddch(bot, );
		}
	}
	endwin();
}




/*

maintain a char buffer with the contents of both windows.

write a procedure that writes the buffer to screen, and
when the window gets resized, just clear the screen and call
that procedure.

what about when

a. top screen has 1 message in it
b. we want to send a new message to it

 -  we cant just wipe the screen...
 - actually we can:

we will basically push the previous messages backward into the buffer,
or delete if it is pushed out of the bounds of the buffer.


*/



