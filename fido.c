/**

 * fido/opus style message base support functions for msged

 * released into the PUBLIC DOMAIN 30 jul 1990 by jim nutt

**/

#define CHUNKSZ 10
#define TEXTLEN 96
#define BUFLEN	4096

#if defined(__MSC__) || defined(__TURBOC__)
#include <sys\types.h>
#include <fcntl.h>
#endif
#include <sys\stat.h>
#include <dos.h>
#include "msged.h"
#include "date.h"

int _pascal dir_findnext(struct _dta * dta);
int _pascal dir_findfirst(char * filename, int attribute, struct _dta * dta);

typedef struct _dosdate {
	unsigned int day:5;
	unsigned int mon:4;
	unsigned int year:7;
	unsigned int sec:5;
	unsigned int min:6;
	unsigned int hour:5;
} DOSDATE;

void _pascal normalize(char *s);
static int cdecl compare(const void *i, const void *j);

typedef struct _fidoheader {
	char from[36];			/* who from,			*/
	char to[36];			/* who to,			*/
	char subj[72];			/* message subject, 	*/
	char date[20];			/* creation date,		*/
	int times;			/* number of times read,		*/
	int dest;			/* destination node,		*/
	int orig;			/* originating node 	*/
	int cost;			/* actual cost this msg 	*/
	int orig_net;			/* originating net		*/
	int dest_net;			/* destination net		*/
	DOSDATE written;		/* when it was written		*/
	DOSDATE arrived;		/* when it arrived		*/
	unsigned int reply; 		/* thread to previous msg		*/
	struct _attributes attrib;	/* message attributes		*/
	int up; 			 /* thread to next msg		*/
} FIDOHEADER;

int _pascal fido_writetext(char *text, int n)

{
	char	path[PATHLEN];
	FIDOHEADER msghead;
	static	FILE   *fp;

	if (fp == NULL) {
		sprintf(path, "%s/%d.MSG", arealist[area].path,n);
		if ((fp = fopen(path, "rb")) == NULL)
			return (FALSE);
		fread(&msghead,sizeof msghead,1,fp);
		fp = freopen(path,"wb",fp);
		fwrite(&msghead, sizeof msghead,1,fp);
	}

	if (text == NULL) {
		fputc(0,fp);
		fclose(fp);
		fp = NULL;
		return(TRUE);
	}

	fputs(text,fp);
	fflush(fp);
	return(TRUE);
}

int _pascal fido_writeheader(MSG *m)

{
	char	path[PATHLEN];
	FIDOHEADER msghead;
	FILE   *fp;
	time_t	now = time(NULL);
	struct	tm *ts;
	int 	n = m->msgnum;

	ts = localtime(&now);

	do {
		sprintf(path, "%s/%d.MSG", arealist[area].path,n);

		if (m->new) {
			if ((fp = fopen(path,"rb")) != NULL) {
				fclose(fp);
				fp = NULL;
				n = ++m->msgnum;
			}
			else {
				if ((fp = fopen(path,"wb")) == NULL)
					return(FALSE);
			}
		}
		else {
			if ((fp = fopen(path,"r+b")) == NULL) {
				if ((fp = fopen(path,"wb")) == NULL) {
					return(FALSE);
				}
			}
			else
				fseek(fp,0l,SEEK_SET);
		}
	} while (fp == NULL);

	memset(&msghead,0,sizeof msghead);
	msghead.attrib = m->attrib;
	msghead.reply = m->replyto;
	msghead.up = m->replyfrom;
	msghead.times = m->times_read;
	msghead.cost = m->cost;

	msghead.dest_net = m->to.net;
	msghead.dest = m->to.node;

	memcpy(msghead.to,m->isto,min(sizeof msghead.to,strlen(m->isto)));
	memcpy(msghead.from,m->isfrom,min(sizeof msghead.from,strlen(m->isfrom)));
	memcpy(msghead.subj,m->subj,min(sizeof msghead.subj,strlen(m->subj)));
	memcpy(msghead.date, mtime(m->timestamp), 20);

	msghead.orig_net = m->from.net;
	msghead.orig = m->from.node;

	if (opusdate) {
		msghead.written.year = ts->tm_year - 80;
		msghead.written.mon = ts->tm_mon + 1;
		msghead.written.day = ts->tm_mday;
		msghead.written.hour = ts->tm_hour;
		msghead.written.min = ts->tm_min;
		msghead.written.sec = ts->tm_sec / 2;
		msghead.arrived = msghead.written;
	}

	fwrite((char *) &msghead, sizeof(FIDOHEADER), 1, fp);
	fclose(fp);
	return(TRUE);
}

