/*

Title:	MsgEd

File:	MakeMsg.c

Author: Jim Nutt

Copr:	released into the PUBLIC DOMAIN 30 jul 1990 by jim nutt

Description:

	routines to create new messages

Support Files:

	msged.h

*/

#include "msged.h"
#include "date.h"

#define TEXTLEN 256
#define INPLEN	60

static void _near _pascal show_attrib(void);
static int _near _pascal change_from(void);
static int _near _pascal change_attrib(void);
static int _near _pascal change_to(void);
static int _near _pascal change_dest(void);
static int _near _pascal change_orig(void);
static int _near _pascal change_subject(void);
static void _near _pascal clear_attributes(struct _attributes *);
static void _near _pascal createmsg(void);

char * _pascal attrib_line(MSG *m, char *format);
char * _pascal makequote(void);

static	char work[TEXTLEN];

int 	do_lookup = FALSE;

int _pascal editmsg(void);

static void _near _pascal createmsg()

{
	int  *t;

	if (arealist[area].messages == 0) {
		message = (MSG *) calloc(1, sizeof(MSG));
		if (arealist[area].echomail)
			message->msgnum = arealist[area].last = 2;
		else
			message->msgnum = arealist[area].last = 1;
	}
	else {
		clearmsg(message);
		message->msgnum = arealist[area].last = msgnum(arealist[area].messages - 1) + 1;
	}

	if (arealist[area].messages == 0) {
		t = (int *) calloc(1,sizeof(int));
		arealist[area].messages = 1;
	}
	else
		t = (int *) realloc(messages,++arealist[area].messages * sizeof(int));

	if (t == NULL)
		outamemory();

	messages = t;

	messages[arealist[area].messages - 1] = message->msgnum;
	do_lookup = TRUE;
	message->timestamp = time(NULL);
	message->date = NULL;
	message->isfrom = strdup(username);
	memset(&message->to,0,sizeof(message->to));
	message->to.zone = thisnode.zone;
	message->replyfrom = message->replyto = 0;
	message->links = NULL;
	message->replies = 0;
	message->text = (LINE *) NULL;
	message->from = thisnode;
	message->new = 1;
	message->cost = message->times_read = 0;
	message->from.domain = message->to.domain = (char *) NULL;
	if (thisnode.domain) message->from.domain = strdup(thisnode.domain);
	clear_attributes(&message->attrib);
}

void _pascal newmsg()

{
	int t = (arealist[area].messages)?arealist[area].current:0;
	int q = 0;

	createmsg();

	while (!q) {
		if (editheader() == ABORT) {
			if (confirm()) {
				arealist[area].messages = msg_scan(&arealist[area]);
				arealist[area].current = (t)?t:arealist[area].current;
				return;
			}
		}
		else
			q = 1;
	}

	switch (editmsg()) {

		case SAVE:
			save(message);
			break;

		case ABORT:
			arealist[area].messages = msg_scan(&arealist[area]);
			break;
	}

	arealist[area].current = (t)?t:arealist[area].current;
}

void _pascal reply()

{
	char	  *msgto;
	char	  *subject;
	int link,q=0;
	ADDRESS tmp;
	MSG *	hlink = NULL;
	int t = arealist[area].current;

	if (arealist[area].messages == 0) return;

	msgto = strdup(message->isfrom);
	subject = strdup(message->subj);
	tmp = message->from;
	if (tmp.domain)
		tmp.domain = strdup(message->from.domain);
	link = msgnum(t);

	createmsg();

	do_lookup = FALSE;
	message->isto = msgto;
	message->subj = subject;
	message->to = tmp;

	message->replyto = link;

	message->new = 1;

	while (!q) {
		if (editheader() == ABORT) {
			if (confirm()) {
				arealist[area].messages = msg_scan(&arealist[area]);
				arealist[area].current = (t)?t:arealist[area].current;
				return;
			}
		}
		else
			q = 1;
	}

	switch (editmsg()) {

		case SAVE:
			if ((hlink = msg_readheader(link)) != NULL) {
				hlink->replyfrom = message->msgnum;
				msg_writeheader(hlink);
			}
			save(message);
			break;

		case ABORT:
			arealist[area].messages = msg_scan(&arealist[area]);
			break;
	}

	arealist[area].current = t;

	dispose(hlink);
	hlink = NULL;
}

char * _pascal makequote()

