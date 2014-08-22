/*

Title:	MsgEd

File:	Editmail.c

Author: Jim Nutt

Copr:	Released into the PUBLIC DOMAIN 29 Jul 1990 by jim nutt

Description:

	routines to do text editing on messages

Support Files:

	msged.h

*/

#define NOSPELL
#include "msged.h"
#include "editmail.h"

#ifndef NOSPELL
#if defined(__ZTC__)
#include <sound.h>
#endif
#endif

#if defined(__ZTC__)
#include <swap.h>
#endif

#include <process.h>
#if defined(__TURBOC__)
#include <dir.h>
#else
#include <direct.h>
#endif

void _pascal e_assignkey(unsigned int key, char *label);
int  _pascal setcwd(char *path);
int  _pascal wrap(LINE *cl, int x, int y);
void _pascal refresh(LINE * curline, int i);
void _pascal putl(LINE *l);
int  _pascal editmsg(void);
char *	_pascal e_getlabels(int i);
char *	_pascal e_getbind(unsigned int key);

#define clearline(l,c)	memset(l,0,c)
#define BUFLEN 256
#define EDITMAIL

static int _near  _pascal down1(int y);
static void _near _pascal insert_char(char c);
static void _near _pascal unmark(LINE *current);
static void _near _pascal atputl(LINE *cl, int y);
static void _near _pascal setline(void);
#ifndef NOSPELL
static int _near  _pascal aspell(char *l);
#endif

static int done = FALSE;
static int insert = ON;
static int x = 1, y = 1;

static char line[BUFLEN];

#ifndef NOSPELL
char lwrd[128];
#endif

static LINE *current = NULL;
static LINE *first = NULL;
static LINE *pastebuf = NULL;
static int changed = 0;
static int noteflag;

LINE *anchorpt = NULL;
extern LINE *top;
extern LINE *bottom;

static void _near _pascal rotate()

{
	rot13 = (rot13 + 1) % 3;
}

static void _near _pascal nada()

{
}

static void _near _pascal rundos()

{
	char tmp[PATHLEN];
	char cmd[64];

	getcwd(tmp,PATHLEN);
	memset(cmd,0,sizeof cmd);
	gotoxy(9,1);
	clreol();
	set_color(colors.info);
	bputs("DOS command: ");
	bgets(cmd,63,63);
#ifdef __ZTC__
	if (swapping) swap_on();
	else swap_off();
#endif
	cls();
	system(cmd);
#ifdef __ZTC__
	if (!swapping) swap_on();
	else swap_off();
#endif
	maxx = maxy = 0;
	video_init();
	if (rm > maxx)
		rm = maxx - 1;
	puts("\nPress any key to return to msged.");
	getkey();
	setcwd(tmp);
	showheader(message);
	x = y = 1;
	refresh(current, 1);
}

char *	_pascal e_getbind(unsigned int key)

{
	unsigned int i = 0;
	void (_near _pascal *action)();

	if (key & 0xff)
		action = editckeys[key & 0xff];
	else
		action = editakeys[(key >> 8) & 0xff];

	while ((editcmds[i].label != NULL) &&
		   (action != editcmds[i].action))
		i++;

	return(editcmds[i].label);

}

char *	_pascal e_getlabels(int i)

{
	return(editcmds[i].label);
}

void	_pascal e_assignkey(unsigned int key, char *label)

{
	unsigned int i = 0;

	while ((editcmds[i].label != NULL) &&
		   (strncmp(editcmds[i].label,label,strlen(editcmds[i].label)) != 0))
		i++;

	if (editcmds[i].label != NULL)
		if (key & 0xff)
			editckeys[key & 0xff] = editcmds[i].action;
		else
			editakeys[(key >> 8) & 0xff] = editcmds[i].action;
}

#ifndef NOSPELL
static int _near _pascal aspell(char *l)

{
	static int speller = 2;

#if defined(__OS2__) || defined(NOSPELL)
	l;
	return(1);
#else
	speller = (speller == 2)?chkspl():speller;

	if (!(spell && speller))
		return(1);

	return !chkwrd(l);

#endif /* __OS2__ */                                                             /**OS2**/
}
#endif

