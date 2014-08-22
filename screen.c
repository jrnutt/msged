/*

Title:  MsgEd

File:   Screen.c

Author: Jim Nutt

Copr:	released into the PUBLIC DOMAIN 30 jul 1990 by jim nutt

Description:

	Screen handling functions for MsgEd.

	if you need to port this... remember, msged uses a 1 based coordinate
	system (i.e. top left is (1,1) NOT (0,0)).

*/

#include "pascal.h"

#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#if !defined(__TURBOC__)
#include <conio.h>
#else
int kbhit(void);
int getch(void);
#endif
#ifndef MK_FP
#include "mkfp.h"
#endif
#include "screen.h"
#include "vfossil.h"

#ifndef _MSC_VER
#include "screen2.h"
#endif

static void pascal vfossil_cursor (int st);
static void pascal vfossil_init (void);
static void pascal vfossil_open (void);
static void pascal vfossil_close (void);

void pascal strins(char *l, char c, int x);
void pascal strdel(char *l, int x);

#define FALSE 0
#define TRUE !FALSE
#define TEXTLEN 256

int int86(int intnum,union REGS *i, union REGS *o);
int int86x(int intnum,union REGS *i, union REGS *o,struct SREGS *s);

unsigned int *macros[41];	 /* function key macros + 1 for autostart */

int maxx;               /* maximum screen columns */
int maxy;               /* maximum screen rows */
int videomethod;        /* how to write to the screen */
unsigned int vbase;     /* where screen memory is located */
unsigned char current_color;    /* current screen attribute */

/* keyboard control values */

static int autostart = 0;
static unsigned int * macro = (void *) NULL;

static char s[TEXTLEN];

static unsigned int *screen = NULL;
static unsigned int cx = 1;
static unsigned int cy = 1;
static int desqview = 0;

static unsigned char scroll_fill [2];
static VIOMODEINFO fossil_data;


void pascal video_init()
{
	union REGS r;
	struct SREGS sr;
	int vmode = 0;

	if (videomethod == ANSI) {
		maxx = 80;
		maxy = 25;
		desqview = 0;
		return;
	}

	if (videomethod == FOSSIL) {
		vfossil_init ();                /* Start Video FOSSIL */
		fossil_data.cb = 2 * sizeof (VIOMODEINFO);
		VioGetMode ((PVIOMODEINFO) &fossil_data,0);    /* Get mode info      */
		maxx = fossil_data.col;        /* Maximum 'X' value  */
		maxy = fossil_data.row;        /* Maximum 'Y' value  */
	}
	else {
		r.h.ah = 0x0f;
		int86(0x10,&r,&r);
		vmode = r.h.al;
		if (maxx == 0)
			maxx = (int) r.h.ah;

		if (maxy == 0) {
			r.x.ax = 0x1130;
			r.x.dx = maxy;
			int86(0x10,&r,&r);
			maxy = (r.x.dx == 0) ? 25 : (r.x.dx + 1);
		}

		if (videomethod == DIRECT) {
			if (vbase == 0)
				if (vmode == 0x07)
					vbase = 0xb000;
				else
					vbase = 0xb800;

			r.h.ah = 0xfe;
			sr.es = vbase;
			r.x.di = 0;
			int86x(0x10,&r,&r,&sr);
			desqview = (vbase != sr.es);
			vbase = sr.es;
			screen = (unsigned int *) MK_FP(vbase,r.x.di);
		}
	}
}

int  *getrgn(int x1,int y1,int x2,int y2)

{
	if ((x1 < 0) || (x2 > maxx) || (y1 < 0) || (y2 > maxy))
		return(NULL);

	return(NULL);
}

void putrgn(int x1,int y1,int x2,int y2,int *rgn)

{
	if ((x1 < 0) || (x2 > maxx) || (y1 < 0) || (y2 > maxy))
		return;

	if (!rgn)
		return;
}


void pascal video_end()
{
	if (videomethod == FOSSIL)
		vfossil_close ();
}

