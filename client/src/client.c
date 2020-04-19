
#include <pthreads.h>
#include <stdlib.h>
#include <signal.h>
#include <ncurses.h>

#include <rsa.h>
#include <ui.h>
#include <client.h>


struct message_buffer
{
	char *buffer;
	int len;
	int size;
	pthreads_mutex_t lock;
} msgbuff;


int client_loop(int sock, rsa_t privkey, rsa_t servkey)
{
	/* set up ncurses */
	WINDOW *top, *bot, *active;
	initscr();
	init_windows(&top, &bot);

	/* bottom window is active by default */
	active = bot;

	/* initialize message buffer struct */
	msgbuff.size = MAX_MSG_LEN;
	msgbuff.buffer = malloc(MAX_MSG_LEN + 1);
	msgbuff.buffer[MAX_MSG_LEN] = 0;
	msgbuff.len = 0;
	Pthread_mutex_init(&msgbuff.lock);

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
	rsa_clear(privkey);
	rsa_clear(servkey);

	return 0;
}


int incoming_message_handler(int sock, rsa_t privkey, WINDOW *win)
{
	int bytes;
	char tmpbuf[MAXMSGLEN];

	while (1)
	{
		/* await messages from server */
		bytes = receive_encrypted_message(sock, tmpbuf, MAXMSGLEN, privkey);

		Pthread_mutex_lock(&msgbuff.lock);

		/* check if buffer resize is necessary */
		if (bytes + msgbuff.len > MAXMSGLEN)
		{
			msgbuff.size *= 2;
			msgbuff.buffer = realloc(msgbuff.buffer, (msgbuff.size + 1));
		}

		/* append message onto msgbuff */
		memcpy(msgbuff.buffer + msgbuff.len, tmpbuf, bytes);
		msgbuff.len += bytes;
		msgbuff.buffer[msgbuff.len] = '\0';

		/* redraw msgbuff */
		display_from_line(win, msgbuff.line, msgbuff.buffer);
		Pthread_mutex_unlock(&msgbuff.lock);
	}
}