LINE * _pascal insline(LINE *cl)

{
	LINE *nl;

	if ((nl = (LINE *) calloc(1, sizeof(LINE))) == NULL)
		return nl;

	if (cl == NULL)
		return nl;

	nl->next = cl;
	nl->text = NULL;
	nl->len = 0;
	nl->alloced = 0;
	nl->prev = cl->prev;

	if (nl->prev != NULL)
		nl->prev->next = nl;
	else
		first = nl;

	return cl->prev = nl;
}

static void _near _pascal setline(void)

{
	clearline(line,sizeof line);

	if (current->text != NULL)
		strcpy(line,current->text);
	else {
		current->text = NULL;
		current->len = 0;
		current->alloced = 0;
	}
}


static void _near _pascal unmark(LINE *current)

{
	if (changed) {
		if (current->text != NULL) {
			if (current->alloced > strlen(line)) {
				strcpy(current->text,line);
				current->len = strlen(line);
			}
			else {
				free(current->text);
				current->text = NULL;
			}
		}

		if (current->text == NULL) {
			current->text = strdup(line);
			current->len = current->alloced = strlen(line);
		}

		changed = FALSE;
	}
}

void _pascal putl(LINE *l)

{
	if (l->quote)
		set_color(colors.quote);
	else if (l->hide)
		set_color(colors.info);
	else if (l->block)
		set_color(colors.block);
	else
		set_color(colors.normal);
	bputs(l->text);
	set_color(colors.normal);
}

static void _near _pascal atputl(LINE *cl, int y)

{
	if ((y+6) <= maxy) {
		gotoxy(1,y+6);
		putl(cl);
		clreol();
	}
}



static void _near _pascal go_bol()

{
	x = 1;
}

static int _near _pascal down1(int y)
{
	if (++y > (maxy - 6)) {
		y = maxy - 6;
		scrollup(1, 7, maxx, maxy, 1);
	}
	gotoxy(1, y + 6);
	return (y);
}

int _pascal wrap(LINE *cl,int x,int y)