{
	int 	i,n;
	char	*qs;
	char	*s;
	char	*l = NULL;
	LINE	*t;
	char	initial[10];
	char    line2[256];

	i = rm;
	rm = qm;
	n = msgnum(arealist[area].current);

	if (attribline)
		l = attrib_line(message,attribline);
	else
		l = NULL;

	dispose(message);
	message = NULL;
	message = readmsg(n);
	rm = i;

	i = 0;
	s = message->isfrom;
	while (s && *s && (i < 10)) {
		while (*s && isspace(*s)) s++;
		initial[i++] = *s;
		while (*s && !isspace(*s)) s++;
	}
	initial[i] = '\0';
	if ((s = strchr(quotestr,'&')) == NULL)
		qs = strdup(quotestr);
	else {
		qs = malloc(strlen(quotestr) + strlen(initial) + 1);
		*s = '\0';
		strcpy(qs,quotestr);
		strcat(qs,initial);
		strcat(qs,s + 1);
		*s = '&';
	}

	s = qs;
	while ((s = strchr(s,'*')) != NULL)
		if (initial[0])
			*s = initial[0];
		else
			strdel(s,1);

	s = qs;
	while ((s = strchr(s,'^')) != NULL) 
		if (initial[1])
			*s = initial[1];
		else
			strdel(s,1);

	t = message->text;
	while (t) {
		if (!t->quote) {
			if (t->text && (*(t->text) != '\n')) {
				strcpy(line2," ");
				strcat(line2,qs);
				if (t->text != NULL) {
					strcat(line2,t->text);
					if (strchr(t->text,'\n') == NULL) {
						strcat(line2,"\n");
					}
					free(t->text);
				}
				t->text = strdup(line2);
			}
			t->quote = 1;
		}
		else {
			strcpy(line2,t->text);
			if ((t->next) && (t->next->text) && (*(t->next->text) != '\n') && !strchr(line2,'\n')) {
				strcat(line2,t->next->text);
				free(t->next->text);
				t->next = t->next->next;
				free(t->next->prev);
				t->next->prev = t;
				free(t->text);
				t->text = strdup(line2);
			}
		}
		t = t->next;
		while (t && t->text && ((strlen(t->text) == 0))) {
			LINE *t2 = t->next;
			if (t->next)
				t->next->prev = t->prev;
			if (t->prev)
				t->prev->next = t->next;
			free(t->text);
			free(t);
			t = t2;
		}
	}

	free(qs);
	return(l);
}


void _pascal quote()
{
	int 	*msgs,q=0;
	int 	tm = arealist[area].current;
	int 	link = msgnum(arealist[area].current);
	MSG    *hlink = NULL;
	char	*t,*l,*s,c;
	LINE	*n;

	if (arealist[area].messages == 0) return;

	l = makequote();

	release(message->isto);
	message->isto = message->isfrom;
	message->to = message->from;
	message->from = thisnode;
	if (message->from.domain)
		message->from.domain = strdup(thisnode.domain);

	if (arealist[area].messages == 0) {
		if (arealist[area].echomail)
			message->msgnum = arealist[area].last = 2;
		else
			message->msgnum = arealist[area].last = 1;
	}
	else
		message->msgnum = arealist[area].last = msgnum(arealist[area].messages - 1) + 1;

	msgs = realloc(messages,++arealist[area].messages*sizeof(int));

	if (msgs == NULL)
		outamemory();

	messages = msgs;

	messages[arealist[area].messages - 1] = message->msgnum;
	message->isfrom = strdup(username);
	message->timestamp = time(NULL);

	clear_attributes(&message->attrib);
	do_lookup = FALSE;

	message->replyto = link;

	message->new = 1;

	if (l) {
		message->text = insline(message->text);
		message->text->text = strdup("\n");
		message->text = insline(message->text);
		t = s = l; l = strchr(s,'\n'); n = message->text;
		if (l != NULL) {
			l++; c = *l; *l = '\0';
			n->text = strdup(s);
			*l = c;
			while (l && *l) {
				s = l; l = strchr(s,'\n'); 
				n = insline(n->next);
				if (l) {
					l++; c = *l; *l = '\0';
					n->text = strdup(s);
					*l = c;
				}
			}
			free(t);
		}
		else
			message->text->text = s;
	}

	while (!q) {
		if (editheader() == ABORT) {
			if (confirm()) {
				arealist[area].messages = msg_scan(&arealist[area]);
				arealist[area].current = (tm)?tm:arealist[area].current;
				return;
			}
		}
		else
			q = 1;
	}

	switch (editmsg()) {

		case SAVE:
			if ((hlink = msg_readheader(link)) != NULL) {
				hlink->replyfrom = message->msgnum;
				msg_writeheader(hlink);
			}
			save(message);
			break;

		case ABORT:
			arealist[area].messages = msg_scan(&arealist[area]);
			break;
	}

	arealist[area].current = tm;
	dispose(hlink);
	hlink = NULL;
}

