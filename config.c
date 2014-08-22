/*

Title:	MsgEd

File:	config.c

Author: Jim Nutt

Copr:	released into the PUBLIC DOMAIN 30 Jul 1990 by jim nutt

Description:

	finds & reads config files, initializes everything

Support Files:

	msged.h

*/

#define TEXTLEN 512

#define NDEBUG 1
#define NOSPELL

#include "msged.h"

#if defined(__TURBOC__)
#include <dir.h>
#else
#include <direct.h>
#endif

void	_pascal e_assignkey(unsigned int key, char *label);
void	_pascal r_assignkey(unsigned int key, char *label);

static void _pascal checkareas(char *cpath);
static void _pascal set_colors(char *keyword,char *value);
static FILE * _pascal fileopen(char *env, char *cpath);
static void _pascal init(void);
static void _pascal parsemail(char *keyword, char *value);
static void _pascal parseconfig(FILE *fp);
static unsigned int * _pascal parse_macro(char *macro);

char * _pascal striplwhite(char *s)

{
	while (*s && isspace(*s))
		s++;
	return((*s)?s:NULL);
}

static FILE * _pascal fileopen(char *env,char *cfn)

{
	FILE	*fp;
	char	pathname[PATHLEN];
	char	*ename;
	char	*cpath;

	if (cfn == NULL)
		return(NULL);

	if ((fp = fopen(cfn,"rt")) != NULL)
		return(fp);

	ename = strtok(env,";");

	while (ename != NULL) {

		if ((cpath = getenv(ename)) == NULL)
			break;

		strcpy(pathname, cpath);

		if (*(pathname + strlen(pathname) - 1) != '\\')
			strcat(pathname,"\\");

		strcat(pathname,cfn);

		if ((fp = fopen(pathname,"rt")) != NULL)
			return(fp);

		ename = strtok(NULL,";");
	}

	return(NULL);
}

static unsigned int * _pascal parse_macro(char *macro)

{
	unsigned int *m;
	unsigned int *t;
	int l;
	char tmp[5];

	t = m = (unsigned int *) calloc(strlen(macro) * 2,sizeof(int));

	if ((t == (void *) NULL) || (macro == NULL))
		return(NULL);

	while (*macro) {
		if (*macro == '^') {
			*t++ = (unsigned int)(*(macro + 1) == '^')?'^':(toupper(*(macro + 1)) - 64);
			macro += 2;
		}
		else if (*macro == '\\') {
			if (*(macro + 1) == '\\') {
				*t++ = (unsigned int) '\\';
				macro += 2;
			}
			else {
				memset(tmp,0,sizeof tmp);
				strncpy(tmp,macro+1,4);
				*t++ = (unsigned int) strtol(tmp,NULL,0) << 8;
				macro += 5;
			}
		}
		else
			*t++ = (unsigned int) *macro++;
	}

	*t = 0;

	l = (int) (t - m) + 1;

		t = realloc(m, l * sizeof (int));

	if (t)
		return(t);

	return(m);
}

static void _pascal checkareas(char *areafile)

{
	FILE   *fp = NULL;
	char	buffer[TEXTLEN];
	char   *s = NULL;
	int 	flag = 1;

	if ((fp = fileopen("BINKLEY;OPUS;SEADOG;BBS;",areafile)) == NULL)
		if ((fp = fileopen("BINKLEY;OPUS;SEADOG;BBS;","areas.bbs")) == NULL)
			return;

	if (fp != NULL) {

		while (fgets(buffer, TEXTLEN, fp) != NULL) {
			char *tag = NULL;
			char *path = NULL;
			int i = 0;

			if ((*buffer == '\r') || (*buffer == '\n'))
				continue;

			s = striplwhite(buffer);

			if ((*s == ';') || (*s == '-') || (*s == '#'))
				continue;

			if ((strchr(buffer,'!')) && flag) {
				char *p = strrchr(buffer,'!');
				if (p != NULL) {
					*p = '\0';
					if (origin != NULL)
						free(origin);
                                        origin = strdup(buffer);
				}
				flag = 0;
				continue;
			}

			path = strtok(s," \t");
			if (path) strlwr(path);
			tag = strtok(NULL," \t\n");
			if (tag) strlwr(tag);
			flag = 0;

			for (;i < areas; i++)
				if ((arealist[i].path != NULL) &&
					(strcmp(path,arealist[i].path) == 0))
					break;

			if (i == areas) {
				areas++;
				area = areas - 1;
				arealist = handle_realloc(arealist, areas * sizeof(struct _area));

				memset(&(arealist[area]),0,sizeof(struct _area));

				arealist[area].echomail = 1;
				arealist[area].description = strdup(tag);
				arealist[area].tag = arealist[area].description;
				arealist[area].path = strdup(path);
				if (*(arealist[area].path + strlen(arealist[area].path) - 1) == '\\')
					*(arealist[area].path + strlen(arealist[area].path) - 1) = '\0';
			}
			else
				arealist[area].echomail = 1;
		}
	}

	if (fp != NULL)
		fclose(fp);
}

