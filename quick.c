/**

 * support for quickbbs style message bases
 *
 * released into the PUBLIC DOMAIN 30 Jul 1990 by jim nutt

**/


#define CHUNKSZ 32

#include <time.h>

#include "msged.h"
#include "date.h"

#ifdef __MSC__
#include <sys/types.h>
#endif
#include <sys/stat.h>

void _pascal normalize(char *t);

int qlast[200];
int qcur[200];

struct qinfo {
	int     low;
	int     high;
	int     active;
	int     areas[200];
};

struct qidx {
	int     number;
	char	board;
};

struct qmsg {
	int     number;
	int	replyto;
	int	replyfrom;
	int     times_read;
	unsigned int start;
	unsigned int count;
	int	destnet;
	int	destnode;
	int	orignet;
	int	orignode;
	char	destzone;
	char	origzone;
	int	cost;
	/* message attributes */
	unsigned int deleted:1;
	unsigned int outnet:1;
	unsigned int netmail:1;
	unsigned int private:1;
	unsigned int recvd:1;
	unsigned int echo:1;
	unsigned int local:1;
	unsigned int xx1:1;
	unsigned int killsent:1;
	unsigned int sent:1;
	unsigned int attach:1;
	unsigned int crash:1;
	unsigned int rreq:1;
	unsigned int areq:1;
	unsigned int rrcpt:1;
	unsigned int xx2:1;
	char    board;
	char    posttime[6];
	char    postdate[9];
	char    whoto[36];
	char    whofrom[36];
	char    subject[73];
};

struct qtoidx {
	char    length;
	char    text[35];
};

struct qtext {
	char length;
	char text[BLOCKLEN];
};

static  struct qinfo info;
static  struct qmsg header;

static	char path[80];
static  long start = -1;
static  long count = 0;
static  int  position = 0;
static 	FILE *infofp = NULL;
static 	FILE *idxfp = NULL;
static 	FILE *textfp = NULL;
static 	FILE *hdrfp = NULL;
static 	FILE *toidxfp = NULL;

int   _pascal quick_delete(int n)

{
	struct qidx index;

	header.deleted = 1;

	if (fseek(hdrfp, (long) (messages[n] * (long) sizeof header), SEEK_SET))
		return FALSE;

	fwrite(&header, sizeof header, 1, hdrfp);
	fflush(hdrfp);

	fseek(idxfp, messages[n] * (long) sizeof index, SEEK_SET);
	index.board = (char) arealist[area].board;
	index.number = -1;
	fwrite(&index,1,sizeof index,idxfp);
	fflush(idxfp);

	start = count = 0;
	position = 0;

	return(TRUE);
}

int   _pascal quick_writetext(char *text, int n)

{
	struct qtext block;
	char *s;
	static char buf[768];
	struct stat b;
	static int f = 0;

	if (f == 0) {
		f = 1;
		fstat(fileno(textfp),&b);
		start = b.st_size / sizeof block;
		count = 0;
	}

	if (text == NULL) {
		n = position;
		memset(block.text,0,sizeof block.text);
		strcpy(block.text,buf);
		block.length = (char) strlen(buf);
		fseek(textfp,(long) (start + count) * (long) sizeof block, SEEK_SET);
		fwrite(&block,1,sizeof block,textfp);
		fflush(textfp);
		header.start = (unsigned int) start;
		header.count = (unsigned int) ++count;
		fseek(hdrfp, (long) n * (long) sizeof header, SEEK_SET);
		fwrite(&header, sizeof header, 1, hdrfp);
		fflush(hdrfp);
		f = 0;
		memset(buf,0,sizeof buf);
		return(TRUE);
	}

	strcat(buf,text);
	while (strlen(buf) > sizeof block.text) {
		s = buf + sizeof block.text;
		memcpy(block.text,buf,sizeof block.text);
		strcpy(buf,s);
		block.length = sizeof block.text;
		fseek(textfp,(long) (start + count) * (long) sizeof block, SEEK_SET);
		fwrite(&block,sizeof block,1,textfp);
		fflush(textfp);
		count++;
	}

	return(TRUE);
}

