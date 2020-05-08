#include <stdlib.h>
#include <signal.h>
#include <ncurses.h>
#include <pthreads.h>

#include <rsa.h>
#include <comm.h>
#include <ui.h>
#include <client.h>


static char *rec_buff;
static int len;
static int size;
static int line;
static int sock;
static pthreads_mutex_t lock;


int client_loop(int sock_fd, rsa_t privkey, rsa_t servkey)
{
	/* set up ncurses */
	WINDOW *top, *bot, *active;
	initscr();
	init_windows(&top, &bot);
	active = bot;

	if (init_shared_data() == -1)
	{
		fprintf(stderr, "client_loop(): failed on call to init_shared_data()\n");
		return -1;
	}

	/* buffer for sending messages */
	char snd_buff[MAXMSGLEN + 1];
	snd_buff[MAXMSGLEN] = 0;
	int snd_buff_len = 0;
	int cursor_index = 0;
 
	/* set up SIGCHLD handler */
	if (signal(SIGCHLD, sigchld_handler) == -1)
	{
		perror("client_loop(): failed to set SIGCHLD handler");
		return -1;
	}

	/* set up incoming message handler handler */
	void* stack = malloc(1024*1024);
	int pid = clone(incoming_message_handler, stack, 0, CLONE_FILES, NULL);

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
			display_from_line(top, line, rec_buff);
			display_from_line(bot, 0, snd_buff);
		}

		/* process top window inputs */
		else if (active == top)
		{
			switch(c)
			{
			case KEY_UP:
				line -= line == 0 ? 0 : 1;
				display_from_line(top, line, rec_buff);
				break;

			case KEY_DOWN:
				line++;
				display_from_line(top, line, rec_buff);
				break;
			}
		}

		/* we are using bottom window */
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

			/* send encrypted message */
			if (send_encrypted_message(sock, snd_buff, snd_buff_len, servkey) == -1)
			{
				fprintf(stderr, "client_loop(): failed on call to send_encrypted_message()\n");
				break;
			}

			/* clear snd_buff */
			snd_buff_len = 0;
			memset(snd_buff, 0, MAXMSGLEN);

			/* redraw both windows */
			display_from_line(top, line, rec_buff);
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

	/* kill child */
	kill(pid, SIGKILL);

	endwin();
	free(rec_buff);
	rsa_clear(privkey);
	rsa_clear(servkey);
	close(sock);

	return 0;
}


int init_shared_data()
{
	/* initialize shared data */
	size = MAXMSGLEN;
	buffer = malloc(MAXMSGLEN + 1);
	buffer[MAXMSGLEN] = 0;
	len = 0;
	line = 0;
	sock = sock_fd;

	if (int err = pthread_mutex_init(&lock))
	{
		errno = err;
		perror("init_shared_data()");
		return -1;
	}

	return 0;
}


int incoming_message_handler(int sock, rsa_t privkey, WINDOW *win)
{
	int bytes;
	char tmp_buff[MAXMSGLEN];

	while (1)
	{
		/* await messages from server */
		bytes = receive_encrypted_message(sock, tmp_buff, MAXMSGLEN, privkey);

		/* lock shared data */
		pthread_mutex_lock(&lock);

		/* check if buffer resize is necessary */
		if (bytes + len + 1 > MAXMSGLEN)
		{
			size *= 2;
			buffer = realloc(buffer, (size + 1));
		}

		/* append message onto rec_buff */
		memcpy(buffer + len, tmp_buff, bytes);
		len += bytes;
		buffer[len++] = '\n';
		buffer[len] = '\0';

		/* redraw top window */
		display_from_line(win, line, buffer);

		/* unlock shared data */
		pthread_mutex_unlock(&lock);
	}
}


int sigchld_handler()
{
  exit();
}

