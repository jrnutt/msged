/*
 *
 * menu
 *
 * moving bar menu code for msged
 *
 * implementation
 *
 */

#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include "menu.h"
#include "screen.h"

#ifdef __MSC__
#define strncmpi strnicmp
#define strcmpi stricmp
#endif

#if defined(__ZTC__)
int _pascal strncmpi(char *, char *, int);
#endif

/* necessary imports from screen */

extern  int maxy,             /* how many screen lines? */
	    maxx,             /* how many screen columns? */
	    videomethod;      /* DIRECT, BIOS or FOSSIL */

int _pascal menu(int x1, int y1, int x2, int y2, char **items, int bar, int normal, int def)

{
	int 	count = 0,
		pos = 0,
		y = y1,
		key = 0,
		i;

	char	*t = NULL,
		*p = NULL,
		**l = items;

	x1 = max(1,x1);
	x2 = min(maxx,x2);
	y1 = max(1,y1);
	y2 = min(maxy,y2);

	t = malloc(x2 - x1 + 2);
	memset(t,0,x2 - x1 + 2);

	set_color(normal);

	clrwnd(x1,y1,x2,y2);

	while (*l != NULL) {
		count++;
		l++;
	}

	p = items[pos = def];
	y = y1;
	do {
		set_color(normal);
		gotoxy(x1,y++);
		bputs(items[pos++]);
		pos %= count;
	} while ((items[pos] != p) && (y <= y2));

	gotoxy(x1,y = y1);
	pos = def;
	video_update();
	set_color(bar);
	p = t;
	bputs(items[pos]);

	while ((key = getkey()) != ABORT) {
		set_color(normal);
		gotoxy(x1,y);
		bputs(items[pos]);
	
		switch (key) {
			case GOBOL:
				clrwnd(x1,y1,x2,y2);
				pos = 0;
				p = items[pos];
				y = y1;
				do {
					set_color(normal);
					gotoxy(x1,y++);
					bputs(items[pos++]);
					pos %= count;
				} while ((items[pos] != p) && (y <= y2));
				y = y1;
				pos = 0;
				memset(t,0,x2 - x1);
				p = t;
				break;

			case DOWN:
				pos++;
				pos %= count;
				if ((y == y2) || (y == (count + y1 - 1)))
					scrollup(x1,y1,x2,y2,1);
				else
					y++;
				break;
				
			case UP:
				pos = (pos==0)?count-1:pos-1;
				if (y == y1)
					scrolldown(x1,y1,x2,min(y2,y1+count-1),1);
				else
					y--;
				break;

			case PGUP:
				i = min(count-1,y2);
				while (i--) {
					pos = (pos==0)?count-1:pos-1;
					if (y == y1) {
						scrolldown(x1,y1,x2,min(y2,y1+count-1),1);
						gotoxy(x1,y1);
						set_color(normal);
						bputs(items[pos]);
					}
					else
						y--;
					};
				break;

			case PGDN:
				i = min(count-1,y2);
				while (i--) {
					pos++;
					pos %= count;
					if ((y == y2) || (y == (count + y1 - 1))) {
						scrollup(x1,y1,x2,y2,1);
						gotoxy(x1,min(y2,y1+count-1));
						set_color(normal);
						bputs(items[pos]);
					}
					else
						y++;
				}
				break;

			case ENTER:
				set_color(normal);
				return pos;

			default:
				if ((key & 0xff) && ((p-t) < (x2-x1))) {
					*p = (char) (key & 0xff);
					p++;
					i = count;
					while (i--) {
						if (strncmpi(t,items[pos],strlen(t)) == 0)
							break;
						pos++;
						pos %= count;
						if ((y == y2) || (y == (count + y1 - 1))) {
							scrollup(x1,y1,x2,y2,1);
							gotoxy(x1,min(y2,y1+count-1));
							set_color(normal);
							bputs(items[pos]);
						}
						else
							y++;
					}
				}

				if ((i<0) || !(key & 0xff)) {
					memset(t,0,x2-x1);
					p = t;
				}
				break;
		}
	
		set_color(bar);
		gotoxy(x1,y);
		video_update();
		bputs(items[pos]);
	}

	set_color(normal);
	free(t);
	return(-1);
}

void _pascal box(int x1, int y1, int x2, int y2, int t) 

{
	int x, y;

	for (x = x1 + 1; x < x2; x++) {
		gotoxy(x,y1); bputc(t?196:205);
		gotoxy(x,y2); bputc(t?196:205);
	}

	for (y = y1 + 1; y < y2; y++) {
		gotoxy(x1,y); bputc(t?179:186);
		gotoxy(x2,y); bputc(t?179:186);
	}
	gotoxy(x1,y1); bputc(t?218:201);
	gotoxy(x1,y2); bputc(t?192:200);
	gotoxy(x2,y1); bputc(t?191:187);
	gotoxy(x2,y2); bputc(t?217:188);
}