static void _pascal set_colors(char *keyword,char *value)

{
	int 	color = 0, i = 0;
	char   *f = NULL,
		   *b = NULL;
	static char *clrs[] = {
		"bla", "blu", "gre", "cya", "red", "mag", "yel", "whi"
	};

	if (value) strlwr(value);
	f = strtok(value,"/");
	while (isspace(*f)) f++;
	b = strtok(NULL,"\0");
	while (isspace(*b)) b++;

	for (i = 0; (i < 8) && (strncmp(clrs[i],f,3) != 0); i++)
		/* empty loop */ ;
	color = (i < 8)?i:0;
	color |= (strchr(f,'+') != NULL)?8:0;

	for (i = 0; (i < 8) && (strncmp(clrs[i],b,3) != 0); i++)
		/* empty loop */ ;
	color |= (i < 8)?i<<4:0;
	color |= (strchr(b,'+') != NULL)?0x80:0;

	colors.normal = (strncmp("normal", keyword,6))?colors.normal:(unsigned char) color;
	colors.quote = (strncmp("quote", keyword,9))?colors.quote:(unsigned char) color;
	colors.warn = (strncmp("warn", keyword,4))?colors.warn:(unsigned char) color;
	colors.block = (strncmp("block", keyword,5))?colors.block:(unsigned char) color;
	colors.info = (strncmp("info", keyword,4))?colors.info:(unsigned char) color;
	colors.hilite = (strncmp("hilite", keyword,6))?colors.hilite:(unsigned char) color;
}

static void _pascal init()

{
	memset(macros,0,sizeof(macros));
	comspec = getenv("COMSPEC");
#ifndef NOSPELL
	speller = strdup("speller.com");
#endif
	outfile = strdup("prn");
	quotestr = strdup(">");
	attribline = strdup("In a message of <%m %d %h>, %f (%a) writes:\n");
	confmail = strdup("confmail.out");
	lastread = strdup("lastread");
	cfgfile = strdup("msged.cfg");
	uucp_gate.notfound = 1;
#ifdef __OS2__
	videomethod = FOSSIL;
#else	
	videomethod = DIRECT;
#endif
}

static void _pascal parsemail(char *keyword, char *value)

