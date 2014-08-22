/*

Title:  Msged

File:   maintmsg.c

Author: Jim Nutt

Copr:   released into the PUBLIC DOMAIN 30 jul 1990 by jim nutt

Description:

    handles all message maintenance

*/

#define TEXTLEN 80

#include "msged.h"
#include "menu.h"
#include "date.h"

extern int direction;

static char *formenu[] = {"Move Message","Copy Message","Forward Message",NULL};

void    _pascal deletemsg()

{
    int     nto = 0;
    int     nfrm = 0;
    MSG    *from = NULL, *to = NULL;
    int     n = arealist[area].current;

    if (message == NULL)
        return;

    if (!confirm())
        return;

    nto = message->replyto;
    nfrm = message->replyfrom;

    if (nto)
        to = msg_readheader(nto);

    if (to == NULL)
        nto = 0;

    if (nfrm)
        from = msg_readheader(nfrm);

    if (from == NULL)
        nfrm = 0;
    else
        from->replyto = nto;


    if (to != NULL)
        to->replyfrom = nfrm;

    if (to) {
        msg_writeheader(to);
        dispose(to);
        to = NULL;
    }

    if (from) {
        msg_writeheader(from);
        dispose(from);
        to = NULL;
    }

    msg_delete(n);
    arealist[area].messages = msg_scan(&arealist[area]);
    if (direction == LEFT)
        arealist[area].current = min(max(0,n-1),arealist[area].messages - 1);
    else
        arealist[area].current = min(n,arealist[area].messages - 1);
}

void    _pascal movemsg()
{
    int     ch = 0;
    int     to_area;
    int     fr_area;
    int     current = arealist[area].current;
    char    sfn[TEXTLEN];
    char    dfn[TEXTLEN];
    time_t  now = time(NULL);

    msg_last(arealist[area]);

    for (;;) {
        box((maxx/2)-9,(maxy/2)-2,(maxx/2)+8,(maxy/2)+2,1);
        ch = menu((maxx/2)-8,(maxy/2)-1,(maxx/2)+7,(maxy/2)+1,
	      formenu,colors.hilite,colors.normal,0);

        switch (ch) {

            case 0: {
                int t = confirmations;
                confirmations = NO;
                deletemsg();
                confirmations = t;
                }
            case 1:
                fr_area = area;
                area = to_area = selectarea();
                arealist[area].messages = msg_scan((&arealist[area]));
                arealist[area].new = 1;
                message->msgnum =  arealist[area].last + 1;
                message->new = 1;
                writemsg(message);
                area = fr_area;
                arealist[area].messages = msg_scan((&arealist[area]));
                arealist[to_area].new = 1;
                arealist[area].current =
                    (current < arealist[area].messages)?current:arealist[area].current;
                return;

            case 2:
                message->msgnum = arealist[area].last + 1;
                message->attrib.sent = 0;
                message->attrib.recvd = 0;
                message->text = insline(message->text);
                message->text->text = strdup("\n");

                sprintf(sfn," * Original to %s @ %s in %s\n",message->isto,
                        (show_address(message->to))?show_address(message->to):show_address(thisnode),
                        (arealist[area].echomail)?arealist[area].tag:"netmail");
                sprintf(dfn," * Forwarded %s by %s @ %s\n",atime(now),username,show_address(thisnode));

                message->text = insline(message->text);
                message->text->text = strdup(dfn);

                message->text = insline(message->text);
                message->text->text = strdup(sfn);

                strset(message->isto,0);
                message->to.notfound = 1;

                if ((editheader() == ABORT) && (confirm()))
                    return;
                save(message);
                arealist[area].messages = msg_scan((&arealist[area]));
                arealist[area].current = current;
                return;

            case -1:    /* Escape */
                return;

        }
    }
}
