/*

Title:	MsgEd

File:	settings.c

Author: Jim Nutt

Copr:	released into the PUBLIC DOMAIN 30 jul 1990 by jim nutt

Description:

	handles configuration setup from within msged

Support Files:

	msged.h
	menu.h

*/

#define NOSPELL
#include "msged.h"
#include "menu.h"

void	_pascal e_assignkey(unsigned int key, char *label);
char *	_pascal e_getlabels(int i);
char *	_pascal e_getbind(unsigned int key);
void	_pascal r_assignkey(unsigned int key, char *label);
char *	_pascal r_getbind(unsigned int key);
char *	_pascal r_getlabels(int i);

static void _pascal set_colors(void);
static void _pascal set_switch(void);
static void _pascal set_margins(void);
static void _pascal set_files(void);
static void _pascal set_areas(void);
static void _pascal set_gates(void);
static void _pascal set_quotes(void);
static void _pascal set_macro(void);
static void _pascal set_address(void);
static void _pascal set_video(void);
static void _pascal set_misc(void);
static void _pascal save_set(void);
static void _pascal set_attrib(int x1, int y1, int x2, int y2, AREA *a);
static void _pascal edit_area(AREA *a);
static void _pascal set_key(int i);
static unsigned int * _pascal build_macro(void);

void _pascal settings()

{
	static char *s_items[] = {
		"Switches ",
		"Margins  ",
		"Files    ",
		"Attribute",
		"Areas    ",
		"Gates    ",
		"Quoting  ",
		"Macros   ",
		"Addresses",
		"Video    ",
		"Misc     ",
		"Save     ",
		NULL
	};

	int i = 0;

	cls();

	set_color(colors.info);
	gotoxy(2,2); bputs("msged version " VERSION " setup.");

	for (;;) switch (i = menu(5,5,15,16,s_items,colors.hilite,colors.normal,i)) {
		case -1:set_color(colors.normal);
			return;
		case  0:
			set_switch();
			break;
		case  1:
			set_margins();
			break;
		case  2:
			set_files();
			break;
		case  3:
			gotoxy(20,7);
			if (areas < 1) {
				bputs("No areas defined, press a key");
				getkey();
			}
			else {
				bprintf("Default message attributes for %s",arealist[area].description);
				set_attrib(20,9,35,14,&arealist[area]);
			}
			clrwnd(20,7,maxx,7);
			break;
		case  4:
			set_areas();
			break;
		case  5:
			set_gates();
			break;
		case  6:
			set_quotes();
			break;
		case  7:
			set_macro();
			break;
		case  8:
			set_address();
			break;
		case  9:
			set_video();
			break;
		case 10:
			set_misc();
			break;
		case 11:
			save_set();
			break;
		default:
			break;
	}
}

static void _pascal set_switch()

{
	static char *switches[] = {
		"Soft Returns off",
		"Seen-Bys off",
		"Tearlines off",
		"Kludge line display off",
		"Confirm Deletes off",
		"MSGIDs off",
		"Strip Kludges off",
		"Opus Dates off",
		"Swapping off",
#ifndef NOSPELL
		"Spell Check off",
#endif
		NULL
	};

	int i = 0;

	for (;;) {

		strcpy(switches[0]+13,softcr?"on ":"off");
		strcpy(switches[1]+9,seenbys?"on ":"off");
		strcpy(switches[2]+10,tearline?"on ":"off");
		strcpy(switches[3]+20,shownotes?"on ":"off");
		strcpy(switches[4]+16,confirmations?"on ":"off");
		strcpy(switches[5]+7,msgids?"on ":"off");
		strcpy(switches[6]+14,stripnotes?"on ":"off");
		strcpy(switches[7]+11,opusdate?"on ":"off");
		strcpy(switches[8]+9,swapping?"on ":"off");
#ifndef NOSPELL
		strcpy(switches[8]+12,spell?"on ":"off");
#endif

		switch (i = menu(20,6,45,17,switches,colors.hilite,colors.normal,i)) {
			case -1:set_color(colors.normal);
				clrwnd(20,6,45,17);
				return;
			case 0: softcr = !softcr;
				break;
			case 1: seenbys = !seenbys;
				break;
			case 2: tearline = !tearline;
				break;
			case 3: shownotes = !shownotes;
				break;
			case 4: confirmations = !confirmations;
				break;
			case 5: msgids = !msgids;
				break;
			case 6: stripnotes = !stripnotes;
				break;
			case 7: opusdate = !opusdate;
				break;
			case 8: swapping = !swapping;
				break;
#ifndef NOSPELL
			case 8: spell = !spell;
				break;
#endif
			default: break;
		}
	}

}

static void _pascal set_margins()

{
	static char *items[] = {
		"Right margin is xxx",
		"Quote margin is xxx",
		"Tab size is xx",
		NULL
	};

	int i = 0;

	for (;;) {
		sprintf(items[0]+16,"%-3d",rm);
		sprintf(items[1]+16,"%-3d",qm);
		sprintf(items[2]+12,"%-2d",tabsize);

		switch(i = menu(20,7,40,10,items,colors.hilite,colors.normal,i)) {
			case -1:
				set_color(colors.normal);
				clrwnd(20,7,maxx,10);
				return;
			case  0:
				set_color(colors.hilite);
				gotoxy(36,wherey());
				rm = getnum(5,maxx,rm);
				break;
			case  1:
				set_color(colors.hilite);
				gotoxy(36,wherey());
				qm = getnum(5,rm,qm);
				break;
			case  2:
				set_color(colors.hilite);
				gotoxy(32,wherey());
				tabsize = getnum(1,rm,tabsize);
				break;
		}
	}
}

static void _pascal set_files()