void _pascal change()

{	
	int q = 0;

	if (arealist[area].messages == 0) return;

	message->attrib.sent = 0;
	message->attrib.orphan = 0;
	message->timestamp = time(NULL);

	do_lookup = FALSE;

	while (!q) {
		if (editheader() == ABORT) {
			if (confirm()) {
				arealist[area].messages = msg_scan(&arealist[area]);
				return;
			}
		}
		else
			q = 1;
	}

	switch (editmsg()) {

		case SAVE:
			writemsg(message);
			break;

		case ABORT:
			break;
	}
}

static void _near _pascal show_attrib()

{
	gotoxy(9, 5);
	set_color((message->attrib.private)?colors.hilite:colors.normal);
	bputs("Privileged ");
	set_color((message->attrib.crash)?colors.hilite:colors.normal);
	bputs("Crash ");
	set_color((message->attrib.attached)?colors.hilite:colors.normal);
	bputs("Attach ");
	set_color((message->attrib.freq)?colors.hilite:colors.normal);
	bputs("Request ");
	set_color((message->attrib.ureq)?colors.hilite:colors.normal);
	bputs("Update ");
	set_color((message->attrib.killsent)?colors.hilite:colors.normal);
	bputs("Kill/sent ");
	set_color((message->attrib.hold)?colors.hilite:colors.normal);
	bputs("Hold ");
	set_color((message->attrib.direct)?colors.hilite:colors.normal);
	bputs("Direct ");
	set_color(colors.normal);
	gotoxy(1, 5);
	bputs("Attrib: ");
}

static int _near _pascal change_attrib()

{
	int 	ch;

	set_color(colors.hilite);
	gotoxy(1, 5);
	video_update();
	bputs("Attrib: ");
	set_color(colors.info);
	ch = getkey();
	message->attrib.private ^= (toupper((ch & 0xff)) == 'P');
	message->attrib.crash ^= (toupper((ch & 0xff)) == 'C');
	message->attrib.attached ^= (toupper((ch & 0xff)) == 'A');
	message->attrib.ureq ^= (toupper((ch & 0xff)) == 'U');
	message->attrib.freq ^= (toupper((ch & 0xff)) == 'R');
	message->attrib.killsent ^= (toupper((ch & 0xff)) == 'K');
	message->attrib.hold ^= (toupper((ch & 0xff)) == 'H');
	message->attrib.direct ^= (toupper((ch & 0xff)) == 'D');
	return (ch);
}

static int _near _pascal change_from()

{
	int 	ch;
	char	tmp[70];
	char	*t;

	set_color(colors.hilite);
	gotoxy(1, 2);
	bputs("From:   ");
	set_color(colors.normal);
	clreol();

	if (message->isfrom != NULL) {
		strncpy(tmp,message->isfrom, sizeof tmp - 1);
		free(message->isfrom);
	}
	else
		memset(tmp,0,sizeof tmp);

	if (message->from.internet)
		strncpy(tmp,message->from.domain,sizeof tmp - 1);
	else if (message->from.bangpath)
		strncat(strcpy(tmp,"@"),message->from.domain,sizeof tmp - 2);

	ch = bgets(tmp,sizeof(tmp)-1,maxx - 9);

	message->from.notfound = 0;

	if (((t = strchr(tmp,'@')) != NULL) &&
		(arealist[area].news || arealist[area].uucp)) {
		message->from.fidonet = 0;
		if (t == tmp) {
			t++;
			message->from.bangpath = 1;
		}
		else {
			message->from.internet = 1;
			t = tmp;
		}
		release(message->from.domain);
		message->from.domain = strdup(t);
		release(message->isfrom);
	}
	else {
		message->isfrom = strdup(tmp);
		if (message->from.internet || message->from.bangpath) {
			message->from = thisnode;
			if (thisnode.domain)
				message->from.domain = strdup(thisnode.domain);
			else
				message->from.domain = NULL;
		}
		message->from.internet = 0;
		message->from.bangpath = 0;
		message->from.fidonet = 1;
	}

	gotoxy(1, 2);
	set_color(colors.info);
	bputs("From:   ");
	return (ch);
}

static int _near _pascal change_orig()