{
	int i = 0;
	int x2 = x;
	LINE *nl = NULL;
	char *s = NULL;
	char *w = NULL;
	char *t = NULL;
	char *tb = NULL;
	int len = 0;

	while (cl != NULL) {

		nl = cl->next;
		if ((cl->text != NULL) && (cl->len < (size_t) rm)) {
			if ((nl == NULL) || (strchr(cl->text,'\n')) ||
				(nl->text == NULL)) {
				atputl(cl,y);
				return(x2);
			}

			if ((size_t) (i = (rm - cl->len)) > nl->len) {
				len = cl->len + nl->len + 1;
				s = cl->text;
				if (cl->alloced < len) {
					s = (char *) realloc(cl->text,len);
					if (s)
						cl->alloced = len;
					else {
						refresh(cl,y);
						return(x2);
					}
				}

				cl->text = strcat(s,nl->text);
				cl->len = len;

				cl->next = nl->next;
				if (cl->next)
					cl->next->prev = cl;

				if (nl->text) free(nl->text);
				if (nl) free(nl);
				if ((y+6) < maxy) {
					int y2 = y + 7;
					atputl(cl,y);
					scrollup(1,y+7,maxx,maxy,1);
					nl = cl->next;
					while ((nl != NULL) && (y2++ < maxy))
						nl = nl->next;
					if (nl)
						atputl(nl,y2-7);
				}
			}
			else {
				t = nl->text + i;
				while ((t > nl->text) && !isspace(*t)) t--;
				if (t <= nl->text) {
					atputl(cl,y);
					atputl(nl,y+1);
					return(x2);
				}
				s = nl->text;
				nl->text = strdup(++t);
				nl->alloced = strlen(nl->text);

				nl->len = strlen(nl->text);
				*t = '\0';

				len = cl->len + strlen(s) + 1;
				t = cl->text;
				if (cl->alloced < len) {
					t = (char *) realloc(cl->text,len);
					if (t)
						cl->alloced = len;
					else {
						refresh(cl,y);
						return(x2);
					}
				}

				cl->text = strcat(t,s);
				cl->len = len;

				if (s) free(s);
				atputl(cl,y);
				atputl(nl,y+1);
			}
		}
		else {
			w = cl->text + rm;
			while ((w > cl->text) && !isspace(*w)) w--;
			while ((w > cl->text) && isspace(*w)) w++;
			if (w <= cl->text) w = cl->text + rm;
			if ((cl->text + x2) > w)
				x2 -= (int) (w - cl->text);
			if ((strchr(w,'\n') == NULL) && (nl != NULL)) {
				len = nl->len + strlen(w) + 1;
				t = nl->text;
				if (len > nl->alloced) {
					t = (char *) realloc(nl->text,strlen(nl->text) + strlen(w) + 1);
					if (t)
						nl->alloced = len;
					else {
						refresh(cl,y);
						return(x2);
					}
				}

				memmove(t + strlen(w), t, strlen(t)+1);
				memmove(t,w,strlen(w));

				nl->text = t;
				nl->len = strlen(t);

				*w = '\0';
				cl->len = strlen(cl->text);

				atputl(cl,y);
			}
			else {
				nl = (LINE *) calloc(1, sizeof(LINE));
				if (nl == NULL) {
					refresh(cl,y);
					return(x2);
				}
				nl->next = cl->next;
				nl->prev = cl;
				if (nl->next)
					nl->next->prev = nl;
				cl->next = nl;
				nl->text = (w)?strdup(w):NULL;
				nl->alloced = nl->len = (nl->text)?strlen(nl->text):0;
				*w = '\0';
				cl->len = strlen(cl->text);
				atputl(cl,y);
				if ((y+7) < maxy) {
					scrolldown(1,y+7,maxx,maxy,1);
					atputl(nl,y+1);
				}
			}
		}
		cl = cl->next;
		y++;
	}

	refresh(cl,y);
	return(x2);
}

static void _near _pascal insert_char(char c)

{
	int x2;

	changed = FALSE;
	strins(line, c, x);

	if (current->text) {
		if (current->alloced < strlen(line)) {
			free(current->text);
			current->text = strdup(line);
			current->alloced = current->len = strlen(line);
		}
		else {
			strcpy(current->text, line);
			current->len = strlen(line);
		}
	}
	else {
		current->text = strdup(line);
		current->alloced = current->len = strlen(line);
	}

	current->quote = 0;
	x2 = wrap(current,x,y);
	if (x2 != x) {
		current = current->next;
		x = x2;
		y = down1(y);
		atputl(current,y);
	}
	x++;
	strcpy(line,current->text);
	gotoxy(x,y);
}

static void _near _pascal delete_character()

{
	LINE *nl;
	int  y2 = y;
	char *s, *t;
	char ch;

	if (*line == '\n') {

		if (current->text)
			free(current->text);
		if (current->prev)
			current->prev->next = current->next;
		if (current->next)
			current->next->prev = current->prev;
		nl = current;
		current = (current->next)?current->next:(current->prev)?current->prev:nl;
		if (nl && (nl != current))
			free(nl);

		clearline(line,BUFLEN);
		nl = current;
		scrollup(1,y+7,maxx,maxy,1);
		while (nl) {
			atputl(nl,y2++);
			nl = nl->next;
		}
		if (current->text)
			  strcpy(line,current->text);
	}
	else {
		strdel(line,x);
		if (!strchr(line,'\n') && (current->next) && ((s = current->next->text)) != NULL)   {
			if ((t = strchr(s,' ')) != NULL) {
				while (isspace(*t)) t++;
				ch = *t; *t = '\0';
				strcat(line,s);
				*t = ch;
				if (current->next->alloced > strlen(t)) {
					strcpy(current->next->text,t);
				}
				else {
					current->next->text = strdup(t);
					current->next->alloced = strlen(t);
					free(s);
				}
				current->next->len = strlen(t);
			}
		}
		if (current->text)
			strcpy(current->text,line);
		else {
			current->text = strdup(line);
			current->alloced = strlen(line);
		}
		current->len = strlen(line);

		wrap(current,x,y);
		if (current && current->text)
			strcpy(line,current->text);
		else
			clearline(line,BUFLEN);
	}
	atputl(current,y);
	gotoxy(x,y);
	changed = FALSE;
}

