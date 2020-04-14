
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ncurses.h>

#define MAXMSGLEN (256)

#define CTRL(x) ((x) & (0x1f))


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


int main()
{

	remove("log.txt");
	FILE *f = fopen("log.txt", "a");

	/* start ncurses */
	initscr();

	/* set up sub windows */
	WINDOW *top, *bot, *active;
	init_windows(&top, &bot);

	/* active window */
	active = bot;

	/* buffer for receiving messages */
	int rec_buff_size = MAXMSGLEN;
	char *rec_buff = calloc(1, MAXMSGLEN + 1);
	rec_buff[MAXMSGLEN] = 0;
	int rec_buff_len = 0;

	/* buffer for sending messages */
	char snd_buff[MAXMSGLEN + 1];
	snd_buff[MAXMSGLEN] = 0;
	int snd_buff_len = 0;

	/* keep track of current scroll line for top window */
	int currline = 0;

	/* keep track of cursor location in bot window */
	int cursor_index = 0;

	int c;
	while ((c = wgetch(active)) != SIGQUIT)
	{
		fprintf(f, "%d = %c\n", c, (char)c);
		fflush(f);
		/* refresh stdcr before sub windows */
		refresh();

		/* Ctrl+W switch active window */
		if (c == CTRL('w'))
		{
			active = active == top ? bot : top;
		}

		/* check for resize signal */
		else if (c == KEY_RESIZE)
		{
			terminal_resize(top, bot);
			display_from_line(top, currline, rec_buff);
			display_from_line(bot, 0, snd_buff);
		}

		/* process top window inputs */
		else if (active == top)
		{
			switch(c)
			{
			case KEY_UP:
				currline -= currline == 0 ? 0 : 1;
				display_from_line(top, currline, rec_buff);
				break;

			case KEY_DOWN:
				currline++;
				display_from_line(top, currline, rec_buff);
				break;
			}
		}

		/* we are using bottom windown */
		else if (c >= KEY_DOWN && c <= KEY_RIGHT)
		{
			cursor_index = move_cursor(bot,
						   snd_buff_len,
						   cursor_index,
						   c);
		}

		else if (c == KEY_ENTER || c == '\r')
		{
			/* return cursor to starting pos */
			cursor_index = 0;
			wmove(bot, 1, 1);

			/* check if rec_buff needs to be resized */
			if (rec_buff_len + snd_buff_len + 3 > rec_buff_size)
			{
				rec_buff_size *= 2;
				rec_buff = realloc(rec_buff, rec_buff_size);
			}

			/* append snd_buff to rec_buff */
			memcpy(rec_buff+rec_buff_len, snd_buff, snd_buff_len+1);
			rec_buff_len += snd_buff_len;

			/* append new line chars */
			rec_buff[rec_buff_len++] = '\n';
			rec_buff[rec_buff_len++] = '\n';

			/* clear snd_buff */
			snd_buff_len = 0;
			memset(snd_buff, 0, MAXMSGLEN);

			/* redraw both windows */
			display_from_line(top, currline, rec_buff);
			display_from_line(bot, 0, snd_buff);
		}

		else if (c == KEY_BACKSPACE || c == 127)
		{
			if (cursor_index == 0) continue;

			/* shift chars back one */
			memmove((snd_buff + cursor_index) - 1,
			       (snd_buff + cursor_index),
			       (snd_buff_len - cursor_index));

			/* set last char to NULL */
			snd_buff[--snd_buff_len] = '\0';

			/* redraw window */
			display_from_line(bot, 0, snd_buff);

			/* move cursor back one */
			cursor_index = move_cursor(bot,
						   snd_buff_len,
						   cursor_index,
						   KEY_LEFT);
		}

		else if (snd_buff_len < MAXMSGLEN)
		{
			snd_buff_len++;

			/* shift chars ahead one */
			memmove((snd_buff + cursor_index + 1),
			       (snd_buff + cursor_index),
			       (snd_buff_len - cursor_index));

			/* put char */
			c = (char) c;
			snd_buff[cursor_index] = c;

			/* redraw window */
			display_from_line(bot, 0, snd_buff);

			/* move cursor ahead one */
			cursor_index = move_cursor(bot,
						   snd_buff_len,
						   cursor_index,
						   KEY_RIGHT);
		}
	}
	endwin();
	free(rec_buff);
	fclose(f);
}