{
	int 	ch;
	char	tmp[TEXTLEN];

	gotoxy(9 + strlen(message->isfrom), 2);
	set_color(colors.hilite);
	bputs(" of ");
	set_color(colors.normal);
	strncpy(tmp,show_address(message->from),TEXTLEN);
	ch = bgets(tmp, sizeof tmp - 1, maxx - (12 + strlen(message->isfrom)));
	message->from = parsenode(tmp);
	gotoxy(9 + strlen(message->isfrom), 2);
	set_color(colors.info);
	bputs(" of ");
	return (ch);
}

static int _near _pascal change_to()

{
	int 	  ch;
	char	  tmp[70];
	char	  *tmp2;
	char	  *t;

	set_color(colors.hilite);
	gotoxy(1, 3);
	bputs("To:     ");
	set_color(colors.normal);
	clreol();
	tmp2 = message->isto;

	if (message->isto != NULL)
		strncpy(tmp,message->isto, sizeof tmp - 1);
	else
		memset(tmp,0,sizeof tmp);

	if (message->to.internet)
		strncpy(tmp,message->to.domain,sizeof tmp - 1);
	else if (message->to.bangpath)
		strncat(strcpy(tmp,"@"),message->to.domain,sizeof tmp - 2);

	ch = bgets(tmp, sizeof(tmp) - 1, maxx - 9);

	message->to.notfound = 0;

	if (((t = strchr(tmp,'@')) != NULL) &&
		 (arealist[area].news || arealist[area].uucp)) {
		message->to.fidonet = 0;
		if (t == tmp) {
			t++;
			message->to.bangpath = 1;
		}
		else {
			message->to.internet = 1;
			t = tmp;
		}

		release(message->to.domain);
		message->to.domain = strdup(t);
		release(message->isto);
		do_lookup = 0;
	}
	else {
		message->to.fidonet = 1;
		message->to.internet = 0;
		message->to.bangpath = 0;
		do_lookup = (tmp != NULL)?((tmp2 != NULL)?abs((strcmpl(tmp2,tmp))):1):0; /*WRA*/
		message->isto = (tmp != NULL)?strdup(tmp):NULL;
/*		do_lookup = (tmp2 == NULL) || !(strcmpl(tmp2,tmp) == 0); */
	}

	gotoxy(1, 3);
	set_color(colors.info);
	bputs("To:     ");
	if ((arealist[area].netmail) && do_lookup && (fidolist != NULL)) {
		message->to = lookup(message->isto,fidolist);
		if ((message->to.notfound) && (userlist != NULL))
			message->to = lookup(message->isto,userlist);
	}
	return (ch);
}

static int _near _pascal change_dest()

{
	int 	ch;
	char	tmp[TEXTLEN];

	gotoxy(9 + strlen(message->isto), 3);
	set_color(colors.hilite);
	bputs(" of ");
	set_color((message->to.notfound)?colors.warn:colors.normal);
	strncpy(tmp,show_address(message->to),TEXTLEN);

	ch = bgets(tmp, sizeof tmp - 1, maxx - (12 + strlen(message->isto)));

	message->to = parsenode(tmp);
	if ((message->to.domain != NULL) && (thisnode.domain != NULL))
		message->from.domain = strdup(thisnode.domain);

	gotoxy(9 + strlen(message->isto), 3);
	set_color(colors.info);
	bputs(" of ");
	return (ch);
}

static int _near _pascal change_subject()

{
	int 	ch;
	char	tmp[73];

	set_color(colors.hilite);
	gotoxy(1, 4);
	bputs(((message->attrib.attached) ||
		  (message->attrib.freq) ||
		  (message->attrib.ureq))?"Files:  ":"Subj:   ");
	set_color(colors.normal);
	if (message->subj != NULL) {
		strncpy(tmp,message->subj, sizeof tmp - 1);
		free(message->subj);
	}
	else
		memset(tmp,0,sizeof tmp);

	ch = bgets(tmp,sizeof tmp - 1, maxx - 9);
	message->subj = strdup(tmp);
	if ((*(tmp+1) == ':') && (*(tmp+2) == '\\')) {
		message->attrib.attached = 1;
		show_attrib();
	}
	gotoxy(1, 4);
	set_color(colors.info);
	bputs(((message->attrib.attached) ||
		  (message->attrib.freq) ||
		  (message->attrib.ureq))?"Files:  ":"Subj:   ");
	return (ch);
}

int _pascal editheader()

