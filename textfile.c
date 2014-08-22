/*

Title:  MsgEd

File:   textfile.c

Author: Jim Nutt

Copr:	released into the PUBLIC DOMAIN 30 Jul 1990 by jim nutt 

Description:

	handles import and export of textfiles

Revision History:

	0.00	4 July 1988

Support Files:

	msged.h

*/

#include "msged.h"
#include "date.h"
#include "menu.h"
#ifdef __MSC__
#include <sys/types.h>
#endif
#include <sys\stat.h>

#define TEXTLEN 2048

char * _pascal makequote(void);
int  _pascal wrap(LINE *cl, int x, int y);
FILE * _pascal repapp(char *path);

void _pascal import(LINE * l)

{
	char   line[TEXTLEN];
	FILE   *fp;
	static char fn[PATHLEN];
	LINE   *n;

	memset(line, 0, sizeof(line));

	gotoxy(9, 1);
	clreol();
	set_color(colors.info);
	bputs("File to import? ");
	bgets(fn, PATHLEN,PATHLEN);
	set_color(colors.normal);

	if ((fp = fopen(fn, "rt")) != NULL) {
		while (fgets(line, TEXTLEN, fp) != NULL) {
			if (l->text != NULL) {
				if ((n = (LINE *) calloc(1, sizeof(LINE))) == NULL) {
					gotoxy(9, 1);
					clreol();
					bputs("Not enough memory, press any key");
					getkey();
					showheader(message);
					return;
				}
				n->next = l->next;
				if (n->next != NULL)
					n->next->prev = n;
				n->prev = l;
				l->next = n;
				l = n;
			}
			else
				n = l;

			n->text = strdup(line);
			if (strlen(n->text) > (size_t) rm) {
				l = n->next;
				wrap(n,maxx,maxy);
				if (l == NULL)
					while (n->next)
						n = n->next;
				else
					n = l->prev;

				l = n;
			}
			memset(line, 0, sizeof(line));
		}
		fclose(fp);
	}

	showheader(message);
}

void _pascal export(LINE * f)
{
	FILE   *fp;
	char   fn[PATHLEN];

	gotoxy(9, 1);
	clreol();
	set_color(colors.info);
	bputs("File name to write to? ");
	memset(fn, 0, sizeof(fn));
	strcpy(fn, outfile);
	bgets(fn, sizeof(fn),sizeof(fn));
	free(outfile);
	outfile = strdup(fn);
	set_color(colors.normal);

	if (*fn == '+')
		fp = fopen(fn+1,"at");
	else
		fp = fopen(fn,"wt");

	if (fp != NULL) {

		fputc('\n', fp);

		for (; f != NULL; f = f->next)
			if ((f->text != NULL) && ((*(f->text) != '\01') || shownotes)) {
				fputs(f->text, fp);
				if (strchr(f->text, '\n') == NULL)
					fputc('\n', fp);
			}

		fclose(fp);
	}

	showheader(message);
}

void _pascal writetxt()

