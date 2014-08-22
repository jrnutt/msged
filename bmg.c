/*
 *	bmg.c ->	Boyer-Moore-Gosper search routines
 *
 * Adapted from:
 *	 Boyer/Moore/Gosper-assisted 'egrep' search, with delta0 table as in
 *	 original paper (CACM, October, 1977).	No delta1 or delta2.  According to
 *	 experiment (Horspool, Soft. Prac. Exp., 1982), delta2 is of minimal
 *	 practical value.  However, to improve for worst case input, integrating
 *	 the improved Galil strategies (Apostolico/Giancarlo, Siam. J. Comput.,
 *	 February 1986) deserves consideration.
 *
 *	 James A. Woods 				Copyright (c) 1986
 *	 NASA Ames Research Center
 *
 * 29 April 1986	Jeff Mogul	Stanford
 *	Adapted as a set of subroutines for use by a program. No
 *	regular-expression support.
 *
 * 29 August 1986	Frank Whaley	Beyond Words
 *	Trimmed for speed and other dirty tricks.
 */

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "bmg.h"

#ifdef __MSC__
#define strcmpl stricmp
#define strncmpi strnicmp
#endif


#ifdef __TURBOC__
#define strcmpl strcmpi
#endif

#if defined (__ZTC__)
int _pascal strncmpi(char *s, char *t, int n);
#endif



/*
 *	bmgCompile() -> compile Boyer-Moore delta table
 *
 *	bmgCompile() compiles the delta table for the given search string, and
 *	 initializes the search argument structure.  This structure must be
 *	 passed to the bmgSearch() function described below.
 */


void _pascal bmgCompile(char *pat, bmgARG *arg, int ignore)

	{
	int i,		/*	general ctr 	*/
		patlen; 	/*	pattern length	*/

	patlen = strlen(pat);

	strcpy(arg->pat, pat);
	if ((arg->ignore = (char) ignore) != 0)
		strlwr(arg->pat);

	memset(arg->delta, patlen, 256);

	for (i = 0; i < patlen; ++i)
		arg->delta[pat[i]] = (char) (patlen - i - 1);

	if (ignore) /*	tweak upper case if ignore on  */
		for (i = 0; i < patlen; ++i)
			arg->delta[toupper(pat[i])] = (char) (patlen - i - 1);
	}


/*
 *	bmgSearch() ->	scan for match
 *
 *	bmgSearch() performs a Boyer-Moore-Gosper search of the given buffer
 *	 for the string described by the given search argument structure.  The
 *	 match action function "action" will be called for each match found.
 *	 This function should return non-zero to continue searching, or 0 to
 *	 terminate the search.	bmgSearch() returns the total number of
 *	 matches found.
 */

char *pascal bmgSearch(char * buffer, int buflen, bmgARG *arg)

	{
	char	*s; 	/*	temp ptr for comparisons	*/
	int inc,		/*	position increment		*/
		k,		/*	current buffer index	*/
		patlen; 	/*	pattern length		*/

	k = (patlen = strlen(arg->pat)) - 1;

	for (;;)
		{
		while (((inc = arg->delta[buffer[k]]) != 0) &&
			((k += inc) < buflen))
			;
		if (k >= buflen)
			break;

		s = buffer + (k++ - (patlen - 1));
		if (!(arg->ignore ? strncmpi(s, arg->pat, patlen) : strncmp(s, arg->pat, patlen)))
			return (s);
		}

	return (NULL);
	}


/*
 *	END of bmg.c
 */

