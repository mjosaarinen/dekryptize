// dekryptize.c
// 2016-12-19  Markku-Juhani O. Saarinen <mjos@iki.fi>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>
#include <locale.h>
#include <ctype.h>
#include <time.h>

#define MAXRNDSET 200

int rndset = 200;
int tablen = 8;
char *buf = NULL, *enc = NULL;
int *per = NULL;
int rows, cols, nchr;

int randper()
{
	int i, j, o;

	o = rand() % nchr;
	
	for (i = 0; i < nchr;) {
	
		switch (rand() % 9) {
			case 0: {
				o = (o + 1) % nchr;
				break;
			}
			case 1: {
				o = (o + nchr - 1) % nchr;
				break;
			}
			case 2: {
				o = (o + cols) % nchr;
				break;
			}
			case 3: {
				o = (o + nchr - cols) % nchr;
				break;
			}
			case 4: {
				o = (o + cols + 1) % nchr;
				break;
			}
			case 5: {
				o = (o + cols - 1) % nchr;
				break;
			}
			case 6: {
				o = (o + nchr - cols + 1) % nchr;
				break;
			}
			case 7: {
				o = (o + nchr - cols - 1) % nchr;
				break;
			}
			case 8: {
				o = rand() % nchr;
				break;
			}
		}
		for (j = 0; j < i; j++) {
			if (per[j] == o)
				break;
		}
		if (j >= i) {
			per[i++] = o;
		}
	}
	
	return 0;
}

int main(int argc, char **argv)
{
	int ch, i, x, y, o, n, b, done;
	int set[MAXRNDSET];
	
	FILE *f = stdin;
	int status = 0;
	char lin[256];

	if (argc >= 2) {
		if ((f = fopen(argv[1], "r")) == NULL) {
			perror(argv[1]);
			return -1;
		}
	}

	srand(time(NULL));

	setlocale(LC_ALL, "");

	if (initscr() == NULL)
		return 1;
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);

	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);

	// check that it is sane
	rows = LINES - 1;
	cols = COLS;
	if (rows < 8 || cols < 22)
		return 1;
	nchr = rows * cols;

	// read the file
	if ((buf = malloc(nchr)) == NULL ||
		(enc = malloc(nchr)) == NULL ||
		(per = calloc(nchr, sizeof(int))) == NULL) {
		exit(-1);
	}
	memset(buf, ' ', nchr);

	for (y = 0; y < rows; y++) {
		if (fgets(lin, sizeof(lin), f) == NULL)
			break;
		x = 0;
		for (i = 0; i < sizeof(lin) && x < cols; i++) {
			ch = lin[i];
			if (ch == 0 || ch == '\n' || ch == '\r')
				break;
			if (ch == '\t') {
				x += tablen - (x % tablen);
				continue;
			}
			if (ch < 32 || ch > 127)
				ch = '*';
			buf[y * cols + x] = ch;
			x++;
		}
	}

	if (f != stdin)
		fclose(f);

	// randomize initial state

	for (i = 0; i < 6; i++) {

		for (y = 0; y < rows; y++) {
			for (x = 0; x < cols; x++) {
				ch = (rand() % 94) + 33;
				enc[y * cols + x] = ch;
				mvaddch(y, x, ch);
			}
		}
		refresh();
	
		if (i < 5) {

			if ((i & 1) == 0)	
				standout();

			mvprintw(rows / 2 - 1, cols / 2 - 6, "            ");
			mvprintw(rows / 2, cols / 2 - 6, " [ANALYSIS] ");
			mvprintw(rows / 2 + 1, cols / 2 - 6, "            ");
			if ((i & 1) == 0)	
				standend();

			refresh();
			usleep(400000);
		}
	}
		
	randper();

	// initial set is random

	n = nchr;
	for (i = 0; i < rndset; i++) {
		set[i] = per[--n];
	}
	
	done = 0;
	b = 0;
	while (getch() == ERR && done < rndset) {
	
		// animate the set
	
		for (i = 0; i < rndset; i++) {
			o = set[i];
			if (o < 0)
				continue;

			x = o % cols;
			y = o / cols;
			
			ch = enc[o];
			if (buf[o] == ' ') {	// reveal space right away
				ch = ' ';
			} else {
				if (ch != buf[o])
					ch = ((ch - 32 + 94) % 95) + 32;
			}
			mvaddch(y, x, ch);
			enc[o] = ch;

			// get a new thing in the set
			if (buf[o] == enc[o]) {
				if (n <= 0) {
					set[i] = -1;
					done++;
				} else {
					set[i] = per[--n];
				}
			}
		}

		doupdate();
		usleep(40000);
		
		if (b < 10) {
			standout();
		}
			
		mvprintw(rows, cols / 2 - 9, " [DECRYPTING %02d%%] ",
			100 * (nchr - n + done) / (nchr + rndset));
			
		if (b < 10) {
			standend();
		}

		refresh();
		
		b++;
		if (b == 20)
			b = 0;
	}

	standout();
	mvprintw(rows, cols / 2 - 11, " [MESSAGE DECRYPTED] ");
	standend();
	refresh();

	nodelay(stdscr, FALSE);

	getch();

	// cleanup
	endwin();
	if (buf != NULL)
		free(buf);
	if (enc != NULL)
		free(enc);
	if (per != NULL)
		free(per);

	return status;
}