void pascal video_update()
{
	if (videomethod == ANSI) {
		printf("\033[%d;%dH",cy,cx);
	}
	else if (videomethod == FOSSIL)
		VioSetCurPos (cy - 1, cx - 1, 0);
	else {
		union REGS r;

		r.h.ah = 2;
		r.h.bh = 0;
		r.h.dh = (char) (cy - 1);
		r.h.dl = (char) (cx - 1);
		int86(0x10, &r, &r);
	}
}

void pascal scrollup(int x1, int y1, int x2, int y2, int lines)

{
	if (videomethod == ANSI) {
		printf("\033[%d;%dH\033F",y1,x1);
		printf("\033[%d;%dH\033G",y2,x2);
		printf("\033[%dS",lines);
		printf("\033[%d;%dH\033F",1,1);
		printf("\033[%d;%dH\033G",maxy,maxx);
		return;
	}

	y2 = min(y2,maxy);
	y1 = min(y1,maxy);
	x1 = min(x1,maxx);
	x2 = min(x2,maxx);

	if (videomethod == FOSSIL) {
		scroll_fill [0] = ' ';
		scroll_fill [1] = current_color;
		if (lines == 0)
			lines = -1;

		VioScrollUp (y1-1,x1-1,y2-1,x2-1,lines,(unsigned char *) scroll_fill,0);
	}
	else if (videomethod == BIOS) {
		union REGS      r;

		r.h.ah = 6;
		r.h.al = (char) lines;
		r.h.ch = (char) (y1-1);
		r.h.cl = (char) (x1-1);
		r.h.dh = (char) (y2-1);
		r.h.dl = (char) (x2-1);
		r.h.bh = current_color;
		int86(0x10, &r, &r);
	}
	else {
		while (lines--) {
#ifndef ASM
			int *scrptr = MK_FP(vbase,((y1-1) * maxx + (x1-1)) << 1);
			int ny = y1-1;
			int l = ((x2 - x1) + 1);

			while (ny++ < y2) {
				memcpy(scrptr,scrptr + maxx,l<<1);
				scrptr += maxx;
			}
			while (l)
				*(scrptr + --l) = 0x20 | (current_color << 8);
#else
			if (y1 != y2)
				dscrollup(x1,y1,x2,y2);
			else
				dclrwnd(min(x1,x2),min(y1,y2),max(x1,x2),max(y1,y2));
#endif
		}
	}
}

void pascal scrolldown(int x1, int y1, int x2, int y2, int lines)

{
	if (videomethod == ANSI) {
		printf("\033[%d;%dH\033F",y1,x1);
		printf("\033[%d;%dH\033G",y2,x2);
		printf("\033[%dT",lines);
		printf("\033[%d;%dH\033F",1,1);
		printf("\033[%d;%dH\033G",maxy,maxx);
		return;
	}

	y2 = min(y2,maxy);
	y1 = min(y1,maxy);
	x1 = min(x1,maxx);
	x2 = min(x2,maxx);

	if (videomethod == FOSSIL) {
		scroll_fill [0] = ' ';
		scroll_fill [1] = current_color;
		VioScrollDn (y1-1,x1-1,y2-1,x2-1,lines,(unsigned char *) scroll_fill,0);
	}
	else if (videomethod == BIOS) {
		union REGS      r;

		r.h.ah = 7;
		r.h.al = (char) lines;
		r.h.ch = (char) (y1-1);
		r.h.cl = (char) (x1-1);
		r.h.dh = (char) (y2-1);
		r.h.dl = (char) (x2-1);
		r.h.bh = current_color;
		int86(0x10, &r, &r);
	}
	else {
		while (lines--) {
#ifndef ASM
			int ny = y2-1;
			int l = ((x2 - x1) + 1);
			int *scrptr = MK_FP(vbase,((y2-1) * maxx + (x1-1)) << 1);
			while (ny-- >= y1) {
				memcpy(scrptr,scrptr - maxx,l<<1);
				scrptr -= maxx;
			}
			while (l)
				*(scrptr + --l) = 0x20 | (current_color << 8);
#else
			if (y1 != y2)
				dscrolldn(x1,y1,x2,y2);
			else
				dclrwnd(min(x1,x2),min(y1,y2),max(x1,x2),max(y1,y2));
#endif
		}
	}
}