{
	char *item[9];
	int i,f=0, j = 0;


	while (f != ABORT) {
		memset(item,0,sizeof item);
		for (i = 0; i < 8; i++)
			item[i] = calloc(1,128);
		item[8] = NULL;

		sprintf(item[0],"Outfile is %s",outfile?outfile:"");
		sprintf(item[1],"Lastread file is %s",lastread?lastread:"");
		sprintf(item[2],"Echomail toss log is %s",confmail?confmail:"");
		sprintf(item[3],"Quickbbs directory is %s",quickbbs?quickbbs:"");
		sprintf(item[4],"Primary userlist is %s",fidolist?fidolist:"");
		sprintf(item[5],"Secondary userlist is %s",userlist?userlist:"");
		sprintf(item[6],
#ifndef NOSPELL
			"Spell Checker is %s",speller);
		sprintf(item[7],
#endif
			"Config file is %s",cfgfile);
#ifdef NOSPELL
		free(item[7]); item[7] = NULL;
#endif

		switch(j = menu(20,8,maxx,15,item,colors.hilite,colors.normal,j)) {
			case -1:
				f = ABORT;
				break;
			case  0:
				if (outfile) free(outfile);
				gotoxy(31,wherey()); set_color(colors.hilite);
				f = bgets(item[0]+11,64,maxx-31);
				outfile = strdup(item[0]+11);
				break;
			case 1:
				if (lastread) free(lastread);
				gotoxy(37,wherey()); set_color(colors.hilite);
				f = bgets(item[1]+17,64,maxx-37);
				lastread = strdup(item[1]+17);
				break;
			case 2:
				if (confmail) free(confmail);
				gotoxy(41,wherey()); set_color(colors.hilite);
				f = bgets(item[2]+21,64,maxx-41);
				confmail = strdup(item[2]+21);
				break;
			case 3:
				if (quickbbs) free(quickbbs);
				gotoxy(42,wherey()); set_color(colors.hilite);
				f = bgets(item[3]+22,64,maxx-42);
				quickbbs = strdup(item[3]+22);
				break;
			case 4:
				if (fidolist) free(fidolist);
				gotoxy(40,wherey()); set_color(colors.hilite);
				f = bgets(item[4]+20,64,maxx-40);
				fidolist = strdup(item[4]+20);
				break;
			case 5:
				if (userlist) free(userlist);
				gotoxy(42,wherey()); set_color(colors.hilite);
				f = bgets(item[5]+22,64,maxx-42);
				userlist = strdup(item[5]+22);
				break;
			case 6:
#ifndef NOSPELL
				if (speller) free(speller);
				gotoxy(37,wherey()); set_color(colors.hilite);
				f = bgets(item[6]+17,64,maxx-37);
				speller = strdup(item[6]+17);
				break;
			case 7:
#endif
				if (cfgfile) free(cfgfile);
				gotoxy(35,wherey()); set_color(colors.hilite);
#ifndef NOSPELL
				f = bgets(item[7]+15,64,maxx-35);
				cfgfile = strdup(item[7]+15);
#else
				f = bgets(item[6]+15,64,maxx-35);
				cfgfile = strdup(item[6]+15);
#endif
				break;
		}
	}

	for (i = 0; 
#ifndef NOSPELL
		i < 8; 
#else 
		i < 7;
#endif
			i++) {
		free(item[i]);
	}

	set_color(colors.normal);
	clrwnd(20,8,maxx,15);
}

static void _pascal set_attrib(int x1, int y1, int x2, int y2, AREA *a)

{
	static char *items[] = {
		"Privileged o",
		"Hold       o",
		"Direct     o",
		"Crash      o",
		"Kill/Sent  o",
		NULL
	};

	int i = 0;

	for (;;) {

		*(items[0]+11) = (char) ((a->priv)?0xfb:0x20);
		*(items[1]+11) = (char) ((a->hold)?0xfb:0x20);
		*(items[2]+11) = (char) ((a->direct)?0xfb:0x20);
		*(items[3]+11) = (char) ((a->crash)?0xfb:0x20);
		*(items[4]+11)	= (char) ((a->killsent)?0xfb:0x20);

		switch(i = menu(x1,y1,x2,y2,items,colors.hilite,colors.normal,i)) {
			case -1:
				set_color(colors.normal);
				clrwnd(x1,y1,x2,y2);
				return;
			case  0:
				arealist[area].priv = !arealist[area].priv;
				break;
			case  1:
				arealist[area].hold = !arealist[area].hold;
				break;
			case  2:
				arealist[area].direct = !arealist[area].direct;
				break;
			case  3:
				arealist[area].crash = !arealist[area].crash;
				break;
			case  4:
				arealist[area].killsent = !arealist[area].killsent;
				break;
		}
	}
}

static void _pascal set_areas()

