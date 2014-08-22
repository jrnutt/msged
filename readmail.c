/*

Title:  MsgEd

File:   readmail.c

Author: Jim Nutt

Copr:	released into the PUBLIC DOMAIN 30 Jul 1990 by jim nutt

Description:

	handles high level message i/o

*/

#ifdef __MSC__
#include <sys\types.h>
#endif

#ifdef __OS2__
#define INCL_DOSFILEMGR
#include <fcntl.h>
#endif

#if defined(__TURBOC__)
#include <dir.h>
#else
#include <direct.h>
#endif
#include <sys\stat.h>
#include <dos.h>

#include "msged.h"
#include "date.h"
#include "bmg.h"

int _pascal setcwd(char *path);

#define TEXTLEN 96

#define ASM

static int count;

void _pascal normalize(char *s);
int  _pascal wrap(LINE *cl, int x, int y);
static void _pascal checkrecvd(MSG * m);

#ifdef __OS2__
extern unsigned short _pascal far DosSelectDisk(unsigned short);
#endif

LINE * _pascal clearbuffer(LINE *buffer)

{
	if (!buffer) return NULL;

	if (buffer->next) {
		buffer->next->prev = NULL;
		clearbuffer(buffer->next);
		buffer->next = NULL;
	}

	else if (buffer->prev) {
		buffer->prev->next = NULL;
		clearbuffer(buffer->prev);
		buffer->prev = NULL;
	}

	if (buffer->text) free(buffer->text);

	free(buffer);

	return NULL;
}

MSG  * _pascal readmsg(int n)

