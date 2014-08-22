/*

main header for msged, most globals, lots of prototypes..

PUBLIC DOMAIN

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>

#ifdef __OS2__
#include "os2supp.h"
#else
#include <dos.h>
#endif

#include <time.h>

#ifdef __MSC__
#include <malloc.h>
#endif

#define NOSPELL

#if defined(__ZTC__)
#define NO_HANDLE 1
#include <handle.h>
#else
#include "nohandl.h"
#endif

#include "pascal.h"	/* hides the oldstyle far/near/pascal */

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef NDEBUG
#define check(p) printf("%s (%d): %s = %p = %s\n",__FILE__,__LINE__,#p,p,p)
#define checkp(p) printf("%s (%d): %s = %p\n",__FILE__,__LINE__,#p,p)
#else
#define check(p)
#define checkp(p)
#endif

#include "nedit.h"
#include "screen.h"

#ifndef __OS2__
#define VERSION "2.08"
#else
#define VERSION "2.06 OS/2"
#endif

#define FALSE			0
#define TRUE	   !FALSE

#define NO				0
#define YES 			1
#define HIDE			2

#define GDOMAINS		1
#define GZONES			2
#define BOTH			GDOMAINS | GZONES

#define FIDO			0
#define QUICK			1

#define USENET			1
#define FIDONET 		2

#define PATHLEN 	   64
#define BLOCKLEN	  255

#define thisnode		alias[0]

#ifdef __MSC__
#define strcmpl stricmp
#define strncmpi strnicmp
#endif

#if defined(__TURBOC__) || defined(__TSC__)
#define strcmpl strcmpi
#endif

#define checkmem(x) 	{ if ((x) == NULL) outamemory(); }

/* structures and typedefs */

struct _attributes {
	unsigned int private:1;    /* private message flag	   */
	unsigned int crash:1;      /* crash mail                   */
	unsigned int recvd:1;      /* received by addressee        */
	unsigned int sent:1;       /* message sent                 */
	unsigned int attached:1;   /* file attached 		   */
	unsigned int forward:1;    /* message in transit	   */
	unsigned int orphan:1;	   /* unknown destination	   */
	unsigned int killsent:1;   /* kill after sending	   */
	unsigned int local:1;	   /* local message 		   */
	unsigned int hold:1;	   /* hold for pickup		   */
	unsigned int direct:1;     /* do no gating on this msg     */
	unsigned int freq:1;       /* file request                 */
	unsigned int rreq:1;       /* return receipt requested     */
	unsigned int rcpt:1;	   /* return receipt		   */
	unsigned int areq:1;	   /* audit trail request	   */
	unsigned int ureq:1;	   /* update file request	   */
};

typedef struct _address {
	int 	zone;
	int 	net;
	int 	node;
	int 	point;
	char   *domain;                         /* the domain tag, uucp path or internet address */
	unsigned int notfound:1;                /* address may be invalid */
	unsigned int fidonet:1;                 /* address is a fidonet technology address */
	unsigned int internet:1;                /* address is in internet user@site.domain format */
	unsigned int bangpath:1;                /* address is a uucp bangpath */
} ADDRESS;

typedef struct _area {
	char *description;                      /* what the user calls the area */
	char *tag;				/* what confmail calls it! */
	char *path;                             /* where the area is on disk */
	unsigned int local:1;                   /* local message area */
	unsigned int netmail:1;                 /* netmail message area */
	unsigned int echomail:1;                /* echomail area */
	unsigned int news:1;                    /* usenet news area */
	unsigned int uucp:1;                    /* usenet mail area */
	unsigned int new:1;                     /* a message has been entered */
	unsigned int priv:1;                    /* default privileged */
	unsigned int hold:1;                    /* default hold */
	unsigned int direct:1;                  /* default direct */
	unsigned int crash:1;                   /* default crash */
	unsigned int killsent:1;                /* default crash */
	int msgtype;                            /* the message type */
	int board;				/* if a quickbbs area, which board number */
	int first;				/* first message in the area */
	int last;				/* last message in the area */
	int current;                            /* current message in the area */
	int messages;                           /* how many messages in the area */
	int lastread;                           /* the highest message read */
} AREA;