{
	AREA __handle *a;

	static char *flist[] = {
		"Add an area",
		"Edit an area",
		"Delete an area",
		NULL
	};

	char **list = NULL;
	char * t = flist[1];
	int i1 = 0, j = 0, i;

	for (;;) {

		clrwnd(20,5,33,7);
		if (areas < 1) flist[1] = NULL;
		else flist[1] = t;

		switch (j = menu(20,5,33,7,flist,colors.hilite,colors.normal,j)) {
			case -1:
				set_color(colors.normal);
				clrwnd(20,5,33,7);
				return;

			case  0:
				set_color(colors.normal);
				clrwnd(20,5,33,7);
				areas++;
				a = arealist;
				arealist = handle_realloc(arealist,sizeof(AREA) * areas);
				if (arealist == NULL)
					arealist = a;
				a = arealist+areas-1;
				memset(a,0,sizeof(AREA));
				edit_area(a);
				break;

			case  1:
				set_color(colors.normal);
				clrwnd(20,5,33,7);
				list = calloc(areas+1,sizeof(char *));
				for (i = 0; i < areas; i++)
					list[i] = arealist[i].description?arealist[i].description:"";
				i1 = i = menu(20,5,maxx,min(maxy,areas+5),list,colors.hilite,colors.normal,i1);
				set_color(colors.normal); clrwnd(20,5,maxx,min(maxy,areas+5));
				if (i == -1)
					break;
				a = arealist+i;
				edit_area(a);
				free(list);
				break;
			case  2:
				set_color(colors.normal);
				clrwnd(20,5,33,7);
				list = calloc(areas+1,sizeof(char *));
				for (i = 0; i < areas; i++)
					list[i] = arealist[i].description?arealist[i].description:"";
				i = menu(20,5,maxx,min(maxy,areas+5),list,colors.hilite,colors.normal,0);
				set_color(colors.normal); clrwnd(20,5,maxx,min(maxy,areas+5));
				if (i == -1)
					break;
				free(list);
				if (arealist[i].msgtype == FIDO)
					free(arealist[i].path);
				if (arealist[i].description)
					free(arealist[i].description);
				if (arealist[i].echomail)
					free(arealist[i].tag);
				while ((i+1) < areas) {
					arealist[i] = arealist[i+1];
					i++;
				}
				areas--;
				a = handle_realloc(arealist,sizeof(AREA) * areas);
				if (a)
					arealist = a;
				break;
		}
	}
}

static void _pascal set_gates()

{
	static char *list[] = {
		"UUCP gateway",
		"Set gating method",
		"Add a domain gate",
		"Delete a domain gate",
		"Change a domain gate",
		NULL
	};

	static char *glist[] = {
		"Neither",
		"Domains",
		"Zones",
		"Both",
		NULL
	};

	char buffer[128];
	int i, j = 0, k = 0, l = 0, m = 0;
	char **alist;
	ADDRESS *a;
	char * t = list[3];
	int f = 0;

	while (f != ABORT) {

		clrwnd(20,11,40,15);
		if (domains < 1) list[3] = NULL;
		else list[3] = t;

		alist = calloc(domains+1,sizeof (char *));
		for (i = 0; i < domains; i++)
			alist[i] = strdup(show_address(domain_list[i]));
		alist[i] = NULL;

		switch(j = menu(20,11,40,15,list,colors.hilite,colors.normal,j)) {
			case -1:
				f = ABORT;
					break;
			case 0:
				if (uucp_gate.notfound)
					memset(buffer,0,sizeof buffer);
				else
					strcpy(buffer,show_address(uucp_gate));

				gotoxy(20,17);
				f = bgets(buffer,sizeof buffer - 1, maxx - 20);
				set_color(colors.normal); clrwnd(20,17,maxx,17);
				if (uucp_gate.domain)
					free(uucp_gate.domain);
				uucp_gate = parsenode(buffer);
				break;
			case 2:
				domains++;
				a = realloc(domain_list,sizeof(ADDRESS) * domains);
				if (a != NULL)
					domain_list = a;
				else
					break;
				gotoxy(20,19);
				bputs("Gate to be added:");
				gotoxy(20,20); memset(buffer,0,sizeof buffer);
				f = bgets(buffer,sizeof(buffer)-1,maxx-20);
				domain_list[domains-1] = parsenode(buffer);
				set_color(colors.normal); clrwnd(20,19,maxx,20);
				break;
			case 3:
				if ((domain_list == NULL) || (domains < 1))
					break;
				k = i = menu(40,15,70,20,alist,colors.hilite,colors.normal,k);
				set_color(colors.normal); clrwnd(40,15,70,20);
				if (i < 0)
					break;
				if (domain_list[i].domain)
					free(domain_list[i].domain);
				while (i+1 < domains) {
					domain_list[i] = domain_list[i+1];
					i++;
				}
				domains--;
				free(alist[domains]);
				break;
			case 4:
				if ((domain_list == NULL) || (domains < 1))
					break;
				l = i = menu(40,15,70,20,alist,colors.hilite,colors.normal,l);
				set_color(colors.normal); clrwnd(40,15,70,20);
				if (i < 0)
					break;
				strcpy(buffer,show_address(domain_list[i]));
				if (domain_list[i].domain)
					free(domain_list[i].domain);
				gotoxy(20,19);
				bputs("Gate to change:");
				gotoxy(20,20);
				f = bgets(buffer,sizeof buffer - 1, maxx-20);
				domain_list[i] = parsenode(buffer);
				set_color(colors.normal); clrwnd(20,19,maxx,20);
				break;
			case 1:
				m = i = menu(45,16,60,19,glist,colors.hilite,colors.normal,m);
				set_color(colors.normal); clrwnd(45,16,60,19);
				if (i > -1)
					gate = i;
				break;
		}
		for (i = 0; i < domains; i++)
			free(alist[i]);
		free(alist);
	}

	set_color(colors.normal);
	clrwnd(20,11,40,15);
}

static void _pascal set_quotes()

{
	char *list[3];
	int f = 0;
	int i = 0;

	list[0] = (char *) malloc(128);
	list[1] = (char *) malloc(256);
	list[2] = NULL;

	while (f != ABORT) {
		sprintf(list[0],"Quote string is %s",quotestr?quotestr:"");
		sprintf(list[1],"Attribution line is %s",attribline?attribline:"");

		switch (i = menu(20,12,maxx,13,list,colors.hilite,colors.normal,i)) {
			case -1:
				f = ABORT;
				break;
			case  0:
				if (quotestr) free(quotestr);
				gotoxy(36,wherey()); set_color(colors.hilite);
				f = bgets(list[0]+16,30,maxx-36);
				quotestr = strdup(list[0]+16);
				break;

			case  1:
				if (attribline) free(attribline);
				gotoxy(40,wherey()); set_color(colors.hilite);
				f = bgets(list[1]+20,128,maxx-40);
				attribline = strdup(list[1]+20);
				break;

		}
	}
	free(list[0]);
	free(list[1]);

	set_color(colors.normal);
	clrwnd(20,12,maxx,13);
}

