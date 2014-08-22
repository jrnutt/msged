/*

Title:	MsgEd

File:	Msged.c

Author: Jim Nutt

Copr:	released into the PUBLIC DOMAIN 30 jul 1990 by jim nutt

Description:

	Small, fast fido message editor.

Support Files:

	msged.h

*/

#define MAIN

#include "msged.h"
#include "maincmds.h"
#include "bmg.h"

#include <process.h>

#if defined(__TURBOC__)
#include <dir.h>
#else
#include <direct.h>
#endif

#ifdef __ZTC__
#include <dos.h>
#include <swap.h>
static int brk;
#endif

unsigned int _vcm_maxres = 0;

void	_pascal r_assignkey(unsigned int key, char *label);
char *	_pascal r_getbind(unsigned int key);
char *	_pascal r_getlabels(int i);

int 	_pascal setcwd(char *path);
static	void _near _pascal set_area(int area);
static	void _near _pascal empty(void);
static	void _near _pascal highest(void);
static	int _near _pascal start(char *, char *);

int 	direction = RIGHT;

static	int 	lastfound = 0;
static	int 	command;
static	int 	root = 0;		/* root message of a thread */
static	int 	back = 0;

char * _pascal show_address(ADDRESS a)

{
	static char s[80];
	char field[20];

	memset(s,0,sizeof s);
	memset(field,0,sizeof field);

	if (a.notfound) {
		strcpy(s,"unknown address");
		return(s);
	}

	if (a.fidonet) {
		if (a.zone) {
			strcat(s,itoa(a.zone,field,10));
			strcat(s,":");
		}
		strcat(s,itoa(a.net,field,10));
		strcat(s,"/");
		strcat(s,itoa(a.node,field,10));
		if (a.point) {
			strcat(s,".");
			strcat(s,itoa(a.point,field,10));
		}

		if (a.domain != NULL) {
			strcat(s,"@");
			strcat(s,strlwr(a.domain));
		}
	}

	if (a.internet) {
		strncpy(s,a.domain,60);
	}

	if (a.bangpath) {
		char *t,*t1;

		t1 = strrchr(a.domain,'!');
		if (t1 == NULL)
			strcpy(s,a.domain);
		else {
			*t1 = '\0';
			t = strrchr(a.domain,'!');
			if (!t)
				t = a.domain;
			*t1 = '!';
			strcat(strcpy(s,"..."),t);
		}

	}

	return(s);
}

static void _near _pascal delete()

{
	deletemsg();
}

static void _near _pascal doreply()

{
	reply();
}

static void _near _pascal doquote()

{
	quote();
}

static void _near _pascal move()

{
	movemsg();
}

static void _near _pascal outtxt()

{
	writetxt();
}

static void _near _pascal set()

{
	settings();
}

static void _near _pascal new()

{
	newmsg();
}

static void _near _pascal dochange()

{
	change();
}

char *	_pascal r_getbind(unsigned int key)

{
	unsigned int i = 0;
	void (_near _pascal *action)();

	if (key & 0xff)
		action = mainckeys[key & 0xff];
	else
		action = mainakeys[(key >> 8) & 0xff];

	while ((maincmds[i].label != NULL) &&
		   (action != maincmds[i].action))
		i++;

	return(maincmds[i].label);

}

char *	_pascal r_getlabels(int i)

{
	return(maincmds[i].label);
}

void	_pascal r_assignkey(unsigned int key, char *label)

{
	unsigned int i = 0;

	while ((maincmds[i].label != NULL) &&
		   (strncmp(maincmds[i].label,label,strlen(maincmds[i].label)) != 0))
		i++;

	if (maincmds[i].label != NULL)
		if (key & 0xff)
			mainckeys[key & 0xff] = maincmds[i].action;
		else
			mainakeys[(key >> 8) & 0xff] = maincmds[i].action;
}

ADDRESS _pascal parsenode(char *t)