typedef struct _msg {
	int msgnum;                             /* local message number              */
	char *reply;                            /* id of message this is a reply to  */
	char *msgid;                            /* this messages msgid               */
	char *isfrom;                           /* who from,                         */
	char *isto;                             /* who to,                           */
	char *subj;                             /* message subject,                  */
	unsigned int new:1;                     /* new message                       */
	unsigned int change:1;
	time_t timestamp;                       /* creation date,                    */
	char * date;                            /* unparsed creation date            */
	unsigned int replyto;                   /* thread to previous msg            */
	unsigned int replyfrom;                 /* thread to next msg                */
	unsigned int *links;                    /* list of replies to this message   */
	int replies;                            /* number of replies to this message */
	struct _attributes attrib;		/* message attribute*/
	int times_read;
	int cost;
	ADDRESS to;                             /* destination address of message    */
	ADDRESS from;                           /* origin address of message         */
	LINE * text;                            /* the message buffer                */
} MSG;

typedef struct _colors {
	unsigned char normal;                   /* the normal text color */
	unsigned char quote;                    /* quoted text color */
	unsigned char warn;                     /* warning and error message color */
	unsigned char block;                    /* block color */
	unsigned char info;                     /* informational messages */
	unsigned char hilite;                   /* general highlighting */
} COLORS;

/* this is a list of possible commands and responses */

struct _command {
	char *label;
	void (_near _pascal *action)();
};

/* these are pointers to the various message handling primitives */

typedef struct _msghandle {
	MSG  *(_pascal *msg_readheader)(int n);                 /* read the header */
	char *(_pascal *msg_readtext)(int n);							  /* read one line */
	int   (_pascal *msg_writeheader)(MSG *m);						  /* write the header */
	int   (_pascal *msg_writetext)(char *text, int n); 	  /* write a line */
	int   (_pascal *msg_delete)(int n);
	int   (_pascal *msg_last)(AREA a);
	int   (_pascal *msg_scan)(AREA *a);
} MSGHANDLE;

struct _dta   {
	char			reserved[21];
	char			attribute;
	unsigned		time;
	unsigned		date;
	long			size;
	char			name[13];
};

/* imports from screen */

extern unsigned int far *macros[41];            /*  one for each of the various
						    function keys */

extern int              maxy,                   /* how many screen lines? */
			maxx,                   /* how many screen columns? */
			videomethod;            /* DIRECT, BIOS or FOSSIL */

extern unsigned int vbase;			/* the video segment */

/* some more includes (dependent on above) */

#include "fido.h"
#include "quick.h"

/* global variables */

#ifdef MAIN
/*
  the following table defines which functions are used for what msgtype
   currently only two message types are recognized, fido/opus style .msg files
   quickbbs style databases.  adding additional types should be relatively easy
*/

MSGHANDLE msgdo[2] = { /* indexed by msgtype in AREA structure */
	{fido_readheader, fido_readtext,
	 fido_writeheader, fido_writetext,
	 fido_delete, fido_setlast, fido_scan},

	{quick_readheader, quick_readtext,
	 quick_writeheader, quick_writetext,
	 quick_delete, quick_setlast, quick_scan}
};

AREA    __handle *arealist = NULL;

char	*username = NULL,			   /* who is you */
	*quotestr = NULL,			   /* how to prefix a quote */
	*fidolist = NULL,			   /* nodelist user list */
	*userlist = NULL,			   /* personal user list */
	*origin   = NULL,			   /* origin line */
	*outfile  = NULL,			   /* default export filename */
	*home	  = NULL,			   /* home directory */
	*attribline = NULL,                        /* attribution string */
	*lastread = NULL,                          /* name of the lastread file */
	*cfgfile = NULL,			   /* name of the config file */
	*quickbbs = NULL,			   /* where a quickbbs message base is */
	*confmail = NULL;			   /* confmail log file name */

char	*comspec  = NULL;			   /* file spec of command processor */