void pascal bputc(int c)

{
#ifndef ASM
	union REGS r;
#endif

	unsigned int d = 0;

	if (videomethod == ANSI) {
		printf("\033[%d;%dH\033[%d;%dm%c",cy,cx,
			30+(current_color & 0x7),
			40+(current_color>>4),c&0x7f);
		return;
	}

#ifndef ASM
	if (videomethod == DIRECT) {
		*(screen + ((((cy - 1) * maxx) + (cx - 1)))) = d;
#else
	if ((videomethod == DIRECT) || (videomethod == BIOS)) {
		dputc(cx,cy,c);
#endif
	}
	else if (videomethod == FOSSIL) {
		d = ((unsigned) c & 0xff) | (current_color << 8);
		VioWrtCellStr ((unsigned int *) &d, 2, cy - 1, cx - 1, 0);
	}
#ifndef ASM
	else {
		video_update();
		r.h.ah = 0x9;
		r.h.bh = 0;
		r.h.al = (unsigned char) c;
		r.h.bl = current_color;
		r.x.cx = 1;
		int86(0x10,&r,&r);
	}
#endif
	if (++cx > (unsigned) maxx) {
		cx = 1;
		if (++cy > (unsigned) maxy)
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
	union REGS r;

	if (macro)      /* always return no keypress during macros */
		return(0);

	if (videomethod == FOSSIL) {
		r.x.ax = 0x0d00;
		int86(0x14,&r,&r);
		if (r.x.ax == 0xffff)
			return(0);
	}
	else {
		r.h.ah = 1;
		int86(0x16,&r,&r);
#if defined(_MSC_VER)
		if (r.x.cflag)
#else
		if (r.x.flags & 64)
#endif
			return(0);
	}

	if (r.h.al)                     /* Must use FOSSIL keymap,   */
		r.h.ah = 0;             /* No scan code if have char */

	return r.x.ax;
}

unsigned int pascal getkey()

{
	union REGS r;

	if ((macros[0] != (void *) NULL) && !autostart) {
		autostart = 1;
		macro = macros[0];
	}

	if (macro != (void *) NULL) {
		if (*(++macro))
			return(*macro);

		macro = (void *) NULL;
	}

	if (videomethod == FOSSIL) {
		r.h.ah = 0x0e;
		int86 (0x14,&r,&r);
	}
	else {
		r.h.ah = 0;
		int86(0x16,&r,&r);
	}

	if (r.h.al) {                    /* Must use FOSSIL keymap,   */
		r.h.ah = 0;             /* No scan code if have char */
		return(r.x.ax);
	}

	if ((r.h.ah >= 0x3b) && (r.h.ah <= 0x44))
		macro = macros[r.h.ah - 0x3a];
	else if ((r.h.ah >= 0x54) && (r.h.ah <= 0x71))
		macro = macros[r.h.ah - 0x49];

	if (macro != (void *) NULL) {
		if (*macro)
			return(*macro);

		macro = (void *) NULL;
	}

	return(r.x.ax);
}

void pascal bputs(char *s)

{
#ifndef ASM
	unsigned int *linebase = screen + ((((cy - 1) * maxx) + (cx - 1)));
	unsigned int d;
#endif
	char *t = s;
	char ch = '\0';
	unsigned int a = current_color;
	unsigned int l = 0,l1 = 0;

	if (s == NULL)
		return;

	if (videomethod == ANSI) {
		printf("\033[%d;%dH\033[%d;%dm%0.79s",cy,cx,
			30+(current_color & 0x7),
			40+(current_color>>4),s);
			return;
	}

	if ((t = strchr(s,'\n')) != NULL) {
		ch = *t;
		*t = '\0';
	}

	if ((strlen(s) + cx) > (size_t) maxx) {
		if (t) *t = ch;
		ch = *(t = s + maxx - cx + 1);
		*t = '\0';
	}

#ifdef ASM
	if ((videomethod == DIRECT) || (videomethod == BIOS)) {
		cx += dputs(cx,cy,s);
		if (t) *t = ch;
		return;
	}
#endif

	if (strlen(s)) {
		l1 = l = (((strlen(s) + cx) < (size_t) maxx)?strlen(s):maxx - cx + 1);

		if (videomethod == FOSSIL)
			VioWrtCharStrAtt((char *) s,l,cy - 1,cx - 1, (unsigned char *) &a,0);
#ifndef ASM
		else
			if (videomethod == DIRECT) {
				d = current_color << 8;
				while (l--)
					*linebase++ = (unsigned int) (*s++ & 0xff) | d;
			}
			else
				while (l--)
					bputc(*s++);
#endif
	}

	if (t) *t = ch;
	if (videomethod != BIOS)
		cx += l1;
}

void pascal cls()

{
	if (videomethod == ANSI) {
		fputs("\033[2J",stdout);
		return;
	}

	if (videomethod == DIRECT) {

#ifndef ASM
		int ny = 0;
		int *scrptr = MK_FP(vbase,0);
		while (ny++ < maxy) {
			int l = maxx;
			while (l)
				*(scrptr + --l) = 0x20 | (current_color << 8);
			scrptr += maxx;
		}
#else
		dcls();
#endif
	}
	else {
		scrollup(1, 1, maxx, maxy, 0);
		gotoxy(1, 1);
	}
}

void pascal clrwnd(int x1, int y1, int x2, int y2)

{
	if (x1 > x2) return;
	if (y1 > y2) return;

	if (videomethod == ANSI) {
		printf("\033[%d;%dH\033F",y1,x1);
		printf("\033[%d;%dH\033G",y2,x2);
		printf("\033[O");
		printf("\033[%d;%dH\033F",1,1);
		printf("\033[%d;%dH\033G",maxy,maxx);
		return;
	}
	if (videomethod == DIRECT) {
#ifndef ASM
		int ny = y1-1;
		int *scrptr = MK_FP(vbase,((y1-1) * maxx + (x1-1)) * 2);

		while (ny++ <= (y2-1)) {
			int l = (((x2-1) - (x1-1)) + 1);
			while (l)
				*(scrptr + --l) = 0x20 | (current_color << 8);
			scrptr += maxx;
		}
#else
		dclrwnd(min(x1,x2),min(y1,y2),max(x1,x2),max(y1,y2));
#endif
	}
	else {
		scrollup(x1, y1, x2, y2, 0);
		gotoxy(x1,y1);
	}
}

void pascal clreol()
{
	int x = cx;

	if (videomethod == ANSI) {
		fputs("\033[K",stdout);
	}
	else if (videomethod == BIOS) {
		union REGS r;

		video_update();
		r.h.ah = 0x9;
		r.h.bh = 0;
		r.h.al = 0x20;
		r.h.bl = current_color;
		r.x.cx = (maxx - cx + 1);
		int86(0x10,&r,&r);
	}
	else
		clrwnd(x,cy,maxx,cy);
	cx = x;
}

int pascal bgets(char *s, int c, int w)

{
	int x1,x2,y,ch,tx,m=1;
	char *o,fmt[10],*p;
	static ins = 1;
	
	w = min(c,w);
	x1 = max(0,cx); y = min(maxy,max(0,cy)); x2 = min(cx + w,(unsigned) maxx);
	o = strdup(s);
	p = s;
	sprintf(fmt,"%%-%d.%ds",w,w);
	cx = x1 + strlen(s);
	if (cx > (unsigned) x2) {
		p = s + (cx - x2);
		cx = x2;
	}

	for (;;) {
		if (m) {
			tx = cx;
			clrwnd(x1,y,x2,y);
			cx = x1; bprintf(fmt,p); 
			cx = tx; m = 0;
		}

		video_update();
		switch (ch = getkey()) {
			case UP:
			case DOWN:
			case ENTER:
				free(o);
				return(ch);

			case GOBOL:
				cx = x1;
				p = s;
				m = 1;
				break;
	

			case GOEOL:
				cx = x1 + strlen(p);
				if (cx > (unsigned) x2) {
					p += cx - x2;
					cx = x2 - 1;
				}
				m = 1;
				break;

			case ABORT:
				strcpy(s,o);
				free(o);
				return(ch);

			case INSERT:
				ins = !ins;
				break;

			case BKSPC:
				if (cx > (unsigned) x1) {
					cx--;
					strdel(p,cx - x1 + 1);			
					m = 1;
				}
				else if (p > s) {
					p--;
					strdel(p,1);
					m = 1;
				}
				break;

			case WORDLT:
				break;

			case WORDRT:
				break;

			case LEFT:
				if (cx > (unsigned) x1) cx--;
				else if (p > s) { p--; m = 1; }
				break;

			case RIGHT:
				if (cx < (unsigned) (x2-1)) {
					if ((cx-x1) < strlen(p)) cx++;
				}
				else if ((unsigned) w < strlen(p)) { p++; m = 1; }
				break;

			case DELCHR:
				strdel(p,cx - x1 + 1);
				m = 1;
				break;

			case CANCEL:
				p = s; m = 1; cx = x1;
				strset(s,0);
				break;

			default:
				if (!(ch & 0xff))
					break;

				if (ins)
					if (strlen(s) >= (unsigned) c) break;
					else strins(p,(char) (ch & 0xff), (int) cx - x1 + 1);
				else
					*(p + cx - x1) = (char) (ch & 0xff);
				
				if (cx < (unsigned) x2)
					cx++;
				else if ((unsigned) w < strlen(p)) p++;

				m = 1;

				break;
			}
	}
}

int pascal getnum(int l, int h, int value)

{
	int     i, x;
	char    s[TEXTLEN];

	i = value;
	x = wherex();

	do {
		memset(s, 0, sizeof(s));
		itoa(i, s, 10);
		gotoxy(x, wherey());
		bgets(s, sizeof s << 1,min(maxx - x, sizeof s << 1));
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
	int     i;

	va_start(params, f);
	i = vsprintf(s, f, params);
	bputs(s);
	return (i);
}

VFOSSIL v;

static void pascal vfossil_init ()

{
	char *q;
	union REGS r;
	struct SREGS s;

	v.vfossil_size = sizeof (VFOSSIL);
	q = (char *) &v;

	r.h.ah = 0x81;
	r.h.al = 0;

	segread (&s);
	s.es = FP_SEG (q);
	r.x.di = FP_OFF (q);
	int86x (0x14, &r, &r, &s);

	if (r.x.ax == 0x1954)
	/* There is a VFOSSIL out there, so set it up for use */
		vfossil_open ();
	else {
		fputs ("No Video FOSSIL installed, aborting....\n\n",stderr);
		exit (255);
	}
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

static void pascal vfossil_open ()

{
	char *q;
	union REGS r;
	struct SREGS s;

	segread (&s);
	r.h.ah = 0x81;
	r.h.al = 1;
	r.x.cx = 80;
	q = (char *) &vfossil_funcs;
	r.x.di = FP_OFF (q);
	s.es = FP_SEG (q);
	int86x (0x14, &r, &r, &s);
	if ((r.x.ax != 0x1954) || (r.x.bx < 14)) {
		fputs ("Unable to initialize Video FOSSIL, aborting....\n\n",stderr);
		exit (255);
	}
}

static void pascal vfossil_close ()

{
   union REGS r;

   vfossil_cursor (1);

   r.h.ah = 0x81;
   r.h.al = 2;

   int86 (0x14, &r, &r);
}