static void _near _pascal backspace()

{
	if (x == 1) {
		if (current->prev == NULL)
			return;
		go_up();
		x = strlen(line);
		if (x == 1)
			delete_character();
		else {
			while ((x > 1) && isspace(*(line + x - 1))) x--;
			x++;
			while (*(line + x)) strdel(line,x);
			delete_character();
		}
		return;
	}

	strdel(line, --x);

	if (current->text) {
		if (current->alloced > strlen(line))
			strcpy(current->text,line);
		else {
			current->text = (char *) realloc(current->text,strlen(line) + 1);
			if (current->text) current->alloced = strlen(line);
			else current->alloced = 0;
		}
		current->len = strlen(line);
	}
	else {
		current->text = strdup(line);
		current->alloced = current->len = strlen(line);
	}
	wrap(current,x,y);
	if (current && current->text)
		strcpy(line,current->text);
	else
		clearline(line,BUFLEN);
	changed = FALSE;
	gotoxy(x, y + 6);
}

static void _near _pascal delword()

{
	char *s = line + x - 1;

	while ((*s) && !isspace(*s)) s++;
	while ((*s) && isspace(*s)) s++;
	strcpy(current->text + x - 1,s);
	current->len = strlen(current->text);
	changed = FALSE;
	wrap(current,x,y);
	strcpy(line,current->text);
}

static void _near _pascal go_eol()

{
	x = strlen(line);

	if (*(line + x - 1) != '\n')
		x++;

	x = max(1, min(rm,x));
}

static void _near _pascal go_up()

{
	if (current->prev == NULL)
		return;

	unmark(current);

	current = current->prev;

	if (y == 1) {
		scrolldown(1, y + 6, maxx, maxy, 1);
		atputl(current,y);
	}
	else
		y--;

	setline();


	if ((size_t) x > strlen(line))
		go_eol();
}

static void _near _pascal go_down()

{
	if (current->next == NULL)
		return;

	unmark(current);

	current = current->next;

	if (y > (maxy - 7)) {
		scrollup(1, 7, maxx, maxy, 1);
		atputl(current,y);
	}
	else
		y++;

	setline();

	if ((size_t) x > strlen(line))
		go_eol();

	x = max(1, x);
}

static void _near _pascal go_left()

{
	if (x == 1) {
		if (current->prev != NULL) {
			go_up();
			go_eol();
		}
		return;
	}
	x--;
}

static void _near _pascal go_right()

{
	if (*(line + x - 1) == '\0') {
		if (current->next != NULL) {
			go_down();
			x = 1;
		}
		return;
	}

	if (*(line + x - 1) == '\n') {
		if (current->next != NULL) {
			go_down();
			x = 1;
		}
		return;
	}
	x++;
}

static void _near _pascal go_word_right()

{
	if (*(line + x - 1) == '\0') {
		if (current->next != NULL) {
			go_down();
			x = 1;
		}
		return;
	}

	if (*(line + x - 1) == '\n') {
		if (current->next != NULL) {
			go_down();
			x = 1;
		}
		return;
	}

	while (isspace(*(line + x - 1)) && ((size_t) x <= strlen(line)))
		x++;
	while (!isspace(*(line + x - 1)) && ((size_t) x <= strlen(line)))
		x++;
	while (isspace(*(line + x - 1)) && ((size_t) x <= strlen(line)))
		x++;

	if (*(line + x - 2) == '\n')
		x--;
}

static void _near _pascal go_word_left()