{
	ADDRESS tmp;
	char *t1 = t;
	int  n = 0;
	int point = 0;

	tmp = thisnode; /* msc doesn't allow auto struct initializers */
	tmp.point = tmp.notfound = 0;
	tmp.fidonet = 1;
	tmp.internet = 0;
	tmp.bangpath = 0;
	if (thisnode.domain != NULL)
		tmp.domain = strdup(thisnode.domain);

	if (t == NULL) {
		tmp.notfound = 1;
		return(tmp);
	}

	while (isspace(*t)) t++;
	if (!isdigit(*t)) {
		tmp.notfound = 1;
		tmp.fidonet = 0;
		tmp.internet = 0;
		tmp.bangpath = 0;
		return(tmp);
	}

	while (t1) {
		n = (int) strtol(t1,&t1,10);
		if (t1 == NULL) {
			if (point) tmp.point = n;
			else tmp.node = n;
			if (tmp.zone == 0) tmp.zone = thisnode.zone;
			return(tmp);
		}
		switch (*t1) {

			case ')' :
			case ' ' :
			case '\0':
				if (point) tmp.point = n;
				else tmp.node = n;
				if (tmp.zone == 0) tmp.zone = thisnode.zone;
				return(tmp);

			case ':' :
				tmp.zone = n;
				break;

			case '/' :
				tmp.net = n;
				break;

			case '.' :
				tmp.node = n;
				point = 1;
				break;

			case '@' :
				if (point) tmp.point = n;
				else tmp.node = n;
				if (tmp.domain != NULL) free(tmp.domain);
				tmp.domain = strdup(t1+1);
				if (strchr(tmp.domain,' ')) *strchr(tmp.domain,' ') = '\0';
				if (tmp.zone == 0) tmp.zone = thisnode.zone;
				return(tmp);
		}
		t1++;
	}

	if (tmp.zone == 0) tmp.zone = thisnode.zone;
	return(tmp);
}

void _pascal dispose(MSG *message)

{
	if (message == NULL)
		return;

	if (message->text != NULL)
		message->text = clearbuffer(message->text);
	release(message->date);
	release(message->links);
	release(message->isto);
	release(message->isfrom);
	release(message->reply);
	release(message->subj);
	release(message->to.domain);
	release(message->from.domain);
	release(message);
}

static void _near _pascal quit()

{
	cleanup();
}

void _pascal cleanup()
{
#ifdef _OS2_
	int errorlevel = 0;
#else
	unsigned char errorlevel = 0;
#endif
	FILE   *fp;
	AREA *a;

	highest();
	msg_last(arealist[area]);

	setcwd(home);
#ifdef __ZTC__
	dos_set_ctrl_break(brk);
#endif

	cls();

	video_end();

	if (*confmail == '+')
		fp = fopen(confmail+1,"at");
	else
		fp = fopen(confmail, "wt");

	for (a = &(arealist[area = 0]); area < areas; a = &(arealist[++area])) {
		errorlevel |= (a->netmail && a->new)? 0x01:0;
		errorlevel |= (a->echomail && a->new)?0x02:0;
		errorlevel |= (a->uucp && a->new)?	  0x04:0;
		errorlevel |= (a->news && a->new)?	  0x08:0;
		errorlevel |= (a->local && a->new)?   0x10:0;

		if (a->echomail && a->new)
			fprintf(fp, "%s\n", a->tag);
	}

	printf("\nexiting with errorlevel %d\n",errorlevel);

	if (messages)
		free(messages);

	if (arealist)
		handle_free(arealist);

#ifdef _OS2_
	fclose(fp);
#else
	fcloseall();
#endif

	exit(errorlevel);
}

void _pascal outamemory()
{
	fprintf(stderr, "\ni've run out of memory!  aborting....\n");
	cleanup();
}

static void _near _pascal empty()