static void _pascal set_macro()

{
	static char *list[] = {
		"Macro key definition",
		"Command redefinition",
		NULL
	};

	static char *items[] = {
		"Message reader",
		"Editor",
		NULL
	};

	int i= 0,j = 0, i1 = 0;

	for (;;) {
		switch(j = menu(20,13,40,14,list,colors.hilite,colors.normal,j)) {
			case -1:
				set_color(colors.normal);
				clrwnd(20,13,40,14);
				return;
			case  0:
				gotoxy(45,14);
				bputs("Function to program: ");
				i = getnum(0,40,0);
				if (macros[i])
					free(macros[i]);
				macros[i] = build_macro();
				set_color(colors.normal);
				clrwnd(45,14,maxx,14);
				clrwnd(5,18,maxx,20);
				break;

			case  1:
				while ((i1 = menu(45,14,60,15,items,colors.hilite,colors.normal,i1)) != -1)
					set_key(i1);
				set_color(colors.normal);
				clrwnd(45,14,60,15);
				break;
		}
	}
}

static unsigned int * _pascal build_macro()

{
	unsigned int key = 0;
	int stat = 0;
	unsigned int mac[80];
	unsigned int *t;
	int c=0;

	gotoxy(5,18); bputs("Enter macro, press <ESC> twice to finish");
	gotoxy(5,19);

	for (;;) {
		video_update();
		key = getkey();
		if (key == ABORT) {
			if (stat) {
				c--;
				t = calloc(c+1,sizeof (int));
				memcpy(t,mac,c * sizeof (int));
				return(t);
			}
			else
				stat = 1;
		}
		else
			stat = 0;

		mac[c++] = key;

		if (key < 32) {
			bputc('^');
			bputc(key+96);
		}
		else if (key & 0xff00)
			bprintf("\\0x%02x",key>>8);
		else
			bputc(key);

		if (c == 40)
			gotoxy(5,20);

		if (c > 78) {
			t = calloc(c+1,sizeof (int));
			memcpy(t,mac,c * sizeof (int));
			return(t);
		}
	}
}

static void _pascal set_key(int t)

{
	char **clist;
	int i,j,k = 0;
	unsigned int key;

	i = 0;
	while ((t?e_getlabels(i):r_getlabels(i)) != NULL)
		i++;

	clist = (char **) calloc(i+1,sizeof(char *));
	for (j = 0; j <= i; j++)								  /*WRA*/
		clist[j] = t?e_getlabels(j):r_getlabels(j);


	for (;;) {
		k = menu(62,15,75,20,clist,colors.hilite,colors.normal,k);
		set_color(colors.normal); clrwnd(62,15,75,20);
		if (k > -1) {
			gotoxy(62,15); bputs("Assign "); bputs(clist[k]);
				       bputs(" to: (press the key)");
			gotoxy(62,16);
			video_update();

			key = getkey();
			bprintf("0x%4x",key);

			if (t)
				e_assignkey(key,clist[k]);
			else
				r_assignkey(key,clist[k]);

			set_color(colors.normal);
			clrwnd(62,15,maxx,16);
		}
		else
			break;
	}

	free(clist);
}

static void _pascal set_address()

{
	static char *list[] = {
		"Add an alias",
		"Set private net",
		"Delete an alias",
		"Change an alias",
		NULL
	};

	char buffer[128];
	char **alist;
	int i, i1 = 0, j = 0;
	ADDRESS *a;
	char * t = list[2];
	int f = 0;

	while (f != ABORT) {

		alist = calloc(aliascount+1,sizeof (char *));
		for (i = 0; i < aliascount; i++)
			alist[i] = strdup(show_address(alias[i]));
		alist[i] = NULL;

		clrwnd(20,14,40,17);
		if (aliascount < 1) list[1] = NULL;
		else list[2] = t;

		switch(j = menu(20,14,40,17,list,colors.hilite,colors.normal,j)) {
			case -1:
				f = ABORT;
				break;
			case  2:
				if (aliascount == 1) {
					set_color(colors.warn);
					gotoxy(40,10);
					bputs("Can't delete only alias!");
					gotoxy(40,11);
					bputs("press any key to continue");
					getkey();
					set_color(colors.normal);
					clrwnd(40,10,80,11);
					break;
				}
				i1 = i = menu(40,15,70,20,alist,colors.hilite,colors.normal,i1);
				set_color(colors.normal); clrwnd(40,15,70,20);
				if (i < 0)
					break;
				if (alias[i].domain)
					free(alias[i].domain);
				while (i+1 < aliascount) {
					alias[i] = alias[i+1];
					i++;
				}
				aliascount--;
				free(alist[aliascount]);
				break;
			case  0:
				aliascount++;
				a = realloc(alias,sizeof(ADDRESS) * aliascount);
				if (a != NULL)
					alias = a;
				else
					break;
				gotoxy(20,19);
				bputs("Alias to be added:");
				gotoxy(20,20); memset(buffer,0,sizeof buffer);
				f = bgets(buffer,sizeof(buffer)-1,maxx-20);
				alias[aliascount-1] = parsenode(buffer);
				set_color(colors.normal); clrwnd(20,19,maxx,20);
				break;
			case  3:
				j = i = menu(40,15,70,20,alist,colors.hilite,colors.normal,j);
				set_color(colors.normal); clrwnd(40,15,70,20);
				if (i < 0)
					break;
				strcpy(buffer,show_address(alias[i]));
				if (alias[i].domain)
					free(alias[i].domain);
				gotoxy(20,19);
				bputs("Alias to change:");
				gotoxy(20,20);
				f = bgets(buffer,sizeof buffer - 1, maxx-20);
				alias[i] = parsenode(buffer);
				set_color(colors.normal); clrwnd(20,19,maxx,20);
				break;
			case  1:
				gotoxy(40,15); bputs("Privatenet number: ");
				pointnet = getnum(0,32767,pointnet);
				set_color(colors.normal); clrwnd(40,15,maxx,15);
				break;
		}
		for (i = 0; i < aliascount; i++)
			free(alist[i]);
		free(alist);
	}
	clrwnd(20,14,40,17);
}

