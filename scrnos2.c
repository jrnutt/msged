/*

Title:	MsgEd

File:	Scrnos2.c

Author: Jim Nutt

Copr:	1987 by Jim Nutt

Description:

	Screen handling functions for MsgEd.  also a few string handlers.

	if you need to port this... remember, msged uses a 1 based coordinate
	system (i.e. top left is (1,1) NOT (0,0)).	anything that is dependent
	on zortech c, quick c or turbo c will complain when you try to compile
	this thing under any other compiler...

*/

#define USEFOSSIL 1

#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#ifndef MK_FP
#include "mkfp.h"
#endif
#include "screen.h"
#include "vfossil2.h"
#include "screen2.h"

static void pascal vfossil_cursor (int st);
static void pascal vfossil_init (void);
static void pascal vfossil_open (void);
static void pascal vfossil_close (void);

void pascal strins(char *l, char c, int x);
void pascal strdel(char *l, int x);

#define FALSE 0
#define TRUE !FALSE
#define TEXTLEN 256

unsigned int *macros[41];	 /* function key macros + 1 for autostart */

int maxx;				/* maximum screen columns */
int maxy;				/* maximum screen rows */
int videomethod;		/* how to write to the screen */
unsigned int vbase; 	/* where screen memory is located */
unsigned char current_color;	 /* current screen attribute */

/* keyboard control values */

static int autostart = 0;
static unsigned int * macro = (void *) NULL;
static unsigned int keybuf = 0;
static unsigned int pascal lgetkey(void);

static char s[TEXTLEN];

static unsigned int *screen = NULL;
static unsigned int cx = 1;
static unsigned int cy = 1;
static int desqview = 0;

static unsigned char scroll_fill [2];
static VIOMODEINFO fossil_data;


void pascal video_init()
{
	KBDINFO ki;
	fossil_data.cb = 2 * sizeof (fossil_data);
	VioGetMode(&fossil_data,0);    /* Get mode info 	 */
	maxx = fossil_data.col; 	   /* Maximum 'X' value  */
	maxy = fossil_data.row; 	   /* Maximum 'Y' value  */
	ki.cb = sizeof(KBDINFO);	   /* Set binary keyboard mode */
	ki.fsMask = KBD_BINARY;
	KbdSetStatus (&ki, 0);
}

void pascal video_end()
{
}

void pascal video_update()
{
	VioSetCurPos (cy - 1, cx - 1, 0);
}

void pascal scrollup(int x1,int y1,int x2,int y2,int lines)

{
	y2 = min(y2,maxy) - 1;
	y1 = min(y1,maxy) - 1;
	x1 = min(x1,maxx) - 1;
	x2 = min(x2,maxx) - 1;
	scroll_fill [0] = ' ';
	scroll_fill [1] = current_color;
	if (lines == 0)
		lines = -1;
	VioScrollUp (y1,x1,y2,x2,lines,(unsigned char *) scroll_fill,0);
}

void pascal scrolldown(int x1,int y1,int x2,int y2,int lines)

{
	y2 = min(y2,maxy) - 1;
	y1 = min(y1,maxy) - 1;
	x1 = min(x1,maxx) - 1;
	x2 = min(x2,maxx) - 1;
	scroll_fill [0] = ' ';
	scroll_fill [1] = current_color;
	VioScrollDn (y1,x1,y2,x2,lines,(unsigned char *)scroll_fill,0);
}

void pascal bputc(int c)
{
	unsigned int d = 0;

	d = ((unsigned) c & 0xff) | (current_color << 8);
	VioWrtCellStr ((PCH)&d, 2, cy - 1, cx - 1, 0);
	if (++cx > maxx) {
		cx = 1;
		if (++cy > maxy)
			cy = 1;
	}
}

void pascal gotoxy(int x, int y)
{
	cx = min(x,maxx);
	cy = min(y,maxy);
}

int pascal wherex()
{
	return (cx);
}

int pascal wherey()
{
	return(cy);
}