{
	MSG  *m;
	char *text;
	LINE *l = NULL;
	int  af = 0;
	int  dmn = 0;
	char *t;
	char tmp[128];
	int  i,j;
	int  blank = 0;

	if ((m = msg_readheader(n)) == NULL)
		return(NULL);

	m->new = 0;
	m->change = 0;

	while ((text = msg_readtext(n)) != NULL) {

		if (*text == '\n') blank = 1;

		if ((*text == '\01') && !blank) {
			char *s = NULL;

			switch (*(text+1)) {
				case 'M' :
					if (strncmp(text+1,"MSGID",5) != 0) break;
					if ((s=strchr(text,':')) != NULL) {
						s++;
						while (isspace(*s)) s++;
						m->msgid = strdup(s);
						if (strchr(m->msgid,'\n'))
							*strchr(m->msgid,'\n') = '\0';
						m->from = parsenode(s);
						if (!m->from.notfound)
							af = 1;
					}
					break;

				case 'R' :
					if (strncmp(text+1,"REPLY",5) != 0) break;
					if ((s=strchr(text,':')) != NULL) {
						s++;
						while (isspace(*s)) s++;
						m->reply = strdup(s);
						if (strchr(m->reply,'\n'))
							*strchr(m->reply,'\n') = '\0';
					}
					break;

				case 'F' :
					if (strncmp(text+1,"FMPT",4) != 0) break;
					if (!af && ((s=strchr(text,' ')) != NULL))
						m->from.point = atoi(s+1);
					break;

				case 'T' :
					if (strncmp(text+1,"TOPT",4) != 0) break;
					if ((s=strchr(text,' ')) != NULL)
						m->to.point = atoi(s+1);
					break;

				case 'D' :
					if (strncmp(text+1,"DOMAIN",6) != 0) break;
					if ((s=strchr(text,' ')) != NULL) {
						dmn = 1;
						i = m->to.point;
						j = m->from.point;
						while(isspace(*s)) s++;
						i = 0;
						while(!isspace(*s) && (i < 8))
							tmp[i++] = *s++;
						tmp[i] = '\0';
						while (!isspace(*s)) s++;
						while(isspace(*s)) s++;
						m->to = parsenode(s);
						m->to.domain = strdup(strupr(tmp));
						if (!af) {
							while (!isspace(*s)) s++;
							while(isspace(*s)) s++;
							i = 0;
							while(!isspace(*s) && (i < 8))
								tmp[i++] = *s++;
							tmp[i] = '\0';
							while (!isspace(*s)) s++;
							while(isspace(*s)) s++;
							m->from = parsenode(s);
							m->from.domain = strdup(strupr(tmp));
							m->from.point = j;
							m->to.point = i;
						}
					}
					break;

				case 'I' :
					if (strncmp(text+1,"INTL",4) != 0) break;
					if (((s=strchr(text,' ')) != NULL) && !dmn) {
						i = m->to.point;
						j = m->from.point;
						strcpy(tmp,s+1);
						m->to = parsenode(strtok(tmp," "));
						if (!af) {
							m->from = parsenode(strtok(NULL," \n\r\0"));
							m->from.point = j;
							m->to.point = i;
						}
					}
					break;
			}

			if (!shownotes)
				continue;
		}

		if (arealist[area].echomail) {
			if (*text == 'S') {
				if ((strncmp(text,"SEEN-BY:",8) == 0) && (!seenbys)) {
					free(text);
					continue;
				}
			}
			else if (!af && (*(text+1) == '*')) {
				if (*(text+3) == 'O') { /* probably the origin line */
					char *t = strrchr(text,'(');
					if (t != NULL) {
						char *e = strchr(t,')');
						char c;
						if (e != NULL) {
							c = *e;
							*e = '\0';
						}
						while (!isdigit(*t) && (*t != ')')) t++;
						if (isdigit(*t))
							m->from = parsenode(t);
						if (e != NULL)
							*e = c;
					}
					else
						m->from.notfound = 1;
				}
			}
		}

		if ((arealist[area].uucp || arealist[area].news) && !blank) {
			char *s;

			if (arealist[area].uucp) {
				if (strncmp(text,"To:",3) == 0) {
					s = strchr(text,' ');
					m->to.fidonet = 0;
					m->to.internet = 0;
					m->to.bangpath = 0;
					m->to.notfound = 0;
					while (isspace(*s)) s++;
					if (strchr(s,'@') != NULL)
						m->to.internet = 1;
					else
						m->to.bangpath = 1;
					m->to.domain = strdup(s);
					if (!shownotes) continue;
				}
			}

			if (strncmp(text,"From:",5) == 0) {
				s = strrchr(text,'(');
				if (s == NULL) {
					if (m->isfrom) free(m->isfrom);
					m->isfrom = strdup("UUCP");
				}
				else {
					*s = '\0';
					if ((t = strrchr(s+1,')')) != NULL)
						*t = '\0';
					m->isfrom = strdup(s+1);
				}
				s = strchr(text,' ') + 1;
				m->from.fidonet = 0;
				m->from.internet = 0;
				m->from.bangpath = 0;
				m->from.notfound = 0;
				while (isspace(*s)) s++;
				if (strchr(s,'@') != NULL)
					m->from.internet = 1;
				else
					m->from.bangpath = 1;
				m->from.domain = strdup(s);
				if (!shownotes) continue;
			}

			if (strncmp(text,"Date:",5) == 0) {
				s = strchr(text,' ');
				if (s != NULL) {
					while (isspace(*s)) s++;
					m->timestamp = parsedate(s);
					if (m->date) free(m->date);
					m->date = strdup(s);
				}
				if (!shownotes) continue;
			}
		}

		if ((*text != '\01') || shownotes) {

			if (l == NULL) {
				l = (LINE *) malloc(sizeof(LINE));
			        m->text = l;
				l->next = l->prev = NULL;
		        }
		        else {
				l->next = (LINE *) malloc(sizeof(LINE));
				if (l->next == NULL) {
				        free(text);
				        break;
			        }
				l->next->next = NULL;
			        l->next->prev = l;
			        l = l->next;
		        }

			l->text = text;
			l->len = strlen(text);
			l->alloced = l->len;

			l->hide = (*text == '\x01');
			l->column = 0;

			if (((t = strchr(text,'>')) != NULL) && ((t - text) < 5)) l->quote = 1;
			else l->quote = 0;

		        l->block = 0;

			if ((*text != '\01') && (*text != '\n') && (l->len > (size_t) rm))        {
			        wrap(l,1,maxy);
			        while (l->next)
				        l = l->next;
		        }
		}
	}

	checkrecvd(m);
	return (m);
}

static void _pascal checkrecvd(MSG * m)

{
	if (m->attrib.recvd)
		return;

	m->times_read++;

	if (strcmpl(username, m->isto) == 0) {
		m->attrib.recvd = 1;
		msg_writeheader(m);
	}
}

void _pascal clearmsg(MSG *m)