MSG  *pascal quick_readheader(int n)

{
	struct stat b;
	char path[80];
	MSG *m;
	long i;

	memset(path,0,sizeof path);
	fstat(fileno(hdrfp),&b);

	i = (long) (messages[n]) * (long) sizeof header;
	if (i > b.st_size)
		return(NULL);

	position = messages[n];

	fseek(hdrfp, i, SEEK_SET);
	if (fread(&header, sizeof header, 1, hdrfp) != 1)
		return(NULL);

	start = (long) header.start;
	count = (long) header.count;

	fstat(fileno(textfp),&b);
	if (((start + count) * (long) sizeof(struct qtext)) > b.st_size)
		return(NULL);

	if (header.deleted)
		return (NULL);

	m = (MSG *) calloc(1, sizeof(MSG));

	m->msgnum = header.number;

	m->isfrom = (char *) calloc(header.whofrom[0] + 1,sizeof(char));
	strncpy(m->isfrom,header.whofrom+1,header.whofrom[0]);

	m->isto = (char *) calloc(header.whoto[0] + 1,sizeof(char));
	strncpy(m->isto,header.whoto+1,header.whoto[0]);

	m->subj = (char *) calloc(header.subject[0] + 1,sizeof(char));
	strncpy(m->subj,header.subject+1,header.subject[0]);

	strncpy(path,header.postdate + 1, header.postdate[0]);
	strcat(path," ");
	strncat(path,header.posttime + 1, header.posttime[0]);
	m->timestamp = parsedate(path);
	m->date = strdup(path);

	m->attrib.private = header.private;
	m->attrib.crash = header.crash;
	m->attrib.recvd = header.recvd;
	m->attrib.sent = header.sent;
	m->attrib.attached = header.attach;
	m->attrib.forward = 0;
	m->attrib.orphan = 0;
	m->attrib.killsent = header.killsent;
	m->attrib.local = header.local;
	m->attrib.hold = header.xx1;
	m->attrib.direct = header.xx2;
	m->attrib.freq = 0;
	m->attrib.rreq = header.rreq;
	m->attrib.rcpt = header.rrcpt;
	m->attrib.areq = header.areq;
	m->attrib.ureq = 0;

	m->to.zone = header.destzone;
	m->to.net = header.destnet;
	m->to.node = header.destnode;

	m->from.zone = header.destzone;
	m->from.net = header.destnet;
	m->from.node = header.destnode;

	m->to.fidonet = m->from.fidonet = 1;

	return(m);
}

int   _pascal quick_writeheader(MSG *m)