unsigned int pascal keyhit()

{
	if (macro)		/* always return no keypress during macros */
		return(0);

	if (!keybuf)
		keybuf = lgetkey();

	return(keybuf);
}


unsigned int pascal getkey()

{
	unsigned int k = keybuf;

	keybuf = 0;
	while (!k)
		k = lgetkey();
	return(k);
}


static unsigned int pascal lgetkey()

{
	KBDKEYINFO ki;

	if ((macros[0] != (void *) NULL) && !autostart) {
		autostart = 1;
		macro = macros[0];
	}

	if (macro != (void *) NULL) {
		if (*(++macro))
			return(*macro);

		macro = (void *) NULL;
	}
	ki.chChar = ki.chScan = 0;
	KbdCharIn (&ki, IO_WAIT, 0);
	if (ki.chChar == 0xe0) {
		if (ki.chScan) {
			ki.chChar = 0;						/* Force Scan return */
		} else {								/* Get next block	 */
			ki.chChar = 0;
			KbdCharIn (&ki, IO_WAIT, 0);
			if (!ki.chScan) {					/* Still no scan?	 */
				ki.chScan = ki.chChar;			/* Move new char over*/
				ki.chChar = 0;					/* Force its return  */
			} else {
				ki.chChar = 0;					/* Force new scan	 */
			}
		}
	}
	if (ki.chScan == 0xe0) {
		if (!ki.chChar) {
			ki.chScan = 0;
			KbdCharIn (&ki, IO_WAIT, 0);
			if (!ki.chScan) {					/* Still no scan?	 */
				ki.chScan = ki.chChar;			/* Move new char over*/
				ki.chChar = 0;					/* Force its return  */
			} else {
				ki.chChar = 0;					/* Force new scan	 */
			}
		} else {
			ki.chScan = 0;						/* Handle 0xe00d case*/
		}
	}

	if (ki.chChar)
		ki.chScan = 0;
	if ((ki.chScan >= 0x3b) && (ki.chScan <= 0x44))
		macro = macros[ki.chScan - 0x3a];
	else if ((ki.chScan >= 0x54) && (ki.chScan <= 0x71))
		macro = macros[ki.chScan - 0x49];

	if (macro != (void *) NULL) {
		if (*macro)
			return(*macro);

		macro = (void *) NULL;
	}

	return((unsigned int)((ki.chScan<<8)+(ki.chChar)));
}

void pascal bputs(char *s)

{
	unsigned int *linebase = screen + ((((cy - 1) * maxx) + (cx - 1)));
/*	unsigned int d; */
	char *t = s;
	char ch;
	unsigned int a = current_color;
	unsigned int l, l1 = 0;
	unsigned int limit = maxx - 1;

	if (s == NULL)
		return;

/*	d = current_color << 8; */

	if ((t = strchr(s,'\n')) != NULL) {
		ch = *t;
		*t = '\0';
	}

	if ((strlen(s) + cx) > maxx) {
		if (t)
			*t = ch;
		ch = *(t = s + maxx - cx + 1);
		*t = '\0';
	}

	if (strlen(s)) {
		l1 = l = (((strlen(s) + cx) < maxx)?strlen(s):maxx - cx + 1);
/*		cx += l; */
/*		VioWrtCharStrAtt(s,l,cy - 1,cx - l - 1, (PBYTE)&a,0); */
		VioWrtCharStrAtt((char *) s,l,cy - 1,cx - 1, (PBYTE) &a,0);
	}
	if (t)
		*t = ch;
	cx += l1;
}

void pascal cls()

{
	scrollup(1, 1, maxx, maxy, 0);
	gotoxy(1, 1);
}

void pascal clrwnd(int x1,int y1,int x2,int y2)

{
	scrollup(x1, y1, x2, y2, 0);
	gotoxy(x1,y1);
}

void pascal clreol()
{
	int x = cx;
/*	clrwnd(cx,cy,maxx,cy); */
	clrwnd(x,cy,maxx,cy);
	cx = x;

}