MSG *pascal fido_readheader(int n)

{
	FIDOHEADER msghead;
	char	path[PATHLEN];
	int 	fd = 0;
	MSG    *m;

	memset(&msghead,0,sizeof msghead);
	if ((m = (MSG *) calloc(1,sizeof(MSG))) == NULL)
		outamemory();

	sprintf(path, "%s/%d.MSG", arealist[area].path,n);

#if defined(__MSC__) || defined(__TURBOC__)
	if ((fd = open(path, O_RDONLY|O_BINARY)) == -1) {	   /*WRA*/
#else
	if ((fd = dos_open(path, 0)) == -1) {
#endif
		free(m);
		return (NULL);
	}

	m->msgnum = n;

	read(fd, (char *) &(msghead), (int) sizeof(FIDOHEADER));

	m->from.net = msghead.orig_net;
	m->from.node = msghead.orig;
	m->to.net = msghead.dest_net;
	m->to.node = msghead.dest;

	memset(path, 0, sizeof path);
	memcpy(path,msghead.date,sizeof msghead.date);
	m->timestamp = parsedate(path);
	m->date = strdup(path);

	if ((m->isto = (char *) calloc(1,sizeof msghead.to + 1)) == NULL) outamemory();
	memcpy(m->isto,msghead.to,sizeof msghead.to);
	if ((m->isfrom = (char *) calloc(1,sizeof msghead.from + 1)) == NULL) outamemory();
	memcpy(m->isfrom,msghead.from,sizeof msghead.from);
	if ((m->subj = (char *) calloc(1,sizeof msghead.subj + 1)) == NULL) outamemory();
	memcpy(m->subj,msghead.subj,sizeof msghead.subj);
	m->attrib = msghead.attrib;
	m->replyto = msghead.reply;
	m->replyfrom = msghead.up;
	m->from.zone = thisnode.zone;
	m->to.zone = thisnode.zone;
	m->cost = msghead.cost;
	m->times_read = msghead.times;
	m->text = NULL;
	m->to.fidonet = 1;
	m->from.fidonet = 1;
	close(fd);
	return(m);
}

static int cdecl compare(const void *i, const void *j)

{
	return(* (int *) i - * (int *) j);
}

int _pascal fido_scan(AREA *a)

{
	struct _dta fileinfo;
	char	path[PATHLEN];
	int 	cnt = 0;
	int lastalloc = 0, c = 10;
	int 	msgnum,l;
	FILE   *fp = NULL;
	int 	status;
	int *t = NULL;

	if (messages != NULL)
		free(messages);

	messages = NULL;

	sprintf(path,"%s/*.msg",a->path);

	a->last = 0;

	status = dir_findfirst(path,0,&fileinfo);

	while (status != -1) {
		if (fileinfo.size > sizeof(FIDOHEADER)) {
			msgnum = atoi(fileinfo.name);
			cnt++;

			if (cnt >= lastalloc) {
				t = calloc((lastalloc += CHUNKSZ), sizeof(int));
				if (t == NULL) outamemory();
				if (messages != NULL)
					memcpy(t,messages,sizeof(int) * cnt);
			}

			if (t) {
				if ((messages != NULL) && (c != lastalloc)) {
					free(messages);
					c = lastalloc;
				}
				messages = t;
				messages[cnt - 1] = msgnum;
			}
		}
		if (t == NULL) {
			cnt--;
			break;
		}
		status = dir_findnext(&fileinfo);
	}

	qsort(messages,cnt,sizeof(int),compare); /* put them in order */

	sprintf(path, "%s/%s",a->path, lastread);

	fp = fopen(path, "rb");
	if (fp != NULL) {
		if (fread(&c, 2, 1, fp) == 1) {
			if (fread(&l,2,1,fp) != 1)
			   l = c;
		}
		else
			l = c = 0;
		fclose(fp);
	}
	else {
		l = c = 0;
		if ((fp = fopen(path, "wb")) != NULL) {
			fputc(0, fp);
			fputc(0, fp);
			fputc(0, fp);
			fputc(0, fp);
			fclose(fp);
		}
	}

	if (cnt != 0) {

		a->first = messages[0];
		a->last = (a->msgtype==0)?messages[cnt - 1]:cnt;

		if (c == 0)
			msgnum = 0;
		else
			for (msgnum = cnt - 1; (msgnum > -1) && (messages[msgnum] != c); msgnum--)
			/* null statement */;
		a->current = (msgnum == -1) ? cnt-1:msgnum;

		if (l == 0)
			msgnum = 0;
		else
			for (msgnum = cnt - 1; (msgnum > -1) && (messages[msgnum] != l); msgnum--)
			/* null statement */;
		a->lastread = (msgnum == -1) ? cnt-1:msgnum;
	}
	else
		a->current = a->lastread =
		a->first = a->last = 0;

	return (cnt);
}

char *pascal fido_readtext(int n)

{
	static	char *buffer = NULL;
	static	char *next = NULL;
	static	char *end = NULL;
	static	int fd = 0;
	static	unsigned int s;
	char	path[PATHLEN];
	int 	i, l;
	char	*t = NULL;
	char	*text = NULL;
	char	eol = '\0';


	if (n < 0) {
		close(fd);
		fd = s = 0;
		next = NULL;
		return(NULL);
	}

	if (fd == 0) {

		sprintf(path, "%s/%d.MSG", arealist[area].path, n);

		s = BUFLEN;

		if (buffer == NULL)
			buffer = (char *) calloc(sizeof(char),s);
		else
			memset(buffer,0,s);

#if defined(__MSC__) || defined(__TURBOC__)
		if ((fd = open(path, O_RDONLY|O_BINARY)) == -1) {  /*WRA*/
#else
		if ((fd = dos_open(path, 0)) == -1) {
#endif
			return (NULL);
		}
		lseek(fd,(long) sizeof(FIDOHEADER), SEEK_SET);
	}

	if (next == NULL) {
		i = read(fd, buffer, s - 1);
		if (i < 1) {
			close(fd);
			next = NULL;
			s = 0;
			fd = 0;
			return(NULL);
		}
		next = buffer;
		while (i && (*next == '\0'))
			i--, next++;
		normalize(next);
		end = buffer + strlen(buffer);
	}

	t = memchr(next,'\n',(int) (end - next));

	if (t == NULL) {
		l = strlen(next);
		memcpy(buffer,next,l+1);
		i = read(fd,buffer+l, s - l - 1);
		if (i < 1) {
			next = NULL;
			return(strdup(buffer));
		}
		*(buffer + l + i) = '\0';
		normalize(buffer+l);
		end = buffer + strlen(buffer);
		next = buffer;
		t = memchr(next,'\n',s);
	}

	if (t != NULL) {
		eol = *(t+1);
		*(t+1) = '\0';
	}

	text = strdup(next);

	if (t != NULL) {
		*(t+1) = eol;
		next = t+1;
	}
	else
		next = NULL;

	return(text);
}

int _pascal fido_setlast(AREA a)

{
	FILE   *fp;
	char	path[PATHLEN];
	int 	i = 1;

	sprintf(path, "%s/%s", a.path, lastread);
	if ((fp = fopen(path, "w+b")) == NULL)
		return(FALSE);
	fseek(fp,0l,SEEK_SET);
	if (messages) {
		fwrite((char *) &messages[a.current], sizeof(int), 1, fp);
		fwrite((char *) &messages[a.lastread], sizeof(int), 1, fp);
	}
	else {
		fwrite(&i, sizeof(int), 1, fp);
		fwrite(&i, sizeof(int), 1, fp);
	}
	fclose(fp);
	return(TRUE);
}


int _pascal fido_delete(int n)

{
	char path[TEXTLEN];

	sprintf(path, "%s/%d.msg", arealist[area].path, msgnum(n));
	remove(path);
	return(TRUE);
}