{
	arealist[area].last = arealist[area].current = arealist[area].first = 0;
	cls();
	gotoxy(3, maxy / 2);
	set_color(colors.hilite);
	bprintf("%s is empty!",arealist[area].description);
	set_color(colors.normal);
}

static void _near _pascal search()

{
	char   *t = NULL;
	int 	tmp = 0;
	MSG    *m;

	static	bmgARG pattern;
	static	char prompt[bmgMAXPAT];
	static	char patstr[bmgMAXPAT];

	gotoxy(9, 1);
	clreol();
	set_color(colors.info);
	bputs("Find: ");
	set_color(colors.normal);
	bgets(prompt,sizeof(prompt)-1,sizeof(prompt)-1);

	if (strlen(prompt) == 0)
		return;

	if (strcmpl(prompt,patstr) == 0)
		tmp = lastfound + 1;
	else {
		tmp = arealist[area].current;
		lastfound = 0;
		memset(patstr,0,sizeof(patstr));
		memset(&pattern,0,sizeof(bmgARG));
		strncpy(patstr,prompt,sizeof(prompt));
		bmgCompile(patstr,&pattern,TRUE);
	}

	gotoxy(9,1);
	clreol();
	set_color(colors.info);
	for (;tmp < arealist[area].messages; tmp++) {
		if (keyhit() && (getkey() == ABORT)) {
			msg_readtext(-1);
			return;
		}

		if ((m = msg_readheader(msgnum(tmp))) != NULL) {
			gotoxy(9,1);
			bprintf("Looking at #%d for \"%s\"",msgnum(tmp),prompt);
			if (bmgSearch(m->isto,strlen(m->isto),&pattern) != NULL) {
				lastfound = tmp;
				set_color(colors.normal);
				arealist[area].current = tmp;
				msg_readtext(-1);
				return;
			}

			if (bmgSearch(m->isfrom,strlen(m->isfrom),&pattern) != NULL) {
				lastfound = tmp;
				set_color(colors.normal);
				arealist[area].current = tmp;
				msg_readtext(-1);
				return;
			}

			if (bmgSearch(m->subj,strlen(m->subj),&pattern) != NULL) {
				lastfound = tmp;
				set_color(colors.normal);
				arealist[area].current = tmp;
				msg_readtext(-1);
				return;
			}

			while ((t = msg_readtext(msgnum(tmp))) != NULL) {
				if (bmgSearch(t,strlen(t),&pattern) != NULL) {
					lastfound = tmp;
					set_color(colors.normal);
					if (t) free(t);
					arealist[area].current = tmp;
					msg_readtext(-1);
					return;
				}
				if (t) free(t);
				t = NULL;
			}
		}
	}
	msg_readtext(-1);
	set_color(colors.normal);
}

int _pascal confirm()

{
	int ch;

	if (!(confirmations))								   /*WRA*/
		return(TRUE);

	gotoxy(9,1);
	clreol();
	set_color(colors.warn);
	bputs("*warning* ");
	bputs("do you really want to do this? (y/n) ");
	video_update();

	do {
		ch = getkey() & 0x7f;
		ch = tolower(ch);

	} while ((ch != 'y') && (ch != 'n'));

	bputc((char) ch);
	set_color(colors.normal);
	return(ch == 'y');
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
		rm = maxx;
	puts("\nPress any key to return to msged.");
	getkey();
	setcwd(tmp);
}

static	void _near _pascal set_area(int area)

{
	lastfound = 1;
	arealist[area].messages = msg_scan((&(arealist[area])));
	direction = RIGHT;
	back = root = 0;
	back = root = arealist[area].current;
	dispose(message);
	message = NULL;
}

void _pascal areascan()

{
	scan_areas();
}

static void _near _pascal scan_areas()

