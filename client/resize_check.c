
#include <signal.h>
#include <ncurses.h>


void resize_handler(int sig)
{
	clear();

	int height, width;
	getmaxyx(stdscr, height, width);
	mvprintw(height/2, (width/2)-5, "H = %d, W = %d", height, width);

	refresh();
}


int main()
{
	signal(SIGWINCH, resize_handler);

	initscr();
	noecho();
	refresh();

	int c;
	while ((c = getch()) != 'Q');

	endwin();
}