int 	area = 0,				   /* current area number */
	helpctxt = 0,				   /* current help context */
	aliascount = 0, 			/* how many aliases do you have? */
	lastline = 0,				/* last line in message */
	rm = 300, qm = 300,                     /* the right and quote margins */
	msgids = YES,				/* add msgid lines */
	eids = YES, 				/* add eid lines */
	opusdate = YES, 			/* put in the opus time stamp */
	fidozone = NO,				/* put in fido 12 addressing */
	stripnotes = YES,                       /* strip hidden lines */
	shownotes = YES,                        /* show hidden lines */
	seenbys = YES,                          /* show seenby lines */
	pointnet = 0,				/* private net number of point */
	tabsize = 8,				/* how many spaces for a tab */
	tearline = YES, 			/* show a tearline? */
	domains = 0,				/* how many domains listed */
	confirmations = YES,                    /* confirm deletes, aborts? */
	softcr = YES,                           /* put in soft carriage returns? */
	override = 0,				/* override the area origin line */
	gate = BOTH,				/* zone/domain gate messages? */
	rot13 = NO, 				/* rot13 this message? */
	areas = 0,                              /* how many message areas */
	swapping = YES,
	scanned = 0;				/* areas have been scanned */

int   *messages = NULL;                         /* what messages exist in this area */

COLORS colors = { 0x07,0x0,0x8f,0x0f,0x07,0x70 };

ADDRESS  *alias = NULL;

MSG 	 *message = NULL;

ADDRESS *domain_list = NULL;

ADDRESS uucp_gate;

#else  /* don't use initializers on extern definitions */

extern MSGHANDLE msgdo[];			  /* indexed by msgtype in AREA structure */

extern AREA  __handle *arealist;

extern	char    *username, 		 /* who is you */
                *quotestr,                              /* how to prefix a quote */
		*fidolist,				/* nodelist user list */
		*userlist,				/* personal user list */
		*origin,				/* origin line */
		*outfile,				/* default export filename */
		*attribline,			/* attribution string */
		*home,					/* home directory */
		*lastread,				/* name of the lastread file */
		*cfgfile,				/* name of the config file */
 		*quickbbs,                              /* where a quickbbs message base is */
		*confmail;				/* confmail log file name */

extern char	*comspec;				/* file spec of command processor */

extern	int area,				   /* current area number */
		aliascount, 			/* how many aliases do you have? */
		helpctxt,				/* current help context */
		lastline,				/* last line in message */
		rm,qm,					/* the right and quote margins */
		msgids,eids,			/* add *ID lines */
		fidozone,
		opusdate,
		stripnotes, 			/* strip hidden lines */
		shownotes,				/* show hidden lines */
		seenbys,				/* show seenby lines */
		pointnet,				/* private net number of point */
		tabsize,				/* how many spaces for a tab */
		tearline,				/* show a tearline? */
		domains,				/* how many domains listed */
		confirmations,			/* confirm deletes, aborts? */
		softcr, 				/* put in soft carriage returns? */
		override,				/* override the area origin line */
		gate,					/* zone/domain gate messages? */
		rot13,					/* rot13 this message? */
		areas,                                  /* how many message areas */
		swapping,
		scanned;				/* areas have been scanned */

extern int 	*messages;			   /* what messages exist in this area */

extern  COLORS colors;

extern ADDRESS *alias;

extern MSG	   *message;

extern ADDRESS uucp_gate;

extern ADDRESS *domain_list;

#endif

/* prototypes */