{
	char *s = value;
	char *e = NULL;
	AREA *t;
	AREA a;

	check(username);
	memset(&a,0,sizeof a);

	switch (tolower(*keyword)) { /* one letter is enough for now */
		default  :
		case 'f' : a.msgtype = FIDO; break;
		case 'q' : a.msgtype = QUICK; break;
	}

	while (*s && isspace(*s)) s++;
	if (!*s) return;

	switch (tolower(*s)) { /* one letter is enough */
		case 'n' : a.news = 1; break;
		case 'u' : a.uucp = 1; break;
		case 'm' : a.netmail = 1; break;
		case 'e' : a.echomail = 1; break;
		case 'l' :
		default  : a.local = 1; break;
	}

	while (*s && !isspace(*s)) s++;  /* skip the rest */
	while (*s && isspace(*s)) s++;
	if (!*s) return;

	if (*s != '\"') {
		while (*s && !isspace(*s)) {
			switch (tolower(*s)) {
				case 'p' : a.priv = 1; break;
				case 'h' : a.hold = 1; break;
				case 'd' : a.direct = 1; break;
				case 'c' : a.crash = 1; break;
				case 'k' : a.killsent = 1; break;
				case  0  : return;
			}
			s++;
		}
		while (*s && isspace(*s)) s++;
		if (!*s) return;
	}

	if ((e = strchr(++s,'\"')) == NULL) return;
	*e++ = '\0';
	a.description = strdup(s);
	s = e;

	while (*s && isspace(*s)) s++;
	if (!*s) {
		free(a.description);
		return;
	}

	if ((e = strchr(s,' ')) == NULL)
		e = strchr(s,'\t');

	if (e == NULL)
		e = s;
	else
		*e++ = '\0';

	while (*s && isspace(*s)) s++;
	if (!*s) {
		free(a.description);
		return;
	}

	switch (a.msgtype) {
		default :
		case FIDO  : a.path = strdup(strlwr(s)); break;
		case QUICK : a.board = atoi(s); break;
	}

	if (a.msgtype == FIDO) {
		if (chdir(a.path) != 0) {
			free(a.path);
			free(a.description);
			return;
		}
		else
			setcwd(home);
	}

	if (a.echomail) {
		s = e;
		while (*s && isspace(*s)) s++;
		if (s && *s)
			a.tag = strdup(s);
		else
			a.tag = NULL;
	}

	for (area = 0; area < areas; area++) {
		/* this is a sneaky use of the ternary operator */

		if (((a.msgtype == QUICK) && (arealist[area].msgtype == QUICK))?
			(a.board == arealist[area].board):
			(strcmp(a.path,arealist[area].path)==0)) {
				arealist[area].priv |= a.priv;
				arealist[area].hold |= a.hold;
				arealist[area].direct |= a.direct;
				arealist[area].crash |= a.crash;
				arealist[area].killsent |= a.killsent;
				arealist[area].news |= a.news;
				arealist[area].echomail |= a.echomail;
				arealist[area].uucp |= a.uucp;
				arealist[area].netmail |= a.netmail;
				arealist[area].local |= a.local;

				if (arealist[area].description == NULL)
					arealist[area].description = a.description;
				else if (a.description)
					free(a.description);

				if (arealist[area].tag == NULL)
					arealist[area].tag = a.tag;
				else if (a.tag)
					free(a.tag);

				if ((a.msgtype == FIDO) && (a.path))
					free(a.path);

				return;
		}
	}

	areas++;
	area = areas - 1;

	check(username);

	if (arealist == NULL)
		arealist = handle_malloc(areas * sizeof(struct _area));
	else {
		check(username);
		arealist = handle_realloc(arealist, areas * sizeof(struct _area));
		check(username);
	}

	check(username);

	if (arealist == NULL)
		outamemory();

	check(username);
	checkp(arealist);
	t = arealist + area;
	checkp(t);
	checkp(arealist);
	check(username);
	*t = a;
	checkp(t);
	check(username);
}

static void _pascal parseconfig(FILE *fp)