{
	int a = 0;

	highest();
	msg_last(arealist[area]);
	set_color(colors.warn);
	for (;a < areas; a++) {
		gotoxy(9,1);
		clreol();
		video_update();
		bprintf("scanning %s for new mail (<esc> to interrupt)",arealist[a].description);
		if (keyhit() && (getkey() == ABORT))
			break;
		else
			arealist[a].messages = msg_scan((&arealist[a]));
	}

	set_color(colors.normal);
	scanned = 1;
	arealist[area].messages = msg_scan((&arealist[area]));
}

static void _near _pascal next_area()

{
	int oarea = area;

	override = FALSE;
	if (areas < 2)
		return;
	highest();
	msg_last(arealist[area]);
	if (scanned) {
		area = (area + 1) % areas;
		while ((arealist[area].messages - arealist[area].lastread - 1) <= 0) {
			area = (area + 1) % areas;
			if (area == oarea) {
				area = (area + 1) % areas;
				break;
			}
		}
	}
	else
		area = (area + 1) % areas;
	set_area(area);
}

static void _near _pascal prev_area()

{
	int oarea = area;

	override = FALSE;
	if (areas < 2)
		return;
	highest();
	msg_last(arealist[area]);
	if (scanned) {
		area--;
		area = (area < 0)?areas-1:area;
		while ((arealist[area].messages - arealist[area].lastread - 1) <= 0) {
			area--;
			area = (area < 0)?areas-1:area;
			if (area == oarea) {
				area--;
				area = (area < 0)?areas-1:area;
				break;
			}
		}
	}
	else {
		area--;
		area = (area < 0)?areas-1:area;
	}
	set_area(area);
}

static void _near _pascal highest()

{
	arealist[area].lastread = min(max(arealist[area].lastread,arealist[area].current),arealist[area].messages-1);
	root = arealist[area].current;
}


static void _near _pascal left()

{
	back = arealist[area].current;
	direction = LEFT;

	if (arealist[area].current > 0)
		arealist[area].current--;

	root = arealist[area].current;
}

static void _near _pascal right()

{
	back = arealist[area].current;
	direction = RIGHT;

	if (arealist[area].current < (arealist[area].messages-1))
		arealist[area].current++;

	highest();
}

static void _near _pascal gotomsg(int i)

{
	gotoxy(9, 1);
	clreol();
	set_color(colors.info);
	bputs("Goto Message #? ");
	set_color(colors.normal);
	back = arealist[area].current;
	i = getnum(0, arealist[area].messages, i);
	if (i == 0)
		return;
	arealist[area].current = i-1;
	highest();
}


static void _near _pascal newarea()

{
	override = FALSE;
	if (areas < 2)
		return;
	highest();
	msg_last(arealist[area]);
	area = selectarea();
	set_area(area);
}

static int _near _pascal start(char *cfg, char *afn)

{
	opening(cfg,afn);

	area = 0;

	arealist[area].messages = msg_scan(&(arealist[area]));

	gotoxy(6, maxy);
	bputs("Press any key to continue....");
	getkey();

	if (arealist[area].messages > 0) {
		if ((message = readmsg(msgnum(arealist[area].current))) != NULL)
			return showmsg(message);
	}
	else
		return 0x1e00;

	return 0x1e00;
}

static void _near _pascal go_last()

{
	arealist[area].current = min(arealist[area].lastread, arealist[area].messages-1);
	root = arealist[area].current;
}

static void _near _pascal link_from()

{
	int t;
	if (!message->replyto)
		return;

	back = arealist[area].current;
	for (t = arealist[area].current; (t > -1) && ((unsigned) msgnum(t) != message->replyto); t--)
		/* null statement */ ;
	if (t > -1)
		arealist[area].current = t;
}

static void _near _pascal view()

{
	shownotes = !shownotes;
	seenbys = shownotes;
}

static void _near _pascal link_to()

{
	int t;
	if (!message->replyfrom)
		return;
	back = arealist[area].current;
	for (t = arealist[area].messages - 1; (t > arealist[area].current) && ((unsigned) msgnum(t) != message->replyfrom); t--)
		/* null statement */;
	if (t > arealist[area].current)
		arealist[area].current = t;

	arealist[area].lastread = max(arealist[area].lastread,arealist[area].current);
}

