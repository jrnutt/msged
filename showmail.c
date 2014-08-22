/*

Title:	MsgEd

File:	Showmail.c

Author: Jim Nutt

Copr:	released into the PUBLIC DOMAIN 30 jul 1990 by jim nutt

Description:

	routines to display messages

*/

#include "msged.h"
#include "date.h"

LINE   *top = NULL, *bottom = NULL;

extern LINE *anchorpt;

static void _pascal up(void);
static void _pascal down(void);
static void _pascal pgdown(void);
static void _pascal pgup(void);

void _pascal refresh(LINE * c, int y);
extern void _pascal putl(LINE *l);

int _pascal showmsg(MSG *m)

{
	int 	command;

	showheader(m);

	refresh(m->text, 1);

	for (;;) {
		gotoxy(1,1);
		video_update();
		switch (command = getkey()) {

			case PGUP:
				if (m->text != NULL)
				pgup();
				break;

			case UP:
				if (m->text != NULL)
				up();
				break;

			case SPACE:
			case PGDN:
				if (m->text != NULL)
				pgdown();
				break;

			case DOWN:
				if (m->text != NULL)
				down();
				break;

			default:
				return (command);
		}
	}
}

void _pascal showheader(MSG *m)

{
	int i;

	set_color(colors.info);

	clrwnd(1, 1, maxx, 6);

	gotoxy(1, 1);

	bprintf("%03d/%03d ", arealist[area].current+1, arealist[area].messages);
	if (m->timestamp) bputs(atime(m->timestamp));
	else bputs(m->date);

	if (m->replyto)
		bprintf("  Reply to #%d", m->replyto);

	if (m->replyfrom)
		bprintf("  See #%d", m->replyfrom);

	gotoxy(1, 2);
	bputs("From:   ");
	set_color(colors.normal);

	if (m->from.fidonet) {
		bputs(m->isfrom);
		set_color(colors.info);
		bputs(" of ");
		set_color((m->from.notfound)?colors.warn:colors.normal);
		bputs(show_address(m->from));
	}
	else {
		bputs(show_address(m->from));
		if (m->isfrom)
			bprintf(" (%s)",m->isfrom);
	}

	set_color(colors.info);
	gotoxy(1, 3);

	bputs("To:     ");

	if (!(m->isto == NULL)) {							   /*WRA*/
		if (strcmpl(username,m->isto)==0)
			set_color(colors.hilite);
		else
			set_color(colors.normal);

		if (m->to.fidonet) {
			bputs(m->isto);
			if (arealist[area].netmail) {
				set_color(colors.info);
				bputs(" of ");
				set_color((m->to.notfound)?colors.warn:colors.normal);
				bputs(show_address(m->to));
			}
		}
		else {
			bputs(show_address(m->to));
			if (m->isto)
				bprintf(" (%s)",m->isto);
		}
	}													   /*WRA*/

	set_color(colors.info);
	gotoxy(1, 4);

	if (m->attrib.attached)
		bputs("Files:  ");
	else
		bputs("Subj:   ");

	set_color(colors.normal);
	bputs(m->subj);

	gotoxy(1, 5);

	set_color(colors.info);
	bputs("Attr:   ");

	set_color(colors.normal);
	if (m->attrib.private)
		bputs("privileged ");
	if (m->attrib.crash)
		bputs("crash ");
	if (m->attrib.recvd)
		bputs("recvd ");
	if (m->attrib.sent)
		bputs("sent ");
	if (m->attrib.attached)
		bputs("f/a ");
	if (m->attrib.killsent)
		bputs("kill/sent ");
	if (m->attrib.freq)
		bputs("freq ");
	if (m->attrib.ureq)
		bputs("ureq ");
	if (m->attrib.hold)
		bputs("hold ");
	if (m->attrib.orphan)
		bputs("orphan ");
	if (m->attrib.forward)
		bputs("in transit ");
	if (m->attrib.local)
		bputs("local ");
	if (m->attrib.direct)
		bputs("direct ");
	if (m->attrib.rreq)
		bputs("rreq ");
	if (m->attrib.rcpt)
		bputs("rcpt ");
	if (m->attrib.areq)
		bputs("areq ");

	for (i = 1; i < maxx; i++) {
		   gotoxy(i,6);
		   bputc('_');
	}

	gotoxy(1, 6);
	set_color(colors.hilite);
	bprintf ("%s (%d)",arealist[area].description,m->msgnum);
	set_color(colors.normal);
}

static void _pascal up()

{
		int 	i = 1;

	while (top->prev) {
		top = top->prev;
		if (shownotes || (*(top->text) != '\01')) {
			scrolldown(1, 7, maxx, maxy, 1);
			gotoxy(1, 7);
			putl(top);
			break;
		}
	}

	for (bottom = top;
		 (bottom->next != NULL) && (i < (maxy - 6));
		 bottom = bottom->next)
		if ((*(bottom->text) != '\01') || shownotes)
			i++;
}

static void _pascal down()

{
		int 	i = 1;

	while (bottom->next) {
		bottom = bottom->next;
		if (shownotes || (*(bottom->text) != '\01')) {
			scrollup(1, 7, maxx, maxy, 1);
			gotoxy(1, maxy);
			putl(bottom);
			break;
		}
	}

	for (top = bottom; (top->prev != NULL) && (i < (maxy - 6)); top = top->prev)
		if ((*(top->text) != '\01') || shownotes)
			i++;
}

static void _pascal pgup()

{
	int i = 0;

	if ((top->prev == NULL) || ((*(top->prev->text) == 1) && !shownotes))
		return;

	for (bottom = top; (top->prev != NULL) && (i < (maxy - 6)); top = top->prev)
		if ((*(top->text) != '\01') || shownotes)
			i++;

	refresh(top, 1);
}

static void _pascal pgdown()

{
	int 	i = 1;

	if ((bottom->next == NULL) || ((*(bottom->next->text) == 1) && !shownotes))
		return;

	clrwnd(1, 7, maxx, maxy);

	for (top = bottom; (bottom->next != NULL) && (i < (maxy - 6)); bottom = bottom->next) {
		if ((*(bottom->text) != '\01') || shownotes) {
			gotoxy(1, 6 + i++);
			putl(bottom);
		}
	}
	if ((*(bottom->text) != '\01') || shownotes) {
		gotoxy(1, 6 + i);
		putl(bottom);
	}
}

void _pascal refresh(LINE * c, int y)

{
	int 	i = y;

	top = bottom = c;
	clrwnd(1, (i + 6), maxx, maxy);

	if ((top == NULL) || (top->text == NULL))
		return;

	while ((top != NULL) && ((*top->text == '\01') && !shownotes))
		top = top->next;

	if (top == NULL)
		return;

		for (bottom = top; (bottom->next != NULL) && (i < (maxy - 6)); bottom = bottom->next) {
		if ((*(bottom->text) != '\01') || shownotes) {
			gotoxy(1, 6 + i++);
			putl(bottom);
		}
	}

	if ((*(bottom->text) != '\01') || shownotes) {
		gotoxy(1, 6 + i);
		putl(bottom);
	}
}