{
	char	buffer[TEXTLEN];
	char   *keyword = NULL;
	char   *value = NULL;
	char   *s = NULL;

	memset(buffer, 0, TEXTLEN);

	while (!feof(fp)) {

		if (fgets(buffer, TEXTLEN, fp) == NULL)
			return;

		keyword = strtok(buffer, " \t\n\r");
		if (keyword) strlwr(keyword); else continue;
		if ((*keyword) == ';')
			continue;

		value = strtok(NULL, ";\n\r");

		if (value != NULL) {
			s = value + strlen(value) - 1;
			while ((s > value) && isspace(*s))
				*s-- = '\0';
		}

		while (value && *value && isspace(*value))
			if ((*value == '\n') || (*value == ';'))
				break;
			else
				value++;

		if (strcmp("attribution", keyword) == 0) {
			free(attribline);
			if (value)
				attribline = strdup(value);
			else
				attribline = NULL;
			continue;
		}

		if (!value) continue;

		if (strcmp("name",keyword) == 0) {
			check(value);
			username = strdup(value);
			check(username);
		}

		else if (strcmp("include", keyword) == 0) {
			FILE *ifp;
			if ((ifp = fopen(value,"rt")) != NULL) {
				parseconfig(ifp);
				fclose(ifp);
			}
		}

		else if (strcmp("outfile", keyword) == 0) {
			free(outfile);
			outfile = strdup(value);
		}

		else if (strcmp("uucp", keyword) == 0) {
			uucp_gate = parsenode(value);
			uucp_gate.notfound = 0;
		}

		else if (strcmp("lastread", keyword) == 0) {
			free(lastread);
			lastread = strdup(value);
		}

		else if (strcmp("tosslog", keyword) == 0) {
			free(confmail);
			confmail = strdup(value);
		}
#ifndef NOSPELL
		else if (strcmp("speller", keyword) == 0) {
			free(speller);
			speller = strdup(value);
		}
#endif

		else if (strcmp("quickbbs", keyword) == 0) {
			int i;
			if (quickbbs != NULL)
				free(quickbbs);
			quickbbs = strdup(value);
			i = strlen(quickbbs);
			if ((*(quickbbs + i - 1) != '\\') && 
			    (*(quickbbs + i - 1) != '/')) {
				char *s = calloc(1,i+2);
				strcat(strcpy(s,quickbbs),"/");
				free(quickbbs);
				quickbbs = strdup(s);
				free(s);
			}
		}

		else if (strcmp("video",keyword) == 0) {
			if (value) strlwr(value);
			if (strncmp(value,"direct",6) == 0)
				videomethod = DIRECT;
			else if (strncmp(value,"bios",4) == 0)
				videomethod = BIOS;
			else if (strncmp(value,"fossil",6) == 0)
				videomethod = FOSSIL;
			else if (strncmp(value,"ansi",4) == 0)
				videomethod = ANSI;
		}

		else if (strcmp("gate",keyword) == 0) {
			if (gate == 7)
				gate = 0;
			if (value) strlwr(value);
			if (strncmp(value,"zones",5) == 0)
				gate |= GZONES;
			else if (strncmp(value,"domains",7) == 0)
				gate |= GDOMAINS;
			else if (strncmp(value,"both",4) == 0)
				gate = GDOMAINS | GZONES;
			else if (strncmp(value,"none",4) == 0)
				gate = 0;
		}

		else if (strcmp("function",keyword) == 0) {
			int i;
			char *s;

			i = (int) strtol(value,&s,0);
			s = striplwhite(s);

			if ((i >= 0) && (i <= 40))
				macros[i] = parse_macro(s);
		}

		else if (strcmp("userlist", keyword) == 0) {
			fidolist = strdup(strtok(value,",\n"));
			if ((userlist = strtok(NULL,",\n")) != NULL)
				userlist = strdup(userlist);
		}

		else if (strcmp("address", keyword) == 0) {
			if (alias == NULL) {
				alias = (ADDRESS *) calloc(1, sizeof(ADDRESS));
				aliascount = 1;
			}
			else
				alias = (ADDRESS *) realloc(alias,++aliascount * sizeof(ADDRESS));
			alias[aliascount - 1] = parsenode(value);
		}

		else if (strcmp("videoseg", keyword) == 0)
			vbase = (int) strtol(value,NULL,0);

		else if (strcmp("privatenet", keyword) == 0)
			pointnet = (int) strtol(value,NULL,0);

		else if (strcmp("maxx", keyword) == 0)
			maxx = (int) strtol(value,NULL,0);

		else if (strcmp("maxy", keyword) == 0)
			maxy = (int) strtol(value,NULL,0);

		else if (strcmp("tabsize", keyword) == 0)
			tabsize = (int) strtol(value,NULL,0);

		else if (strcmp("right", keyword) == 0)
			rm = (int) strtol(value,NULL,0);

		else if (strcmp("quoteright", keyword) == 0)
			qm = (int) strtol(value,NULL,0);

		else if (strcmp("no",keyword) == 0) {
			if (value) strlwr(value);
			softcr ^=		 (strcmp("softcr", value) == 0);
			seenbys ^=		 (strcmp("seen-bys", value) == 0);
			tearline ^= 	 (strcmp("tearline", value) == 0);
			shownotes ^=	 (strcmp("shownotes", value) == 0);
			confirmations ^= (strcmp("confirm", value) == 0);
			msgids ^=		 (strcmp("msgids",value) == 0);
			stripnotes ^=	 (strcmp("strip",value) == 0);
			opusdate ^=      (strcmp("opusdate",value) == 0);
			swapping ^= (strcmp("swapping",value) == 0);
		}

		else if (!(strcmp("editkey", keyword))||!(strcmp("readkey",keyword))) { /*WRA*/
			int scancode;

			if (value) {
				strlwr(value);
				scancode = (int) strtol(value,&value,0);
				value = striplwhite(value);
			}
			if (*keyword == 'e')
				e_assignkey(scancode,value);
			else
				r_assignkey(scancode,value);
		}

		else if (strcmp("quote", keyword) == 0) {
			char *s;
			if (quotestr != NULL)
				free(quotestr);
			s = quotestr = strdup(value);
			while (*s) {
				*s = (*s == (char) '_')?(char)' ':*s;
				s++;
			}
		}

		else if (strcmp("origin", keyword) == 0) {
			if (origin != NULL)
				free(origin);
			origin = strdup(value);
		}

		else if ((strcmp("quick", keyword) == 0) ||
			 (strcmp("fido", keyword) == 0)) {
			parsemail(keyword,value);
		}

		else if (strcmp("color", keyword) == 0) {
			keyword = strtok(value," \t");
			value	= strtok(NULL,"\0");
			set_colors(keyword,value);
		}

		memset(buffer, 0, TEXTLEN);
	}
}