static void _near _pascal go_root()

{
	back = arealist[area].current;
	arealist[area].current = root;
	highest();
}

static void _near _pascal rotate()

{
	rot13 = (rot13 + 1) % 3;
}

static void _near _pascal go_dos()

{
	char tmp[PATHLEN];

	getcwd(tmp,PATHLEN);
	setcwd(home);
	cls();
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
}

static void _near _pascal nada()

{
}

typedef struct _mlhead {
	int n;
	char to_name[37];
	int  to_net;
	int  to_node;
	char fr_name[37];
	int  fr_net;
	int  fr_node;
	char subj[73];
} MLHEAD;

static void _near _pascal update(MLHEAD *headers, int n,int y);
static void _near _pascal showit(MLHEAD *h);
static void _near _pascal getheader(int n, MLHEAD *h);

static	char	tmp[16];
static	int 	f = 1;

void _near _pascal list()

{
	MLHEAD *headers = NULL;
	int i = 0, y = 0, ch = 0, a = 0;
	char   *s = NULL;
	static int inlist = 0;

	if (inlist) return;
	else inlist = 1;

	cls();
	sprintf(tmp,"%%-%d.%ds",maxx,maxx);
	i = a = arealist[area].current;
	highest();
	headers = (MLHEAD *) calloc((maxy+1),sizeof(MLHEAD));
	if (headers == NULL) outamemory();

	y = 1;

	update(headers,i,y);

	ch = 0;
	y = 1;

	while ((ch != ENTER) && (ch != ABORT) && arealist[area].messages) {
		gotoxy(1, y);
		clreol();
		set_color(colors.hilite);
		showit(&headers[y-1]);
		set_color(colors.normal);

		switch (ch = getkey()) {

			case 0x1e00 :
				f = !f;
				y = 1;
				update(headers,a,y);
				break;

			case PGDN:
				i = maxy;
				while ((i > 1) && (a < (arealist[area].messages-1))) {
					i--;
					a++;
				}
				y = 1;
				update(headers,a,y);
				break;

				case PGUP:
					i = maxy;
					while ((i > 1) && (a > 0)) {
						i--;
						a--;
					}
					y = 1;
					update(headers,a,y);
					break;

			case UP:
				if (a > 0) {
					gotoxy(1, y);
					clreol();
					if (strcmpl(headers[y-1].to_name,username) == 0)
						set_color(colors.info);
					else
						set_color(colors.normal);

					showit(&headers[y-1]);
					a--;
					y--;

					if (y < 1) {
						y = 1;
						scrolldown(1, 1, maxx, maxy, 1);
						memmove((headers + 1),headers,(sizeof(MLHEAD) * maxy));
						getheader(msgnum(a), &headers[0]);
						s = strchr(headers[0].fr_name,'\n');
						if (s != NULL)
							*s = '\0';
						s = strchr(headers[0].to_name,'\n');
						if (s != NULL)
							*s = '\0';
					}
				}
				break;

			case DOWN:
				if (a < (arealist[area].messages-1)) {
					gotoxy(1, y);
					clreol();
					if (strcmpl(headers[y-1].to_name,username) == 0)
						set_color(colors.info);
					else
						set_color(colors.normal);

					showit(&headers[y-1]);
					a++;
					y++;

				if (y > maxy) {
					y = maxy;
					scrollup(1, 1, maxx, maxy, 1);
					memmove(headers,(headers + 1),sizeof(MLHEAD) * (maxy+1));
					getheader(msgnum(a), &headers[maxy-1]);
					s = strchr(headers[maxy - 1].fr_name,'\n');
					if (s != NULL)
						*s = '\0';
					s = strchr(headers[maxy - 1].to_name,'\n');
					if (s != NULL)
						*s = '\0';
					}
				}
				break;

				case ENTER:
					arealist[area].current = a;
					dispose(message);
					message = readmsg(messages[a]);
					free(headers);
					inlist = 0;
					return;

				case ABORT:
					free(headers);
					inlist = 0;
					return;

			       default:
					arealist[area].current = a;
					if (ch & 0xff) {
						if (isdigit(ch))
							gotomsg(ch - 0x30);
						else if (mainckeys[ch])
							mainckeys[ch & 0xff]();
					}
					else if (mainakeys[ch >> 8])
						mainakeys[ch >> 8]();

					i = a = arealist[area].current;
					update(headers,i,y=1);
		}
	}
}

