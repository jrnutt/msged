/**

 * string.c
 * released into the PUBLIC DOMAIN 30 jul 1990 by jim nutt
 * a few string handling routines for msged

**/

#if defined(__ZTC__)
#define NO_HANDLE 1
#include <handle.h>
#else
#include "nohandl.h"
#endif

#include <string.h>
#include <ctype.h>

#include "pascal.h"

char __handle * _pascal strdup_handle(const char *s);


#if defined(__ZTC__)
int _pascal strncmpi(char *s, char *t, int n);
#endif
void _pascal strins(char *l, char c, int x);
void _pascal strdel(char *l, int x);

void _pascal strins(char *l, char c, int x)

{
	int 	i = strlen(l);

	if (x > (i+1))
		return;
	else {
		x--;
		memmove((l + x + 1), (l + x), (i - x) + 1);
		*(l + x) = c;
	}
}

void _pascal strdel(char *l, int x)

{
	int 	i = strlen(l);

	if (x > i) return;
	x--;
	memmove((l + x), (l + x + 1), (i - x) + 1);
	*(l + i) = 0;
}

#if defined(__ZTC__)
/*
 *	strncmpi() ->	strncmp(), ignore case
 */

int _pascal strncmpi(char *s, char *t, int n)

{
	for (; n-- && (tolower(*s) == tolower(*t)); ++t)
		if (!*s++)
			return (0); /* equal */

	if (n < 0)		/* maximum hit */
		return (0); 	/* equal */

	return ((tolower(*s) > tolower(*t)) ? 1 : (-1));	/* not equal */
}
#endif