{
	LINE   *f = message->text;
	FILE   *fp;
	char	fn[PATHLEN];
	static char *modes[] = {"Text", "Quote", "Msged", NULL};
	static char *ovr[] = {"Replace", "Append", NULL}; 
	char   *s;
	char   *l;
	static int mode = 0;

	gotoxy(9, 1);
	clreol();
	set_color(colors.info);
	bputs("File name to write to? ");
	memset(fn, 0, sizeof(fn));
	strcpy(fn, outfile);
	bgets(fn, sizeof(fn),sizeof(fn));
	free(outfile);
	outfile = strdup(fn);
	set_color(colors.normal);

	if ((s = strchr(fn,',')) != NULL)
		*s++ = '\0';

	if (s && (*s == 't')) mode = 0;
	if (s && (*s == 'q')) mode = 1;
	if ((s && (*s == 'm')) || (s == NULL)) mode = 2;

	if (*fn == '?') {
		box(60,1,70,5,1);
		mode = menu(61,2,69,4,modes,colors.hilite,colors.normal,mode);
		clrwnd(60,1,70,5);
		if (mode == -1) {
			mode = 0;
			return;
		}
	}

	if (*fn == '+')
		fp = fopen(fn+1,"at");

	else if (*fn == '?') {
		if ((fp = fopen(fn+1,"rt")) == NULL)
			fp = fopen(fn+1,"wt");
		else if (isatty(fileno(fp))) {
			fclose(fp);
			fp = fopen(fn+1,"wt");
			mode = 0;
		}
		else {	
			int i;
			box(60,1,70,4,1);
			i = menu(61,2,69,3,ovr,colors.hilite,colors.normal,0);
			if (i == -1) {
				clrwnd(60,1,70,4);
				return;
			}
			fclose(fp);
			if (i) 
				fp = fopen(fn+1,"at");
			else
				fp = fopen(fn+1,"wt");
		}
	}
	else
		fp = fopen(fn,"wt");

	if (fp == NULL)
		return;

	if (mode == 0) {
		fprintf(fp, "\nDate:   %s\n", atime(message->timestamp));
		fprintf(fp, "From:   %s", message->isfrom);
		if (arealist[area].netmail)
			fprintf(fp," of %s",show_address(message->from));
		fputc('\n', fp);
		fprintf(fp, "To:     %s", message->isto);
		if (arealist[area].netmail)
			fprintf(fp," of %s",show_address(message->to));
		fputc('\n', fp);
		if (message->attrib.attached)
			fprintf(fp, "Files:  %s", message->subj);
		else
			fprintf(fp, "Subj:   %s", message->subj);
		fputc('\n', fp);
		fputs("Attr:   ", fp);

		if (message->attrib.private)
			fputs("privileged ", fp);
		if (message->attrib.crash)
			fputs("crash ", fp);
		if (message->attrib.recvd)
			fputs("recvd ", fp);
		if (message->attrib.sent)
			fputs("sent ", fp);
		if (message->attrib.attached)
			fputs("f/a ", fp);
		if (message->attrib.killsent)
			fputs("kill/sent ", fp);
		if (message->attrib.freq)
			fputs("freq ", fp);
		if (message->attrib.rreq)
			fputs("rreq ", fp);
		if (message->attrib.areq)
			fputs("areq ", fp);
		if (message->attrib.ureq)
			fputs("ureq ", fp);
		fputc('\n', fp);
		fprintf(fp,"%-30s -------------------------------\n",arealist[area].description);
	}
	else if (mode == 1) {
		l = makequote();
		fputs(l,fp);
		fputc('\n',fp);
		f = message->text;
	}

	for (; f != NULL; f = f->next) {
		if ((f->text != NULL) && ((*(f->text) != '\01') || shownotes)) {
			fputs(f->text, fp);
			if ((strchr(f->text, '\n') == NULL) && ((mode == 0) || (mode == 1)))
				fputc('\n', fp);
		}
	}

	if (isatty(fileno(fp)))
		fputc(12, fp);

	fclose(fp);
}

FILE * _pascal repapp(char *path)

{
	FILE *fp;
	int ch;


	if ((fp = fopen(path,"rt")) == NULL)
		return(fp = fopen(path,"wt"));

	if (isatty(fileno(fp))) {
		fclose(fp);
		return(fp = fopen(path,"wt"));
	}
	fclose(fp);

	gotoxy(9,1);
	clreol();
	set_color(colors.hilite);
	bputc('r');
	set_color(colors.info);
	bputs("eplace or ");
	set_color(colors.hilite);
	bputc('a');
	set_color(colors.info);
        bputs("ppend? ");
        video_update();

	ch = getkey() & 0x7f;
	ch = tolower(ch);
	if (ch == 0x1b)
		return(NULL);

	while ((ch != 'a') && (ch != 'r')) {
		ch = 0x7f & getkey();
		ch = tolower(ch);
		if (ch == 0x1b)
			return(NULL);
	}

	if (ch == 'a')
		fp = fopen(path,"at");
	else
		fp = fopen(path,"wt");

	return(fp);
}