static void _near _pascal update(MLHEAD *headers, int i,int y)

{
	char *s = NULL;

		clrwnd(1,y,maxx,maxy);
		while ((i <= (arealist[area].messages-1)) && (y <= maxy)) {
				gotoxy(1,y);
		getheader(msgnum(i), &headers[y-1]);
		s = strchr(headers[y - 1].fr_name,'\n');
		if (s != NULL)
			*s = '\0';
		s = strchr(headers[y - 1].to_name,'\n');
		if (s != NULL)
			*s = '\0';

		if (strcmpl(headers[y-1].to_name,username) == 0)
			set_color(colors.info);
		else
			set_color(colors.normal);

		showit(&headers[y-1]);
				i++;
				y++;
	}

}

static void _near _pascal showit(MLHEAD *h)

{
	char	line[256];

	if (f && arealist[area].netmail)
		sprintf(line,"%4d: %-15.15s@%5d/%-5d to %-15.15s@%5d/%-5d RE: %-70s",
			h->n, h->fr_name,h->fr_net,h->fr_node,
			h->to_name,h->to_net,h->to_node, h->subj);
	else
		sprintf(line,"%4d: %-15.15s to %-15.15s RE: %-70s",
			h->n,h->fr_name,h->to_name,h->subj);
	bprintf(tmp,line);
}

static void _near _pascal getheader(int n, MLHEAD *h)

{
	MSG *i = NULL;

	i = msg_readheader(n);
	memset(h,0,sizeof(MLHEAD));
	if (i == NULL) return;

	h->n = i->msgnum;
	h->to_net = i->to.net;
	h->to_node = i->to.node;
	h->fr_net = i->from.net;
	h->fr_node = i->from.node;
	strncpy(h->subj,i->subj,72);
	strncpy(h->to_name,i->isto,36);
	strncpy(h->fr_name,i->isfrom,36);
	dispose(i);
}


void _cdecl main(int argc, char *argv[])

{
	messages = NULL;
	arealist = NULL;

#ifdef __ZTC__
	brk = dos_get_ctrl_break();
	dos_set_ctrl_break(0);
#endif

	if ((argc > 1) && (argc < 3))
		command = start(argv[1],NULL);
	else if (argc > 1)
		command = start(argv[1],argv[2]);
	else
		command = start(NULL,NULL);

	for (;;) {			/* endless loop... exit is through cleanup */

		if (command & 0xff) {
			if (isdigit(command))
				gotomsg(command - 0x30);
			else if (mainckeys[command])
				mainckeys[command & 0xff]();
		}
		else if (mainakeys[command >> 8])
			mainakeys[command >> 8]();

		if (arealist[area].messages < 1) {
			empty();
			command = getkey();
		}
		else {
			dispose(message);
			message = NULL;

			while ((message = readmsg(msgnum(arealist[area].current))) == NULL)

				if ((direction == RIGHT) && (arealist[area].current < arealist[area].last))
					arealist[area].current++;
				else if (arealist[area].current > 0) {
					arealist[area].current--;
					direction = LEFT;
				}
				else {
					empty();
					command = getkey();
					continue;
				}

			command = showmsg(message);
		}
	}
}