static void _pascal set_video()

{
	char *list[5];
	int i, j = 0, k = 0;

	static char *vlist[] = {
		"Direct",
		"BIOS",
		"FOSSIL",
		NULL
	};

	for (i = 0; i < 4; i++)
		list[i] = (char *) malloc(32);
	list[4] = NULL;

	for (;;) {

		sprintf(list[0],"Screen width is %d",maxx);
		sprintf(list[1],"Height is %d",maxy);
		strcpy(list[2],"Colors");
		sprintf(list[3],"Video %s",(videomethod==DIRECT)?"direct":(videomethod==BIOS)?"bios":(videomethod==ANSI)?"ERROR":"fossil");

		switch (j = menu(20,15,40,18,list,colors.hilite,colors.normal,j)) {
			case -1:
				for (i = 0; i < 4; i++)
					free(list[i]);
				set_color(colors.normal);
				clrwnd(20,15,40,18);
				return;
			case 0:
				gotoxy(36,wherey()); set_color(colors.hilite);
				maxx = getnum(0,1000,maxx);
				break;
			case 1:
				gotoxy(30,wherey()); set_color(colors.hilite);
				maxy = getnum(0,1000,maxy);
				break;
			case 2:
				set_colors();
				break;
			case 3:
				k = i = menu(45,15,55,19,vlist,colors.hilite,colors.normal,k);
				set_color(colors.normal); clrwnd(45,15,55,18);
				if (i == -1)
					return;
				videomethod = i;
				video_init();
				break;
		}
	}

}

static void _pascal set_colors()

{
	static char *clist[] = {
		"Black", "Blue", "Green", "Cyan",
		"Red", "Magenta", "Brown", "White",
		"Dark Grey", "Lt Blue", "Lt Green", "Lt Cyan",
		"Lt Red", "Lt Magenta", "Yellow", "White", NULL
	};

	static char *fblist[] = {
		"Foreground", "Background", NULL
	};

	static char *tlist[] = {
		"Normal", "Warnings", "Quotes",
		"Block Anchor", "Information",
		"Highlight", NULL
	};

	int which = 0;
	int fb = 0;
	int co = 0;
	int mask = 0;


	for (;;) {
		gotoxy(40,5); set_color(colors.normal);
		bputs("Normal text");
		gotoxy(40,6); set_color(colors.warn);
		bputs("Warning messages");
		gotoxy(40,7); set_color(colors.quote);
		bputs("Quoted text");
		gotoxy(40,8); set_color(colors.block);
		bputs("Block anchor");
		gotoxy(40,9); set_color(colors.info);
		bputs("Information");
		gotoxy(40,10); set_color(colors.hilite);
		bputs("Highlighted text");
		set_color(colors.normal);

		which = menu(40,17,52,22,tlist,colors.hilite,colors.normal,which);
		if (which == -1) {
			set_color(colors.normal);
			clrwnd(40,5,60,10);
			clrwnd(40,17,52,22);
			return;
		}

		do {
			fb = menu(55,18,65,19,fblist,colors.hilite,colors.normal,fb);
			if (fb != -1) {
				co = menu(67,19,77,24,clist,colors.hilite,colors.normal,co);
				if (co != -1) {
					if (fb) {
						co <<= 4;
						mask = 0x0f;
					}
					else
						mask = 0xf0;

					switch (which) {
						case 0: colors.normal &= mask;
							colors.normal |= co;
							break;
						case 1: colors.warn &= mask;
							colors.warn |= co;
							break;
						case 2: colors.quote &= mask;
							colors.quote |= co;
							break;
						case 3: colors.block &= mask;
							colors.block |= co;
							break;
						case 4: colors.info &= mask;
							colors.info |= co;
							break;
						case 5: colors.hilite &= mask;
							colors.hilite |= co;
							break;
					}
				}
				set_color(colors.normal);
				clrwnd(55,18,77,24);
			}
			else {
				set_color(colors.normal);
				clrwnd(55,18,65,19);
			}

			gotoxy(40,5); set_color(colors.normal);
			bputs("Normal text");
			gotoxy(40,6); set_color(colors.warn);
			bputs("Warning messages");
			gotoxy(40,7); set_color(colors.quote);
			bputs("Quoted text");
			gotoxy(40,8); set_color(colors.block);
			bputs("Block anchor");
			gotoxy(40,9); set_color(colors.info);
			bputs("Information");
			gotoxy(40,10); set_color(colors.hilite);
			bputs("Highlighted text");
			set_color(colors.normal);

		} while (fb != -1);
	}
	cls();
}

static void _pascal set_misc()

{
	char *list[3];
	int i;
	int f = 0;
	int j = 0;

	for (i = 0; i < 2; i++)
		list[i] = (char *) malloc(128);
	list[2] = NULL;

	while (f != ABORT) {
		sprintf(list[0],"Your name is %s",username?username:"");
		sprintf(list[1],"Origin line is %s",origin?origin:"");

		switch (j = menu(20,16,maxx,17,list,colors.hilite,colors.normal,j)) {
			case -1:
				f = ABORT;
				break;
			case  0:
				if (username) free(username);
				gotoxy(33,wherey()); set_color(colors.hilite);
				f = bgets(list[0]+13,80,maxx-33);
				username = strdup(list[0]+13);
				break;
			case  1:
				if (origin) free(origin);
				gotoxy(35,wherey()); set_color(colors.hilite);
				f = bgets(list[1]+15,65,maxx-35);
				origin = strdup(list[1]+15);
				break;

		}
	}

	for (i = 0; i < 3; i++)
		free(list[i]);

	set_color(colors.normal);
	clrwnd(20,16,maxx,17);
}