{
	if (x == 1) {
		if (current->prev != NULL) {
			go_up();
			go_eol();
		}
		return;
	}

	while (isspace(*(line + x - 1)) && (x > 1))
		x--;
	while (!isspace(*(line + x - 1)) && (x > 1))
		x--;
	while (isspace(*(line + x - 1)) && (x > 1))
		x--;
	while (!isspace(*(line + x - 1)) && (x > 1))
		x--;

	if (x != 1)
		x++;
}

static void _near _pascal newline()

{
	char	l[BUFLEN];
	char   *t = line + x - 1;

	changed = FALSE;
	clearline(l, BUFLEN);
	strncpy(l, line, x - 1);
	strcat(l, "\n");
	current = insline(current);

	current->hard = 1;

	if (current->alloced > strlen(l))
		strcpy(current->text,l);
	else {
		free(current->text);
		current->text = strdup(l);
		current->alloced = strlen(l);
	}

	current->len = strlen(current->text);
	current = current->next;

	if (current->alloced < strlen(t)) {
		if (current->text) free(current->text);
		current->text = strdup(t);
		current->alloced = strlen(t);
	}
	else
		strcpy(current->text,t);

	current->len = strlen(t);

	clreol();
	x = 1;
	y = down1(y);
	gotoxy(x, y + 6);
	if ((y+6) < maxy)
		scrolldown(1,y+6,maxx,maxy,1);

	wrap(current,x,y);
	setline();
}

static void _near _pascal go_pgup()

{
	int 	count = y - 1;
	LINE   *l = current;

	unmark(current);
	while (count-- && current->prev) {
		current = current->prev;
	}

	count = maxy - 7;
	while (count-- && current->prev) {
		current = current->prev;
	}

	if (l != current) {
		y = 1;
		l = current;
		count = 0;
		clrwnd(1,7,maxx,maxy);
		while (((count + 7) < maxy) && (l->next)) {
			atputl(l,count++ + 1);
			l = l->next;
		}
		atputl(l,count + 1);
	}
	setline();
}

static void _near _pascal go_pgdown()

{
	int 	count = maxy - 7;
	LINE   *l = current;


	unmark(current);
	while (count-- && current->next) {
		current = current->next;
	}

	if (l != current) {
		count = 0;
		l = current;
		clrwnd(1,7,maxx,maxy);
		while (((count + 7) < maxy) && l->next) {
			atputl(l, ++count);
			l = l->next;
		}
		atputl(l, ++count);
	}
	y = 1;
	setline();
}

static void _near _pascal delete_line()

{
	LINE   *t = current;
	LINE   *l = current;
	int 	y2;

	changed = FALSE;

	if (current->next == NULL) {
		if (current->text != NULL)
			free(current->text);
		current->text = strdup("");
		current->alloced = 0;
		current->len = 0;
		current->hide = current->block = current->quote = 0;
		gotoxy(x=1,y+6);
		clearline(line,sizeof(line));
		clreol();
		return;
	}

	if ((y+6) < maxy)
		scrollup(1, y + 6, maxx, maxy, 1);
	else
		scrollup(1,maxy,maxx,maxy,0);

	t = current;

	clearline(line, BUFLEN);

	if (current->prev != NULL) {	 /* first line? */
		current->next->prev = current->prev; /* no */
		current->prev->next = current->next;
	}
	else {
		current->next->prev = NULL; /* yes */
		first = current->next;
	}

	current = current->next;

	message->text = first;

	if (t->text != NULL)
		free(t->text);

	t->text = NULL;

	free(t);
	t = NULL;
	setline();

	if ((size_t) x > strlen(line))
		go_eol();

	l = current; y2 = y;
	while (((y2 + 6) < maxy) && l->next && l->next->text) {
		y2++;
		l = l->next;
	}

	atputl(l,y2);
	x = max(1,x);
}

static void _near _pascal anchor()