{
	m->msgnum = 0;
	if (m->reply) free(m->reply);
	if (m->msgid) free(m->msgid);
	if (m->isfrom) free(m->isfrom);
	if (m->isto) free(m->isto);
	if (m->subj) free(m->subj);
	m->reply = m->msgid = m->isfrom = m->isto = m->subj = NULL;
	m->replyto = 0;
	m->replyfrom = 0;
	m->timestamp = 0;
	if (m->date) free(m->date);
	memset(&m->attrib,0,sizeof m->attrib);
	m->attrib.private = arealist[area].priv;
	m->attrib.crash = arealist[area].crash;
	m->attrib.hold = arealist[area].hold;
	m->attrib.direct = arealist[area].direct;
	m->attrib.killsent = arealist[area].killsent;
	m->attrib.local = 1;
	if (m->to.domain != NULL) free(m->to.domain);
	memset(&m->to, 0, sizeof m->to);
	if (m->from.domain != NULL) free(m->from.domain);
	memset(&m->from, 0, sizeof m->from);
	m->text = clearbuffer(m->text);
}

#ifndef ASM

void _pascal normalize(char *s)

{
	char   *tmp = s;

	while (*s)
		if ((unsigned) (0xff & *s) == (unsigned) 0x8d)
			s++;
		else if (*s == 0x0a)
			s++;
		else if (*s == 0x0d)
			s++, *tmp++ = '\n';
		else {
			*tmp++ = (char) DOROT13((int) *s);
			s++;
		}
	*tmp = '\0';
}

#endif

int _pascal setcwd(char *path)

{
	char *p;

	if ((p = strchr(path,':')) == NULL)
		p = path;

	if (*p == ':') {
		p++;
#ifdef __OS2__
		(void) DosSelectDisk((USHORT)(toupper(*path) - 'A'+1));
#else
		bdos(14,toupper(*path) - 'A',0);
#endif
	}
	return(chdir(p));
}

int _pascal writemsg(MSG *m)