static void _pascal save_set()

{
	FILE *fp;
	int i;
	char *s;
	time_t ntime;
	unsigned int key;
	unsigned int *keys;

	static char *clrs[] = {
		"bla", "blu", "gre", "cya",
		"red", "mag", "yel", "whi"
	};

	if (*cfgfile == '+')
		fp = fopen(cfgfile+1,"a");
	else
		fp = fopen(cfgfile,"w");

	time(&ntime);

	fputs("\n; msged " VERSION " config file\n",fp);
	fputs("; generated ",fp);
	fputs(ctime(&ntime),fp);

	fputs("\n; who you are\n\n",fp);

	fprintf(fp,"name %s\n\n", username?username:"");

	fputs("; where you are\n\n",fp);

	for (i = 0; i < aliascount; i++)
		fprintf(fp,"address %s\n",show_address(alias[i]));

	if (pointnet)
		fprintf(fp,"privatenet %d\n",pointnet);

	fputs("\n; default file names\n\n",fp);

	if (quickbbs) {
		if (*(quickbbs+strlen(quickbbs)-1) == '\\')
			*(quickbbs+strlen(quickbbs)-1) = '\0';
		fprintf(fp,"quickbbs %s\n",quickbbs);
		strcat(quickbbs,"\\");
	}

	fprintf(fp,"outfile %s\n",outfile?outfile:"");
	fprintf(fp,"lastread %s\n",lastread?lastread:"");
	fprintf(fp,"tosslog %s\n",confmail?confmail:"");
	fprintf(fp,"userlist %s,%s\n\n",fidolist?fidolist:"",userlist?userlist:"");

	fputs("; how you like your quotes\n\n",fp);

	fprintf(fp,"attribution %s\n",attribline?attribline:"");
	fputs("quote ",fp);
	s = quotestr;
	while (*s) {
		if (*s == ' ')
			fputc('_',fp);
		else
			fputc(*s,fp);
		s++;
	}

	fputs("\n\n; switch settings\n\n",fp);

	if (!softcr) fputs("no softcr\n",fp);
	if (!seenbys) fputs("no seen-bys\n",fp);
	if (!tearline) fputs("no tearline\n",fp);
	if (!shownotes) fputs("no shownotes\n",fp);
	if (!confirmations) fputs("no confirm\n",fp);
	if (!msgids) fputs("no msgids\n",fp);
	if (!stripnotes) fputs("no strip\n",fp);
	if (!opusdate) fputs("no opusdate\n",fp);
	if (!swapping) fputs("no swapping\n",fp);

	fputs("\n; video settings\n\n",fp);

	fprintf(fp,"video %s\n",(videomethod==DIRECT)?"direct":(videomethod==BIOS)?"bios":(videomethod==ANSI)?"ansi":"fossil");

	fprintf(fp,"color normal %s%c/%s%c\n",
		clrs[colors.normal & 0x07],
		(colors.normal & 0x08)?'+':' ',
		clrs[(colors.normal >> 4) & 0x07],
		(colors.normal & 0x80)?'+':' ');

	fprintf(fp,"color warn %s%c/%s%c\n",
		clrs[colors.warn & 0x07],
		(colors.warn & 0x08)?'+':' ',
		clrs[(colors.warn >> 4) & 0x07],
		(colors.warn & 0x80)?'+':' ');

	fprintf(fp,"color quote %s%c/%s%c\n",
		clrs[colors.quote & 0x07],
		(colors.quote & 0x08)?'+':' ',
		clrs[(colors.quote >> 4) & 0x07],
		(colors.quote & 0x80)?'+':' ');

	fprintf(fp,"color block %s%c/%s%c\n",
		clrs[colors.block & 0x07],
		(colors.block & 0x08)?'+':' ',
		clrs[(colors.block >> 4) & 0x07],
		(colors.block & 0x80)?'+':' ');

	fprintf(fp,"color info %s%c/%s%c\n",
		clrs[colors.info & 0x07],
		(colors.info & 0x08)?'+':' ',
		clrs[(colors.info >> 4) & 0x07],
		(colors.info & 0x80)?'+':' ');

	fprintf(fp,"color hilite %s%c/%s%c\n",
		clrs[colors.hilite & 0x07],
		(colors.hilite & 0x08)?'+':' ',
		clrs[(colors.hilite >> 4) & 0x07],
		(colors.hilite & 0x80)?'+':' ');

	fputs("\n; margins and tabs\n\n",fp);

	fprintf(fp,"right %d\nquoteright %d\ntabsize %d\n",
		rm,qm,tabsize);

	fputs("\n; all about your areas\n\n",fp);

	for (i = 0; i < areas; i++) {
		if (arealist[i].msgtype == FIDO) {
			if (arealist[i].local) {
				fputs("fido local ",fp);
				if (arealist[i].priv) fputc('p',fp);
				if (arealist[i].hold) fputc('h',fp);
				 if (arealist[i].direct) fputc('d',fp);
				if (arealist[i].crash) fputc('c',fp);
				if (arealist[i].killsent) fputc('k',fp);
				fprintf(fp," \"%s\" %s\n",arealist[i].description?arealist[i].description:"",
							  arealist[i].path?arealist[i].path:"");
			}

			if (arealist[i].netmail) {
				fputs("fido mail ",fp);
				if (arealist[i].priv) fputc('p',fp);
				if (arealist[i].hold) fputc('h',fp);
				if (arealist[i].direct) fputc('d',fp);
				if (arealist[i].crash) fputc('c',fp);
				if (arealist[i].killsent) fputc('k',fp);
				fprintf(fp," \"%s\" %s\n",arealist[i].description?arealist[i].description:"",
							  arealist[i].path?arealist[i].path:"");
			}

			if (arealist[i].echomail) {
				fputs("fido echo ",fp);
				if (arealist[i].priv) fputc('p',fp);
				if (arealist[i].hold) fputc('h',fp);
				if (arealist[i].direct) fputc('d',fp);
				if (arealist[i].crash) fputc('c',fp);
				if (arealist[i].killsent) fputc('k',fp);
				fprintf(fp," \"%s\" %s\n",arealist[i].description?arealist[i].description:"",
							  arealist[i].path?arealist[i].path:"");
			}

			if (arealist[i].news) {
				fputs("fido news ",fp);
				if (arealist[i].priv) fputc('p',fp);
				if (arealist[i].hold) fputc('h',fp);
				if (arealist[i].direct) fputc('d',fp);
				if (arealist[i].crash) fputc('c',fp);
				if (arealist[i].killsent) fputc('k',fp);
				fprintf(fp," \"%s\" %s\n",arealist[i].description?arealist[i].description:"",
							  arealist[i].path?arealist[i].path:"");
			}

			if (arealist[i].uucp) {
				fputs("fido uucp ",fp);
				if (arealist[i].priv) fputc('p',fp);
				if (arealist[i].hold) fputc('h',fp);
				if (arealist[i].direct) fputc('d',fp);
				if (arealist[i].crash) fputc('c',fp);
				if (arealist[i].killsent) fputc('k',fp);
				fprintf(fp," \"%s\" %s\n",arealist[i].description?arealist[i].description:"",
							  arealist[i].path?arealist[i].path:"");
			}
		}
		if (arealist[i].msgtype == QUICK) {
			if (arealist[i].local) {
				fputs("quick local ",fp);
				if (arealist[i].priv) fputc('p',fp);
				if (arealist[i].hold) fputc('h',fp);
				if (arealist[i].direct) fputc('d',fp);
				if (arealist[i].crash) fputc('c',fp);
				if (arealist[i].killsent) fputc('k',fp);
				fprintf(fp," \"%s\" %d\n",arealist[i].description?arealist[i].description:"",
							  arealist[i].board);
			}

			if (arealist[i].netmail) {
				fputs("quick mail ",fp);
				if (arealist[i].priv) fputc('p',fp);
				if (arealist[i].hold) fputc('h',fp);
				if (arealist[i].direct) fputc('d',fp);
				if (arealist[i].crash) fputc('c',fp);
				if (arealist[i].killsent) fputc('k',fp);
				fprintf(fp," \"%s\" %d\n",arealist[i].description?arealist[i].description:"",
							  arealist[i].board);
			}

			if (arealist[i].echomail) {
				fputs("quick echo ",fp);
				if (arealist[i].priv) fputc('p',fp);
				if (arealist[i].hold) fputc('h',fp);
				if (arealist[i].direct) fputc('d',fp);
				if (arealist[i].crash) fputc('c',fp);
				if (arealist[i].killsent) fputc('k',fp);
				fprintf(fp," \"%s\" %d\n",arealist[i].description?arealist[i].description:"",
							  arealist[i].board);
			}

			if (arealist[i].news) {
				fputs("quick news ",fp);
				if (arealist[i].priv) fputc('p',fp);
				if (arealist[i].hold) fputc('h',fp);
				if (arealist[i].direct) fputc('d',fp);
				if (arealist[i].crash) fputc('c',fp);
				if (arealist[i].killsent) fputc('k',fp);
				fprintf(fp," \"%s\" %d\n",arealist[i].description?arealist[i].description:"",
							  arealist[i].board);
			}

			if (arealist[i].uucp) {
				fputs("quick uucp ",fp);
				if (arealist[i].priv) fputc('p',fp);
				if (arealist[i].hold) fputc('h',fp);
				if (arealist[i].direct) fputc('d',fp);
				if (arealist[i].crash) fputc('c',fp);
				if (arealist[i].killsent) fputc('k',fp);
				fprintf(fp," \"%s\" %d\n",arealist[i].description?arealist[i].description:"",
							  arealist[i].board);
			}
		}
	}

	fputs("\n; uucp and domain gates\n\n",fp);

	if (!uucp_gate.notfound)
		fprintf(fp,"uucp %s\n\n",show_address(uucp_gate));

	for (i = 0; i < domains; i++)
		fprintf(fp,"domain %s\n\n",show_address(domain_list[i]));

	switch (gate) {
		case GDOMAINS:	fputs("gate domains\n",fp);
				break;
		case GZONES:	fputs("gate zones\n",fp);
				break;
		case BOTH:		fputs("gate both\n",fp);
				break;
		case 0: 		fputs("gate none\n",fp);
				break;
	}

	fputs("\n; your origin line\n\n",fp);

	fprintf(fp,"origin %s\n",origin?origin:"");

	fputs("\n; message reader key definitions\n\n",fp);

	for (key = 0; key < 256; key++)
		if (r_getbind(key) != NULL)
			fprintf(fp,"readkey 0x%04x %s\n",key,r_getbind(key));

	for (key = 0; key < 256; key++)
		if (r_getbind(key<<8) != NULL)
			fprintf(fp,"readkey 0x%02x00 %s\n",key,r_getbind(key<<8));

	fputs("\n; editor key definitions\n\n",fp);

	for (key = 0; key < 256; key++)
		if (e_getbind(key) != NULL)
			fprintf(fp,"editkey 0x%04x %s\n",key,e_getbind(key));

	for (key = 0; key < 256; key++)
		if (e_getbind(key<<8) != NULL)
			fprintf(fp,"editkey 0x%02x00 %s\n",key,e_getbind(key<<8));

	fputs("\n; function key definitions\n\n",fp);

	for (i = 0; i < 40; i++) {
		if (macros[i] != NULL) {
			fprintf(fp,"function %d ",i);
			keys = macros[i];
			while (*keys) {
				if (*keys < 32) {
					fputc('^',fp);
					fputc(*keys+96,fp);
				}
				else if (*keys & 0xff00)
					fprintf(fp,"\\0x%02x",*keys>>8);
				else
					fputc(*keys,fp);
				keys++;
			}
			fputc('\n',fp);
		}
	}

	fclose(fp);
}