void _pascal opening(char *cfgfile, char *areafile)
{
	FILE   *fp = NULL;
	int 	count = 0,i;
	char	tmp[PATHLEN];

	init();

	video_init();

	/* start looking for the config file... */

	if ((fp = fileopen("BINKLEY;OPUS;SEADOG;BBS;",cfgfile)) == NULL)
		if ((fp = fileopen("BINKLEY;OPUS;SEADOG;BBS","msged.cfg")) == NULL)
			settings();

	if (fp) {
		parseconfig(fp);
		if (videomethod != BIOS)
			video_init();
	}

	if (opusdate)
		fidozone = NO;

	for (i = 0; i < 40; i++)
		count += (macros[i] != (void *) NULL)?1:0;

	if (fp != NULL)
		fclose(fp);

	rm = (rm > maxx)?maxx:rm;
	qm = (qm > rm)?rm - 5 - strlen(quotestr):qm;

	colors.quote = (colors.quote)?colors.quote:colors.normal;

	checkareas(areafile);

	set_color(colors.normal);
	cls();
	gotoxy(6, 9);
	set_color(colors.hilite);
	bputs("msged FTS Compatible Mail Editor");
	set_color(colors.normal);
	gotoxy(6, 11);
	bputs("version " VERSION " PUBLIC DOMAIN by Jim Nutt");
	gotoxy(6,13);
	bprintf("%d by %d ",maxx,maxy);
	bputs((videomethod==DIRECT)?"direct video":(videomethod==BIOS)?"bios video":(videomethod==ANSI)?"ansi video":"fossil video");

	gotoxy(6, 15);
	bprintf("%s at %s", username, show_address(thisnode));

	if (aliascount > 1)
		bprintf(" (primary)");

	if (thisnode.point)
		bprintf(" (private net %d/%d)", pointnet,
			thisnode.point);

	if (origin != NULL) {
		gotoxy(6,16);
		bputs(origin);
		gotoxy(6,18);
	}
	else
		gotoxy(6,17);

	bprintf("%d message areas found", areas);
	gotoxy(6,wherey() + 1);
	bprintf("%d macros defined",count);
	getcwd(tmp,PATHLEN);
	if (tmp) strlwr(tmp);
	home = strdup(tmp);
	gotoxy(6,wherey() + 2);
	bprintf("home directory is %s",home);

	if (arealist == NULL) {
		puts("Oops! at least one message area must be defined.");
		puts("Exiting...");
		exit(0);
	}
}