{
	if (anchorpt != NULL) {
		set_color(colors.normal);
		refresh(current,y=1);
		anchorpt->block = 0;
		anchorpt->column = 0;
		anchorpt = NULL;
		while (pastebuf) {
			free(pastebuf->text);
			if (pastebuf->next) {
				pastebuf = pastebuf->next;
				free(pastebuf->prev);
			}
			else {
				free(pastebuf);
				pastebuf = NULL;
			}
		}
	}
	anchorpt = current;
	current->column = x;
	current->block = 1;
	atputl(current,y);
}

static void _near _pascal cut()

{
	LINE *f = current;
	LINE *l = current;

	unmark(current);
	while (l->next && (l != anchorpt))
		l = l->next;

	if (l != anchorpt) {
		l = current;
		while (f->prev && (f != anchorpt))
			f = f->prev;
		if (f != anchorpt)
			return;
	}

	if (f->prev == NULL)
		first = l->next;
	else
		f->prev->next = l->next;

	if (l->next != NULL) {
		current = l->next;
		l->next->prev = f->prev;
	}
	else
		current = f->prev;

	l->next = NULL;
	f->prev = NULL;

	if (first == NULL)
		first = current = (LINE *) calloc(1,sizeof(LINE));

	pastebuf = f;

	setline();
	refresh(current,1);
	y = x = 1;
}

static void _near _pascal paste()

{
	LINE *l = pastebuf;
	LINE *t,*s = NULL;
	LINE *t1 = current->prev;

	if (l == NULL)
		return;

	unmark(current);
	while (l) {
		t = (LINE *) calloc(1,sizeof(LINE));
		if (s == NULL)
			s = t;
		current->prev = t;
		t->next = current;
		t->prev = t1;
		if (t1 == NULL)
			first = t;
		else
			t1->next = t;
		t->text = (l->text)?strdup(l->text):NULL;
		t->alloced = t->len = (t->text)?strlen(t->text):0;
		l = l->next;
		t1 = t;
	}

	current = s;
	setline();
	refresh(current,1);
	y = x = 1;
}

static void _near _pascal quit()

{
	anchorpt = NULL;
	set_color(colors.normal);
	shownotes = noteflag;
	if (changed) {
		if (current->text)
			free(current->text);

		if ((current->text = strdup(line)) == 0) {
			gotoxy(9,1);
			set_color(colors.warn);
			bputs("*warning* ");
			bputs("memory error! message truncated, press a key");
			clreol();
			getkey();
			set_color(colors.normal);
		}
	}

	done =	SAVE;
}


static void _near _pascal die()

{
	if (!confirm())
		showheader(message);
	else
		done = ABORT;
}

static void _near _pascal imptxt()

{
	message->text = first;
	if (current->text != NULL)
		free(current->text);

	if (strlen(line) > 0) {
		current->text = strdup(line);
		current->alloced = current->len = strlen(line);
	}
	else
		current->text = NULL;

	import(current);
	first = message->text;
	refresh(current, 1);
	x = y = 1;
	setline();
	changed = FALSE;
}

static void _near _pascal outtext()

{
	if (pastebuf != NULL)
		export(pastebuf);
	else if (anchorpt != NULL)
		export(anchorpt);
	else if (current != NULL)
		export(current);
}


static void _near _pascal shellos()

{
	char tmp[PATHLEN];

	getcwd(tmp,PATHLEN);
	setcwd(home);
	unmark(current);
	cls();
	gotoxy(1,1);
	fputs("\nType EXIT to return to msged.\n",stderr);
#ifdef __ZTC__
	if (!swapping) swap_off();
	else swap_on();
#endif
	spawnl(0,comspec,comspec,NULL);
#ifdef __ZTC__
	if (swapping) swap_off();
	else swap_on();
#endif
	maxx = maxy = 0;
	video_init();
	if (rm > maxx)
		rm = maxx;
	setcwd(tmp);
	showheader(message);
	x = y = 1;
	refresh(current, 1);
}

static void _near _pascal toggle_ins()

{
	insert = !insert;
	if (insert) {
		set_color(colors.info);
		gotoxy((maxx - 4), 6);
		bputs("ins");
	}
	else {
		gotoxy((maxx - 4), 6);
		bputs("   ");
	}
	set_color(colors.normal);
}

