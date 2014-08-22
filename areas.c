/*

Title:  MsgEd

File:   areas.c

Author: Jim Nutt

Copr:   released into the PUBLIC DOMAIN 30 Jul 1990 by jim nutt

Description:

	This file contains the routines necessary to select individual
	areas for reading, etc.

*/

#include "msged.h"
#include "menu.h"

void _pascal areascan(void);

int _pascal selectarea()

{
	char  **list;
	char line[50];
	AREA *a;
	int i = 0,j;

	cls();

	if ((maxx - 50) > 15) {
		gotoxy(2,2); bputs("msged version " VERSION);
		gotoxy(2,4); bputs("Area statistics and");
		gotoxy(2,5); bputs("selection menu");
		gotoxy(2,7); bputs("Use arrow keys or type");
		gotoxy(2,8); bputs("a unique segment of the");
		gotoxy(2,9); bputs("area description to");
		gotoxy(2,10);bputs("select an area, then");
		gotoxy(2,11);bputs("press the return or ");
		gotoxy(2,12);bputs("enter key");

	}
	gotoxy(maxx-50,1); bputs("Area                          New  Unread Number");
	gotoxy(maxx-50,2); bputs("Description                   msg   msgs    of");
	gotoxy(maxx-50,3); bputs("                               ?           msgs");
	gotoxy(maxx-50,4); bputs("------------------------------------------------");

	list = calloc(areas + 2, sizeof(char *));

	if (!scanned) {

		list[0] = strdup("Scan for new messages");

		for (i = 0; i < areas; i++) {
			memset(line, 0, sizeof line);
			a = arealist + i;
			if (a->messages)
				sprintf(line, "%-30.30s %c %6d %6d",a->description,
					((a->lastread+1) < a->messages)?'y':'n',
					a->messages - a->lastread - 1,a->messages);
		        else
				sprintf(line, "%-30.30s %15s",a->description,scanned?"empty":"unscanned");

			list[i+1] = strdup(line);
	        }

		i = menu(maxx-50,5,maxx,maxy,list,colors.hilite,colors.normal,0);

		if (i == 0) {
			areascan();
			gotoxy(1,1); clreol();
			gotoxy(maxx-50,1); bputs("Area                          New  Unread Number");
		}

		for (j = 0; j < areas+1; j++)
			free(list[j]);
	}

	i--;

	if (i == -1) {

		memset(list,0, sizeof(char *) * (areas + 2));

		for (i = 0; i < areas; i++) {
			memset(line, 0, sizeof line);
			a = arealist + i;
			if (a->messages)
				sprintf(line, "%-30.30s %c %6d %6d",a->description,
					((a->lastread+1) < a->messages)?'y':'n',
					a->messages - a->lastread - 1,a->messages);
		        else
				sprintf(line, "%-30.30s %15s",a->description,"empty");

			list[i] = strdup(line);
	        }

		i = menu(maxx-50,5,maxx,maxy,list,colors.hilite,colors.normal,area);

		for (j = 0; j < areas; j++)
			free(list[j]);
	}

	gotoxy(2,15); set_color(colors.warn); bputs("wait"); set_color(colors.normal);

	free(list);

	set_color(colors.normal);

	return((i < 0)?area:i);
}