static void _pascal edit_area(AREA *a)

{
	char *fields[7];

	static char *slist[] = {
		"Fido (.msg)", "QuickBBS   ", NULL
	};

	static char *tlist[] = {
		"local  ",
		"net    ",
		"echo   ",
		"news   ",
		"uucp   ",
		NULL
	};

	int i = 0,j;
	int f2 = 0;

	fields[6] = NULL;

	for (;;) {
		fields[0] = calloc(256,sizeof(char));
		sprintf(fields[0],"Description:     %s",a->description?a->description:"");
		fields[1] = calloc(128,sizeof(char));

		if (a->msgtype == FIDO) {
			sprintf(fields[1],"Path:            %s",a->path?a->path:"");
			fields[3] = strdup("Message Base:    Fido (.msg)");
		}
		else {
			sprintf(fields[1],"Board Number:    %d",a->board);
			fields[3] = strdup("Message Base:    QuickBBS");
		}

		fields[2] = strdup("Attributes:              ");
		*(fields[2] + 17) = '\0';
		if (a->priv) strcat(fields[2],"p");
		if (a->hold) strcat(fields[2],"h");
		if (a->direct) strcat(fields[2],"d");
		if (a->crash) strcat(fields[2],"c");
		if (a->killsent) strcat(fields[2],"k");

		fields[4] = calloc(128,sizeof(char));
		strcpy(fields[4],  "Type of Message: ");
		if (a->local) {
			strcat(fields[4],"local ");
			*(tlist[0] + 6) = 0xfb;
		}
		else
			*(tlist[0] + 6) = ' ';

		if (a->netmail) {
			strcat(fields[4],"net ");
			*(tlist[1] + 6) = 0xfb;
		}
		else
			*(tlist[1] + 6) = ' ';


		if (a->echomail) {
			*(tlist[2] + 6) = 0xfb;
			fields[5] = calloc(128,sizeof(char));
			sprintf(fields[5],"Echomail Tag:    %s",a->tag?a->tag:"");
			strcat(fields[4],"echo ");
		}
		else {
			*(tlist[2] + 6) = ' ';
			fields[5] = NULL;
		}

		if (a->news) {
			strcat(fields[4],"news ");
			*(tlist[3] + 6) = 0xfb;
		}
		else
			*(tlist[3] + 6) = ' ';

		if (a->uucp) {
			strcat(fields[4],"uucp");
			*(tlist[4] + 6) = 0xfb;
		}
		else
			*(tlist[4] + 6) = ' ';

		i = menu(5,18,maxx,maxy,fields,colors.hilite,colors.normal,i);

		switch (i) {
			case 0: if (a->description) free(a->description);
				gotoxy(22,wherey()); set_color(colors.hilite);
				bgets(fields[0]+17,80,min(80,maxx-22));
				a->description = strdup(fields[0]+17);
				break;

			case 1: if (a->msgtype == FIDO) {
					if (a->path) free(a->path);
					gotoxy(22,wherey()); set_color(colors.hilite);
					bgets(fields[1]+17,80,min(80,maxx-22));
					a->path = strdup(fields[1]+17);
				}
				else if (a->msgtype == QUICK) {
					gotoxy(22,wherey()); set_color(colors.hilite);
					a->board = getnum(1,200,a->board);
				}
				break;

			case 2: box(14,14,27,20,1);
				set_attrib(15,15,26,19,a);
				clrwnd(14,14,27,20);
				break;

			case 3: box(15,15,27,18,1);
				a->msgtype = menu(16,16,26,17,slist,colors.hilite,colors.normal,a->msgtype);
				clrwnd(15,15,27,18);
				break;

			case 4: box(14,14,22,20,1);
				while ((f2 = menu(15,15,21,19,tlist,colors.hilite,colors.normal,f2)) != -1) {
					switch (f2) {
						case 0: *(tlist[0]+6) = (char) (((a->local = !a->local) != 0)?0xfb:0x20); break;
						case 1: *(tlist[1]+6) = (char) (((a->netmail = !a->netmail) != 0)?0xfb:0x20); break;
						case 2: *(tlist[2]+6) = (char) (((a->echomail = !a->echomail) != 0)?0xfb:0x20); break;
						case 4: *(tlist[4]+6) = (char) (((a->uucp = !a->uucp) != 0)?0xfb:0x20); break;
						case 3: *(tlist[3]+6) = (char) (((a->news = !a->news) != 0)?0xfb:0x20); break;
					}
				}
				clrwnd(14,14,22,20);
				break;

			case 5: if (a->echomail) {
					if (a->tag) free(a->tag);
					gotoxy(22,wherey()); set_color(colors.hilite);
					bgets(fields[5]+17,64,min(64,maxx-22));
					a->tag = strdup(fields[5]+17);
				}
				break;
		}

		for (j = 0; j < 6; j++)
			if (fields[j]) free(fields[j]);

		if (i == -1) {
			set_color(colors.normal);
			clrwnd(5,18,maxx,maxy);
			return;
		}
	}
}