static void _near _pascal tabit()

{
	if (!(x%tabsize))
		insert_char((char) ' ');

	while (x % tabsize)
		insert_char((char) ' ');

	insert_char((char) ' ');
}

static void _near _pascal go_tos()

{
	int 	count = y - 1;

	unmark(current);
	while (count-- && current->prev) {
		current = current->prev;
	}
	x = y = 1;
	setline();
}

static void _near _pascal go_bos()

{
	unmark(current);
	while (((y + 6) < maxy) && current->next) {
		current = current->next;
		y++;
	}
	x = 1;
	setline();
}

static void _near _pascal go_tom()

{
	unmark(current);
	current = first;
	setline();
	refresh(current,1);
	x = y = 1;
}

static void _near _pascal go_bom()

{
	unmark(current);
	while (current->next)
		current = current->next;
	x = y = 1;

	setline();
	if (current->prev) {
		refresh(current->prev,1);
		y = 2;
	}
	else
		refresh(current,1);
}

static void _near _pascal killeol()

{
	if (strchr(line,'\n') == NULL)
		strset(line + x - 1,'\0');
	else {
		*(line + x - 1) = '\n';
		strset(line+x,'\0');
	}
	changed = TRUE;
	clreol();
}

int _pascal editmsg()

{
	unsigned int     ch;

	noteflag = shownotes;
	anchorpt = NULL;
	shownotes = TRUE;
	x = y = 1;
	current = first = message->text;

	if (insert) {
		set_color(colors.info);
		gotoxy((maxx-4), 6);
		bputs("ins");
		set_color(colors.normal);
	}

	if (first == NULL) {
		if ((current = first = (LINE *) calloc(1, sizeof(LINE))) == NULL) {
			gotoxy(9,1);
			set_color(colors.warn);
			bputs("*warning* no memory!  press a key");
			clreol();
			getkey();
			set_color(colors.normal);
			return(ABORT);
		}

		first->len  = 0;
		first->alloced = 0;
		first->text = NULL;
		first->prev = NULL;
	}

	setline();

	refresh(current, 1);
	gotoxy(1, 7);
	done = FALSE;

	while (!done) {
		set_color(colors.normal);
		gotoxy(x,y+6);
		video_update();
		ch = getkey();
		if (ch & 0xff) {
#ifndef NOSPELL
			if (!isalpha(ch & 0xff) && isprint(ch & 0xff)) {
				char *l = line + x - 2;
				int i = 0;

				memset(lwrd,0,sizeof lwrd);
				while (isalpha(*l))
					lwrd[i++] = *l--;

				strrev(lwrd);

				if (strlen(lwrd) > 1) {
					if (!aspell(lwrd)) {
						gotoxy(maxx>>1, 5); clreol();
						set_color(colors.warn);
						bputs(lwrd);
#ifdef __ZTC__
						sound_beep(1171);
#endif

					}
				}
			}
#endif

			if (editckeys[(ch & 0xff)] == NULL) {
				changed = TRUE;
				if (insert)
					insert_char((char) DOROT13((char) (ch & 0xff)));
				else {
					*(line + x - 1) = (char) (ch & 0xff);
					gotoxy(x,y+6);
					bputc((char) (ch & 0xff));
					x++;
					changed = TRUE;
					if (x > rm) {
						changed = FALSE;
						if ((current->alloced > strlen(line)) && (current->text))
							strcpy(current->text,line);
						else {
							if (current->text) free(current->text);
							current->text = strdup(line);
							current->alloced = strlen(line);
						}
						current->len = strlen(line);
						x = wrap(current,x,y);
						y = down1(y);
						current = current->next;
						setline();
					}
					gotoxy(x,y+6);
				}
			}
			else
				editckeys[(ch & 0xff)]();
		}
		else if (editakeys[(ch >> 8)] != NULL)
			editakeys[(ch >> 8)]();
		else
			continue;
	}

	message->text = (current->prev == NULL)?current:first;
	shownotes = noteflag;
	return(done);
}
