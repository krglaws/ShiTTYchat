
#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <signal.h>
#include <ncurses.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <rsa.h>
#include <comm.h>
#include <ui.h>
#include <client.h>

#include <settings.h>


char *rec_buff;
int rec_buff_len;
int rec_buff_size;
int line;
int sock;
WINDOW *win;
rsa_key_t key;
pthread_mutex_t lock;


int client_loop(int sock_fd, rsa_key_t privkey, rsa_key_t servkey)
{
  /* set up ncurses */
  WINDOW *top, *bot, *active;
  initscr();
  init_windows(&top, &bot);
  active = bot;

  /* initialize shared data */
  rec_buff_size = MAXMSGLEN;
  rec_buff = malloc(MAXMSGLEN + 1);
  rec_buff[MAXMSGLEN] = 0;
  rec_buff_len = 0;
  line = 0;
  sock = sock_fd;
  win = top;

  key->d = privkey->d;
  key->e = privkey->e;
  key->b = privkey->b;

  int err;
  if (err = pthread_mutex_init(&lock, NULL))
  {
    errno = err;
    perror("init_shared_data(): failed on call to pthread_mutex_init()");
    return -1;
  }

  /* variables for sending messages */
  char snd_buff[MAXMSGLEN+1];
  int snd_buff_len = 0;
  int cursor_index = 0;
  memset(snd_buff, 0, MAXMSGLEN + 0);
 
  /* set up SIGCHLD handler */
  if (signal(SIGCHLD, &sigchld_handler) == SIG_ERR)
  {
    perror("client_loop(): failed to set SIGCHLD handler");
    return -1;
  }

  /* set up incoming message handler handler */
  int stack_size = 1024*1024;
  void* stack = malloc(stack_size);
  int pid = clone(&incoming_message_handler, stack + stack_size, 
                  CLONE_FILES | CLONE_VM | SIGCHLD, NULL);

  int c;
  while ((c = wgetch(active)) != SIGQUIT)
  {
    /* refresh stdcr before sub windows */
    pthread_mutex_lock(&lock);
    refresh();
    pthread_mutex_unlock(&lock);

    /* Ctrl+W switch active window */
    if (c == CTRL('w'))
    {
      active = active == top ? bot : top;
    }

    /* check for resize signal */
    else if (c == KEY_RESIZE)
    {
      pthread_mutex_lock(&lock);

      terminal_resize(top, bot);
      display_from_line(top, line, rec_buff);
      display_from_line(bot, 0, snd_buff);

      pthread_mutex_unlock(&lock);
    }

    /* process top window inputs */
    else if (active == top)
    {
      pthread_mutex_lock(&lock);

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

      pthread_mutex_unlock(&lock);
    }

    /* we are using bottom window */
    else if (c >= KEY_DOWN && c <= KEY_RIGHT)
    {
      pthread_mutex_lock(&lock);

      cursor_index = move_cursor(bot,
               snd_buff_len,
               cursor_index,
               c);

      pthread_mutex_unlock(&lock);
    }

    else if (c == KEY_ENTER || c == '\r')
    {
      pthread_mutex_lock(&lock);

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

      /* redraw bot window */
      display_from_line(bot, 0, snd_buff);

      pthread_mutex_unlock(&lock);
    }

    else if (c == KEY_BACKSPACE || c == 127)
    {
      pthread_mutex_lock(&lock);

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

      pthread_mutex_unlock(&lock);
    }

    else if (snd_buff_len < MAXMSGLEN)
    {
      pthread_mutex_lock(&lock);

      snd_buff_len++;

      /* shift chars ahead one */
      memmove((snd_buff + cursor_index + 1),
             (snd_buff + cursor_index),
             (snd_buff_len - cursor_index));

      /* put char */
      c = (char) c;
      snd_buff[cursor_index] = c;

      /* redraw bottom window */
      display_from_line(bot, 0, snd_buff);

      /* move cursor ahead one */
      cursor_index = move_cursor(bot,
               snd_buff_len,
               cursor_index,
               KEY_RIGHT);

      pthread_mutex_unlock(&lock);
    }
  }

  /* send disconnection message */
  char *dis = "DISCONNECT\n";
  send_encrypted_message(sock, dis, strlen(dis), servkey);

  /* kill listener thread */
  signal(SIGCHLD, SIG_IGN);
  kill(pid, SIGKILL);

  /* cleanup and quit */
  endwin();
  free(rec_buff);
  rsa_clear_key(privkey);
  rsa_clear_key(servkey);
  close(sock);

  return 0;
}


int incoming_message_handler(void *args)
{
  /* prevent listener thread from crashing on window resize */
  signal(SIGWINCH, SIG_IGN);

  /* allocate space for time + name + msg */
  int bytes;
  int bufflen = MAXMSGLEN + UNAMELEN + 32;
  char tmp_buff[bufflen];

  while (1)
  {
    /* await messages from server */
    bytes = receive_encrypted_message(sock, tmp_buff, bufflen, key);

    if (bytes == -1)
    {
      fprintf(stderr, "incoming_message_handler(): failed on call to receive_encrypted_message()\n");
      return -1;
    }

    /* lock shared data */
    pthread_mutex_lock(&lock);

    /* check if buffer resize is necessary */
    if (bytes + rec_buff_len + 1 > bufflen)
    {
      rec_buff_size *= 2;
      rec_buff = realloc(rec_buff, (rec_buff_size + 1));
    }

    /* append message onto rec_buff */
    memcpy(rec_buff + rec_buff_len, tmp_buff, bytes);
    rec_buff_len += bytes;
    rec_buff[rec_buff_len++] = '\n';
    rec_buff[rec_buff_len] = '\0';

    /* redraw top window */
    display_from_line(win, line, rec_buff);

    /* unlock shared data */
    pthread_mutex_unlock(&lock);
  }

  return 0;
}


void sigchld_handler(int signum)
{
  _exit(-1);
}