{
	char    path[PATHLEN];
	LINE   *l;
	FILE   *fp;
	ADDRESS tmpto, tmpfrom;
	char    corigin[TEXTLEN];
	char    buf[BLOCKLEN+1];
	int     i,n = m->msgnum;
	int     newtear = 1;
	time_t  now = time(NULL);
	int     inquote;
	char   *uucp_to = NULL, *uucp_fr = NULL;

	count++;
	if (!override && arealist[area].echomail) {
		memset(corigin,0,sizeof corigin);
		sprintf(path,"%s\\origin",arealist[area].path);

		if ((fp = fopen(path,"rt")) != NULL) {
			fgets(corigin,sizeof(corigin),fp);
			fclose(fp);
		}
		else if (origin != NULL)
			strcpy(corigin,origin);
		else
			strcpy(corigin,username);
	}
	else
		strcpy(corigin,origin);

	if (strchr(corigin,'\n'))
		*strchr(corigin,'\n') = '\0';

	/* if this is a uucp message, replace the message address with that
	   of the nearest uucp gateway
	*/

	if (m->to.internet || m->to.bangpath) {
		uucp_to = m->to.domain;
		m->to = uucp_gate;
		if (uucp_gate.domain)
			m->to.domain = strdup(uucp_gate.domain);
		if (!arealist[area].news)
			m->isto = strdup("UUCP");
		else
			m->isto = strdup("All");
	}

	if (m->from.internet || m->from.bangpath) {
		uucp_fr = m->from.domain;
		m->from = uucp_gate;
		if (uucp_gate.domain)
			m->from.domain = strdup(uucp_gate.domain);
		m->isfrom = strdup("UUCP");
	}

	/* remap the message header */
	
	/* save the unmapped source and destination of the message */

	tmpto = m->to;
	tmpfrom = m->from;


	/* map the address by alias if possible */

	if ((aliascount > 1) && (thisnode.zone != m->to.zone)) {
		for (i = 1; i < aliascount; i++)
			if (alias[i].zone == m->to.zone)
				break;
		if (alias[i].zone == m->to.zone) {
			m->from = alias[i];
		}
	}

	/* do zone gating as necessary */

	if ((thisnode.zone != m->to.zone) && (!m->attrib.direct) &&
	    (!m->attrib.crash) && (arealist[area].netmail) &&
	    (gate & GZONES)) {
		m->to.net = thisnode.zone;
		m->to.node = m->to.zone;
	}

	/* remap point originated crashmail */

	if ((!m->attrib.direct) && (!m->attrib.crash) &&
	    (m->from.point) && (pointnet != 0)) {
		m->from.net = pointnet;
		m->from.node = m->from.point;
		m->from.point = 0;
	}

	/* do domain gating */

	if ((gate & GDOMAINS) && domains && 
	    !m->attrib.direct && strcmpl(m->from.domain,m->to.domain)) {
		int i;
		for (i = 0; i < domains; i++)
			if (strcmpl(domain_list[i].domain,m->to.domain) == 0) {
				m->to = domain_list[i];
				break;
			}
	}

	msg_writeheader(m);

	m->to = tmpto;
	n = m->msgnum;

	if (arealist[area].netmail) {

		if ((m->to.domain != NULL) && (m->from.domain != NULL) &&
			(strncmp(strupr(m->to.domain),strupr(m->from.domain),8))) {
			sprintf(buf,"\01DOMAIN %s %d:%d/%d %s %d:%d/%d\r",
				m->to.domain,m->to.zone,m->to.net,m->to.node,
				tmpfrom.domain,tmpfrom.zone,tmpfrom.net,tmpfrom.node);
			msg_writetext(buf,n);
		}

		if (((m->from.zone != m->to.zone) || (thisnode.zone != m->to.zone)) &&
			(arealist[area].netmail)) {
			sprintf(buf, "\01INTL %d:%d/%d %d:%d/%d\r",
				m->to.zone, m->to.net, m->to.node,
				m->from.zone, m->from.net, m->from.node);
			msg_writetext(buf,n);
		}

		if (m->to.point) {
			sprintf(buf, "\01TOPT %d\r", m->to.point);
			msg_writetext(buf,n);
		}

		if (m->from.point) {
			sprintf(buf, "\01FMPT %d\r", m->from.point);
			msg_writetext(buf,n);
		}
	}

	if (msgids && !uucp_to) {
		sprintf(buf,"\01MSGID: %s %08lx\r",show_address(tmpfrom),now);
		msg_writetext(buf,n);
	}

	if (m->msgid && !uucp_to && m->new && msgids) {
		msg_writetext("\01REPLY: ",n);
		msg_writetext(m->msgid,n);
		msg_writetext("\r",n);
	}
	else if (m->reply && msgids) {
		msg_writetext("\01REPLY: ",n);
		msg_writetext(m->reply,n);
		msg_writetext("\r",n);
	}

	if (uucp_to && !arealist[area].news) {
		msg_writetext("To:   ",n);
		msg_writetext(uucp_to,n);
		msg_writetext("\r",n);
	}

	if (uucp_fr) {
		msg_writetext("From: ",n);
		msg_writetext(uucp_fr,n);
		msg_writetext("\r",n);
	}

	l = m->text;
	while (l->next) l = l->next;

	while (l) {
		if ((l->quote) || (*(l->text) == '\n')) {
			l = l->prev;
			if (l->next->text) free(l->next->text);
			free(l->next);
			l->next = NULL;
		}
		else
			break;
	}

	l = m->text;

	while (l != NULL) {
		char   *t = NULL;

		if (l->text == NULL)
			break;

		if ((*l->text == '\01') && (stripnotes)) {
			if (*(l->text+1) != 'P') {
				l = l->next;
				continue;
			}
		}

		if (newtear)
			newtear = strncmp(l->text,"--- ",4);

		if ((t = strchr(l->text,'\n')) != NULL)
			*t = '\r';

		msg_writetext(l->text,n);

		if (t != NULL)
			*t = '\n';

		if ((t == NULL) && softcr) {
			if (!isspace(*(l->text+strlen(l->text)-1)))
				msg_writetext(" ",n);
			msg_writetext("\x8d",n);
		}

		l = l->next;
	}

	if ((tearline && arealist[area].echomail) && newtear) {
		msg_writetext("\r\r--- msged " VERSION "\r",n);
		sprintf(buf," * Origin: %s (%s)\r",corigin,show_address(tmpfrom));
		msg_writetext(buf,n);
	}

	msg_writetext("\r",n);
	msg_writetext(NULL,n);
	arealist[area].new = 1;

	if (uucp_fr) {
		release(m->from.domain);
		m->from = thisnode;
		m->from.fidonet = 0;
		if (strchr(uucp_fr,'@'))
			m->from.internet = 1;
		else
			m->from.bangpath = 0;

		m->from.domain = uucp_fr;
	}

	if (uucp_to) {
		release(m->to.domain);
		m->to = thisnode;
		m->to.fidonet = 0;
		if (strchr(uucp_to,'@'))
			m->to.internet = 1;
		else
			m->to.bangpath = 0;

		m->to.domain = uucp_to;
	}

	return (TRUE);
}