void	_cdecl  main(int argc, char *argv[]);			 /* msged.c 	 */
void	_pascal cleanup(void);
void	_pascal outamemory(void);						 /* msged.c 	 */
MSG    *_pascal readmsg(int n);						 /* readmail.c	 */
int 	_pascal writemsg(MSG *m);						 /* readmail.c	 */
void	_pascal deletemsg(void);						 /* maintmsg.c	 */
int 	_pascal showmsg(MSG *m);						 /* showmail.c	 */
void	_pascal opening(char *,char *);				 /* config.c	 */
void	_pascal parseareas(char *);					 /* areas.c 	 */
int 	_pascal nextmsg(int);							 /* msged.c 	 */
int 	_pascal selectarea(void);						 /* areas.c 	 */
void	_pascal showheader(MSG *m);					 /* showmail.c	 */
ADDRESS _pascal parsenode(char *s);					 /* msged.c 	 */
LINE *	_pascal clearbuffer(LINE *buffer); 			 /* readmail.c	 */
void	_pascal import(LINE *l);						 /* textfile.c	 */
void	_pascal export(LINE *f);						 /* textfile.c	 */
int 	_pascal confirm(void); 						 /* msged.c 	 */
int 	_pascal editheader(void);						 /* makemsg.c	 */
void	_pascal save(MSG *message);					 /* makemsg.c	 */
ADDRESS _pascal lookup(char *name, char *fn);			 /* userlist.c	 */
void	_pascal newmsg(void);							 /* makemsg.c	 */
void	_pascal reply(void);							 /* makemsg.c	 */
void	_pascal quote(void);							 /* makemsg.c	 */
void	_pascal movemsg(void); 						 /* maintmsg.c	 */
void	_pascal writetxt(void);						 /* textfile.c	 */
void	_pascal settings(void);						 /* settings.c	 */
void	_pascal change(void);							 /* makemsg.c	 */
char   *_pascal show_address(ADDRESS a);				 /* showmail.c	 */
char   *_pascal striplwhite(char *s);					 /* config.c	 */
void    _pascal clearmsg(MSG *m);                                                /* readmail.c   */
int 	_pascal setcwd(char *path);					 /* readmail.c	 */
void	_pascal dispose(MSG *message); 				 /* msged.c 	 */
void	_pascal strins(char *l, char c, int x);		 /* string.c	 */
void	_pascal strdel(char *l, int x);				 /* string.c	 */
LINE   *_pascal insline(LINE *nl); 					 /* editmail.c	 */
int 	_pascal dir_findnext(struct _dta * dta);		 /* dir.c		 */
int 	_pascal dir_findfirst(char * filename,
			   int attribute, struct _dta * dta);	   /* dir.c 			   */
#if defined(__ZTC__)
int 	_pascal strncmpi(char *si, char *so, int l);
#endif

/* command key scancode table */

#define DONE	0x001b			/* <ESC> done	*/
#define RUBOUT	0x007f			/* DEL			*/
#define SPACE	0x0020			/* space bar	*/
#define ENTER	0x000d			/* <enter>		*/
#define ROOT	0x4700			/* <Home>		*/
#define LAST	0x4f00			/* <End>		*/
#define SCAN	0x1f00			/* <ALT><S> 	*/

/* various and sundry macros */

#define msg_readtext(n) (msgdo[arealist[area].msgtype].msg_readtext(n))
#define msg_readheader(n) (msgdo[arealist[area].msgtype].msg_readheader(n))
#define msg_writetext(t,n) (msgdo[arealist[area].msgtype].msg_writetext(t,n))
#define msg_writeheader(m) (msgdo[arealist[area].msgtype].msg_writeheader(m))
#define msg_last(a) (msgdo[a.msgtype].msg_last(a))
#define msg_delete(n) (msgdo[arealist[area].msgtype].msg_delete(n))
#define msg_scan(a) (msgdo[(a)->msgtype].msg_scan((a)))
#define msgnum(x) ((arealist[area].msgtype == 0)?messages[x]:(x))
#define release(s) {if (s != NULL) free(s); s = NULL;}
#define DOROT13(c) ((rot13==0)?(c):(rot13==1)?(!isalpha((c)))?(c):((((c) >= (char) 'A') && \
				   ((c) <= (char) 'M')) || (((c) >= (char) 'a') && ((c) <= (char) 'm')))?((c) + (char) 0xd)\
				   :((((c) >= (char) 'N') && ((c) <= (char) 'Z')) || (((c) >= (char) 'n') && ((c) <= (char) 'z')))?\
				   ((c) - (char) 0xd):(c):((c) >= (char) '!') ? ((((c) + (char) 47) > (char) '~') ? ((c) - (char) 47) :\
				   ((c) + (char) 47)) : (c))