{
	struct qidx index;
	struct tm *ts;
	struct stat b;
	FILE *fp;
	int c = arealist[area].current;

	memset(&header,0,sizeof header);
	header.number = m->msgnum;
	header.start = (unsigned int) start;
	header.count = (unsigned int) count;

	if (m->new) {
		c = arealist[area].messages - 1;
		fstat(fileno(hdrfp),&b);
		messages[c] = (int) (b.st_size / sizeof header);
		start = (unsigned long) (header.start = 0);
		count = (unsigned long) (header.count = 0);
		info.areas[arealist[area].board-1]++;
		info.active++;
		header.number = ++info.high;
	}

	position = messages[c];

	header.replyto = 0;
	header.replyfrom = 0;
	header.times_read = m->times_read;
	header.destzone = (char) m->to.zone;
	header.destnet = m->to.net;
	header.destnode = m->to.node;
	header.origzone = (char) m->from.zone;
	header.orignet = m->from.net;
	header.orignode = m->from.node;
	header.cost = m->cost;

	header.deleted = 0;
	header.outnet = 0;
	header.netmail = 0;
	header.echo = 0;

	if (arealist[area].netmail) {
		header.netmail = 1;
		header.outnet = 1;
	}

	if (arealist[area].echomail)
		header.echo = 1;

	header.private = m->attrib.private;
	header.recvd = m->attrib.recvd;
	header.local = m->attrib.local;
	header.xx1 = 0;
	header.killsent = m->attrib.killsent;
	header.sent = m->attrib.sent;
	header.attach = m->attrib.attached;
	header.crash = m->attrib.crash;
	header.rreq = m->attrib.rreq;
	header.areq = m->attrib.areq;
	header.rrcpt = m->attrib.rcpt;
	header.xx2 = m->attrib.direct;
	header.board = (char) arealist[area].board;

	ts = localtime(&m->timestamp);
	header.posttime[0] = 5;
	sprintf(header.posttime+1,"%02d:%02d",ts->tm_hour, ts->tm_min);
	header.postdate[0] = 8;
	sprintf(header.postdate + 1,"%02d-%02d-%02d",ts->tm_mon+1,ts->tm_mday,ts->tm_year);

	header.whoto[0] = (char) min(strlen(m->isto), sizeof(header.whoto) - 1);
	memcpy(header.whoto+1,m->isto,header.whoto[0]);
	header.whofrom[0] = (char) min(strlen(m->isfrom), sizeof(header.whofrom));
	memcpy(header.whofrom+1,m->isfrom,header.whofrom[0]);
	header.subject[0] = (char) min(strlen(m->subj), sizeof(header.subject) - 1);
	memcpy(header.subject+1,m->subj, header.subject[0]);

	fseek(hdrfp, (long) position * (long) sizeof header, SEEK_SET);
	fwrite(&header, sizeof header, 1, hdrfp);

	fseek(infofp,0l,SEEK_SET);
	fwrite(&info,sizeof info, 1, infofp);

	index.number = header.number;
	index.board = (char) arealist[area].board;
	fseek(idxfp,(long) position * (long) sizeof(struct qidx),SEEK_SET);
	fwrite(&index,sizeof index,1,idxfp);

	fflush(idxfp);
	fflush(infofp);
	fflush(hdrfp);

	strcpy(path,quickbbs);
	strcat(path,"msgtoidx.bbs");
	if ((fp = fopen(path,"r+b")) == NULL)
		if ((fp = fopen(path,"w+b")) == NULL)
			return(TRUE);

	fseek(fp,(long) position * (long) sizeof(header.whoto), SEEK_SET);
	fwrite(header.whoto,sizeof(header.whoto),1,fp);
	fclose(fp);

	return(TRUE);
}

char *pascal quick_readtext(int n)

{
	struct qtext text;

	static long int b = -1;
	static long int c = -1;

	static char *next = NULL;
	char *t,*t2,ch = '\0';

	static char *s = NULL;

	if (n < 0) {
		b = c = -1;
		next = NULL;
		if (s) free(s);
		s = NULL;
		return NULL;
	}

	if ((next == NULL) && (s != NULL)) {
		free(s);
		b = c = -1;
		s = NULL;
		return(NULL);
	}

	if (s == NULL) {

		if (b == -1) {
			b = (long) (start = (long) header.start);
			c = (long) (count = (long) header.count);
		}

		if ((c < 1) || (b < 0)) {
			b = c = -1;
			return NULL;
		}

		if ((s = (char *) malloc((size_t) count * sizeof text + 1)) == NULL) {
			b = c = -1;
			return NULL;
		}

		memset(s,0,c * sizeof text + 1);

		fseek(textfp, (long) (start * (long) sizeof text), SEEK_SET);
		while (c) {
			memset(text.text,0,sizeof text);
			fread(&text, sizeof text, 1, textfp);
			strncat(s,text.text,text.length);
			c--;
		}
		normalize(s);
		next = s;
	}

	t = next;
	next = strchr(t,'\n');
	if (next) {
		ch = *(next+1);
		*(next+1) = '\0';
	}

	t2 = strdup(t);

	if (next) {
		*(next+1) = ch;
		next++;
	}

	return(t2);
}

int   _pascal quick_setlast(AREA a)