{
	int 	field = 2;
	int 	ch = 0;

	cls();
	showheader(message);
	show_attrib();
	for (ch = 1; ch < maxx; ch++) {
		   gotoxy(ch,6);
		   bputc('_');
		}
	gotoxy(1, 6);
	set_color(colors.hilite);
	bputs(arealist[area].description);
	set_color(colors.normal);

	while (ch != DONE) {
		switch (field) {
		case 0:
			ch = change_from();
			break;
		case 1:
			if (arealist[area].netmail ||
			   (arealist[area].uucp && message->from.fidonet))
				ch = change_orig();
			break;
		case 2:
			if (arealist[area].news) {
				if (message->isto) free(message->isto);
				message->isto = strdup("All");
				message->to = uucp_gate;
				if (uucp_gate.domain)
					message->to.domain = strdup(uucp_gate.domain);
				ch = DOWN;
				break;
			}
			ch = change_to();
			break;
		case 3:
			if (arealist[area].netmail && message->to.fidonet)
				ch = change_dest();
			break;
		case 4:
			ch = change_subject();
			break;
		case 5:
			ch = change_attrib();
			show_attrib();
			break;
		}

		if (ch == ABORT)
			return(ABORT);

		if (ch == UP) {

			field--;

			if (field < 0)
				field = 5;

		}

		if ((ch == DOWN) || (ch == ENTER)) {

			if ((field == 5) && (ch == ENTER))
				break;

			field++;

			if (field > 5)
				field = 0;

			continue;
		}
	}
	showheader(message);
	return(0);
}

static void _near _pascal clear_attributes(struct _attributes * h)

{
	h->recvd = h->sent = h->attached = h->forward = h->orphan = h->freq =
	h->rreq = h->rcpt = h->areq = h->ureq = 0;

	h->crash	 = arealist[area].crash;
		h->private	 = (h->private)?h->private:arealist[area].priv;
		h->killsent  = arealist[area].killsent;
		h->hold 	 = arealist[area].hold;
		h->direct	 = arealist[area].direct;
		h->local	 = 1;
}

void _pascal save(MSG *m)

{
	LINE *current;
	char *s,*t;
	char *name;

	writemsg(m);
	current = m->text;
	s = current->text;
	while (isspace(*s))
		s++;

	if (tolower(*s) != 'c')
		return;

	s++;
	if (tolower(*s) != 'c')
		return;

	s++;
	if (*s != ':')
		return;
	s++;

	m->text = insline(m->text);
	m->text->text = strdup("\n");
	sprintf(work," * Original to %s of %s\n",m->isto,show_address(m->to));
	m->text = insline(m->text);
	m->text->text = strdup(work);
	m->attrib.killsent = 1;

	while ((s != NULL)	&& (*s != '\0') && isspace(*s))
		s++;

	while ((s != NULL) && (*s != '\0')) {

		name = strdup(s);

		/* strip trailing white space */

		t = strchr(name,'\n');
		if (t == NULL)
			break;

		while (isspace(*t) && (t > name))
			*t-- = '\0';

		memset(&(m->to),0,sizeof(ADDRESS));
		t = strrchr(name,' ');
		s = strchr(name,' ');
		if ((t != NULL)) {
			while (isspace(*s)) s++;
			while (isspace(*t)) t++;
			if ((s == t) && isdigit(*s)) {
				m->to = parsenode(s);
				if (!m->to.notfound) {
					while (isspace(*s--)) ;
					*s = '\0';
				}
			}
			else if (isdigit(*t)) {
				m->to = parsenode(t);
				if (!m->to.notfound) {
					while (isspace(*t--)) ;
					*t = '\0';
				}
			}
			else {
				m->to = thisnode;
				if (m->to.domain != NULL)
					m->to.domain = strdup(m->to.domain);
				m->to.notfound = 1;
			}
		}
		if ((fidolist != NULL) && (m->to.notfound)) {
			if (m->to.domain != NULL)
				free(m->to.domain);
			m->to = lookup(name,fidolist);
			if ((m->to.notfound) && (userlist != NULL)) {
				m->to = lookup(name,userlist);
				if (m->to.notfound) {
					m->to = thisnode;
					if (m->to.domain != NULL)
						m->to.domain = strdup(m->to.domain);
					m->to.notfound = 1;
				}
			}
		}

		if (m->isto != NULL) {
			free(m->isto);
			m->isto = NULL;
		}
		for (t = name + strlen(name) - 1; isspace(*t); t--)
			*t = '\0';
		m->isto = name;
		++m->msgnum;
		writemsg(m);
		current = current->next;

		/* strip leading white space */

		s = (current == NULL) ? NULL : current->text;

		while ((s != NULL) && (*s != '\0') && isspace(*s))
			s++;
	}
	arealist[area].messages = msg_scan((&arealist[area]));
}