int pascal bgets(char *s1, int c, int w)

{
	int 	ch;
	int 	x1;
	char   *t;
	static	insert = ON;
	int 	y = wherey();
	int 	x = wherex();
	int 	ofs = 0;
	char	*o;

	o = strdup(s1);
	w = (w+x)>maxx?maxx-x:w;

	memset(s, 0, sizeof s);
	strcpy(s, s1);
	t = s + strlen(s);
	*t = '\0';
	bputs(s+ofs);

	while (TRUE) {			/* endless loop */
		video_update();
		switch (ch = getkey()) {

		case UP:
		case DOWN:
		case PGUP:
		case PGDN:
		case ENTER:
			free(o);
			strcpy(s1, s);
			return (ch);

		case ABORT:
			strcpy(s1, o);
			free(o);
			gotoxy(x,y);
			clreol();
			bputs(s1);
			return(ch);

		case WORDRT:
			break;

		case WORDLT:
			break;

		case CANCEL:
			memset(s, 0, sizeof s);
			t = s;
			gotoxy(x, y);
			clreol();
			break;

		case GOBOL:
			t = s;
			ofs = 0;
			gotoxy(x, y);
			break;

		case GOEOL:
			t = s + strlen(s);
			gotoxy(x + strlen(s) - ofs, y);
			break;

		case RUBOUT:
		case BKSPC:
			if (x == wherex())
				break;
			t--;
			memmove(t, (t + 1), strlen(t) + 1);
			gotoxy(wherex() - 1, y);
			x1 = wherex();
			bputs(t);
			bputc(' ');
			gotoxy(x1, y);
			break;

		case LEFT:
			if (t == s)
				break;
			t--;
			gotoxy(wherex() - 1, y);
			break;

		case RIGHT:
			if (t >= (s + strlen(s)))
				break;
			t++;
			gotoxy(wherex() + 1, y);
			break;

		case DELCHR:
			memmove(t, t + 1, strlen(t) + 1);
			x1 = wherex();
			bputs(t);
			bputc(' ');
			gotoxy(x1, y);
			break;

		case INSERT:
			insert = !insert;
			break;

		default:
			if (!(ch & 0xff) || (strlen(s) == (unsigned) c))
				break;
			if (insert) {
				x1 = wherex();
				t++;
				strins(s, (char) (ch & 0xff), (x1 - x + 1));
				gotoxy(x, y);
				bputs(s+ofs);
				if (ofs)
					gotoxy(x1, y);
				else
					gotoxy(x1 + 1, y);
			}
			else {
				*t++ = (char) (ch & 0xFF);
				bputc((char) (ch & 0xff));
			}
			video_update();
			break;
		}

		if (ofs) {
			gotoxy(x,y);
			bputs(s+ofs);
		}
	}
}

int pascal getnum(int l, int h, int value)

{
	int 	i, x;
	char	s[TEXTLEN];

	i = value;
	x = wherex();
	do {
		clreol();
		memset(s, 0, sizeof(s));
		itoa(i, s, 10);
		gotoxy(x, wherey());
		bgets(s, TEXTLEN/2, TEXTLEN/2);
		i = atoi(s);
	} while ((i < l) || (i > h));
	return (i);
}

void pascal set_color(unsigned int attr)

{
	current_color = (unsigned char) attr;
}

unsigned pascal get_color()

{
	return(current_color);
}

int cdecl bprintf(char *f,...)

{
	va_list params;
	int 	i;

	va_start(params, f);
	i = vsprintf(s, f, params);
	bputs(s);
	return (i);
}


CURSOR _cursor;

static void pascal vfossil_cursor (int st)
{
	CURSOR *q;

	q = (CURSOR *) &_cursor;
	/* We can make the cursor go away */
	VioGetCurType (q, 0);
	_cursor.cur_attr = st ? 0 : -1;
	VioSetCurType (q, 0);
}