{
	FILE *fp;

	strcpy(path,quickbbs);
	strcat(path,"lastread.bbs");

	qlast[a.board-1] = messages[a.lastread];
	qcur[a.board-1] = messages[a.current];

	if ((fp = fopen(path,"wb")) != NULL) {
		fwrite(qlast,1,sizeof qlast,fp);
		fclose(fp);
	}

	strcpy(path,quickbbs);
	strcat(path,"current.bbs");

	if ((fp = fopen(path,"wb")) != NULL) {
		fwrite(qcur,1,sizeof qcur,fp);
		fclose(fp);
	}

	return(TRUE);
}

int   _pascal quick_scan(AREA *a)

{
	struct qidx  index;
	FILE *fp;
	int i, i2, idx;
	int  *t;

	position = 0;

	if (infofp != NULL) fclose(infofp);
	strcpy(path,quickbbs);
	strcat(path,"msginfo.bbs");

	if ((infofp = fopen(path,"r+b")) == NULL)
		if ((infofp = fopen(path,"w+b")) == NULL)
			return(0);

	if (idxfp != NULL) fclose(idxfp);
	strcpy(path,quickbbs);
	strcat(path,"msgidx.bbs");
	if ((idxfp = fopen(path,"r+b")) == NULL)
		if ((idxfp = fopen(path,"w+b")) == NULL)
			return(0);

	if (textfp != NULL) fclose(textfp);
	strcpy(path,quickbbs);
	strcat(path,"msgtxt.bbs");
	if ((textfp = fopen(path,"r+b")) == NULL)
		if ((textfp = fopen(path,"w+b")) == NULL)
			return(0);

	if (hdrfp != NULL) fclose(hdrfp);
	strcpy(path,quickbbs);
	strcat(path,"msghdr.bbs");
	if ((hdrfp = fopen(path,"r+b")) == NULL)
		if ((hdrfp = fopen(path,"w+b")) == NULL)
			return(0);

	if (toidxfp != NULL) fclose(toidxfp);
	strcpy(path,quickbbs);
	strcat(path,"msgtoidx.bbs");
	if ((toidxfp = fopen(path,"r+b")) == NULL)
		if ((toidxfp = fopen(path,"r+b")) == NULL)
			return(0);

	if (messages != NULL)
		free(messages);

	messages = NULL;

	rewind(infofp);
	if (fread(&info, (unsigned) sizeof info, 1, infofp) != 1)
		memset(&info,0,sizeof info);

	i = 0; i2 = 0; idx = 0;
	rewind(idxfp);
	while ((fread(&index, (unsigned) sizeof index, 1, idxfp) == 1)) {
		if ((index.board == (char) a->board) && (index.number > 0)) {
			if (i >= i2) {
				t = realloc(messages, (i2+=CHUNKSZ) * sizeof(int));
				if (t == NULL)
					break;
				messages = t;
			}
			messages[i++] = idx;
		}
		idx++;
	}

	a->first = 0;
	a->last = i;

	strcpy(path,quickbbs);
	strcat(path,"lastread.bbs");
	if ((fp = fopen(path,"rb")) != NULL) {
		fread(qlast,1,sizeof qlast,fp);
		a->lastread = qlast[a->board - 1];
		fclose(fp);
	}

	strcpy(path,quickbbs);
	strcat(path,"current.bbs");
	if ((fp = fopen(path,"rb")) != NULL) {
		fread(qcur,1,sizeof qcur,fp);
		a->current = qcur[a->board - 1];
		fclose(fp);
	}
	else
		a->current = a->lastread;

	for (i2 = i-1; i2 && (messages[i2] != a->lastread); i2--) ;
	if (i2) a->lastread = i2;

	for (i2 = i-1; i2 && (messages[i2] != a->current); i2--) ;
	if (i2) a->current = i2;

	if (a->lastread >= i) a->lastread = i - 1;
	if (a->current >= i) a->current = i - 1;

	info.areas[a->board-1] = i;

	return(i);
}
