/*--------------------------------------------------------------------------*/
/*																			*/
/*																			*/
/*		------------		 Bit-Bucket Software <no-Inc>					*/
/*		\ 10001101 /		 Writers and Distributors of					*/
/*		 \ 011110 / 		 No-Cost<no-tm> Software.						*/
/*		  \ 1011 /															*/
/*		   ------			 KopyRong (K) 1987.  ALL RIGHTS REVERSED.		*/
/*																			*/
/*																			*/
/*				 This module was written by Vince Perriello 				*/
/*																			*/
/*																			*/
/*				   BinkleyTerm Fidolist processing module					*/
/*																			*/
/*																			*/
/*	 This  software  package is being distributed WITH FULL SOURCE CODE 	*/
/*	 with the  following  conditions:	 1)  If  anything awful happens 	*/
/*	 because  you  use	  it   (or	don't  use  it),  you  accept  full     */
/*	 responsibility;  2) you  don't start making tons of voice calls to     */
/*	 the authors to complain or  make  suggestions	about enhancements, 	*/
/*	 useful or otherwise;  3) you  do not reuse this code in commercial 	*/
/*	 products without specific permission to do so	from  the  authors; 	*/
/*	 4) If you find any problems you send  fixes  to  the  authors	for 	*/
/*	 inclusion	in	updates;	5) You find some way  to  express  your 	*/
/*	 appreciation  for	this  method of distribution, either by writing 	*/
/*	 code or  application  notes,  or  just sending along a "Thank You"     */
/*	 message.																*/
/*																			*/
/*	 There is  copyrighted	code  in  this product.  We either wrote it 	*/
/*	 ourselves or got  permission  to use it.  Please don't force us to     */
/*	 pay a lawyer --  have some respect for our motives and don't abuse     */
/*	 this "license".                                                        */
/*																			*/
/*										*/
/*	 heavily modified 02 Aug 1988 for use in msged by jim nutt				*/
/*	 further modified 28 oct 1988 for use in msged by jim nutt				*/
/*	 used with permission							*/
/*																			*/
/*--------------------------------------------------------------------------*/

#include "msged.h"
#ifdef __MSC__
#include <sys/types.h>
#endif
#include <sys/stat.h>
#if !defined(__ZTC__)
#include <fcntl.h>
#endif

#define TEXTLEN 80

ADDRESS _pascal lookup(char *name, char *fn)
{
	int 	low, high, mid, f, cond, namelen;
	struct stat buf;
		char	midname[TEXTLEN];
		char	last_name_first[TEXTLEN];
	char   *c, *p, *m;
	int 	reclength;
		int 	nrecs;
		ADDRESS tmp;

	tmp = thisnode;
	if (thisnode.domain != NULL)
		tmp.domain = strdup(thisnode.domain);

	tmp.notfound = 1;

	memset(midname, 0, sizeof midname);

	c = midname;		/* Start of temp name buff	 */
	p = name;		/* Point to start of name	 */
	m = NULL;		/* Init pointer to space	 */

	while ((*c = *p++) != '\0') {	/* Go entire length of name  */
		if (*c == ' ')  /* Look for space            */
			m = c;	/* Save location			 */
		c++;
	}

	if (m != NULL) {	/* If we have a pointer,	 */
		*m++ = '\0';    /* Terminate the first half  */
		strcpy(last_name_first, m); /* Now copy the last name	 */
		strcat(last_name_first, ", ");  /* Insert a comma and space  */
		strcat(last_name_first, midname);	/* Finally copy first
							 * half   */
	}
	else
		strcpy(last_name_first, midname);	/* Use whole name
							 * otherwise  */

	strlwr(last_name_first);/* all lower case */
	namelen = strlen(last_name_first);	/* Calc length now			 */

		stat(fn, &buf);   /* get the file size */

#ifdef __MSC__
		if ((f = open(fn, O_RDONLY|O_BINARY)) == -1) {	   /*WRA*/
#else
		if ((f = open(fn, O_RDONLY)) == -1) {
#endif
		reclength = -1; /* Reset all on open failure */
				return (tmp);
	}

		memset(midname, 0, sizeof(midname));
		read(f, midname, sizeof(midname));	 /* Read 1 record */
	reclength = (int) (strchr(midname, '\n') - midname) + 1;    /* FindEnd */
	nrecs = (int) (buf.st_size / reclength); /* Now get num of records	  */

	/* Binary search algorithm */
	low = 0;
	high = nrecs - 1;
	while (low <= high) {
		mid = low + (high - low) / 2;
		lseek(f, (long) ((long) mid * (long) reclength), SEEK_SET);
		read(f, midname, reclength);
		strlwr(midname);
		if ((cond = strncmp(last_name_first, midname, namelen)) < 0)
			high = mid - 1;
		else {
			if (cond > 0)
				low = mid + 1;
			else {
				/* Return the address information */
				close(f);

				c = midname + reclength - 1;

				while (isspace(*c)) *c-- = '\0'; c--;
				while (!isspace(*c)) c--;

				if (tmp.domain != NULL)
					free(tmp.domain);

				tmp.notfound = 0;

				return parsenode(c+1);
			}
		}
	}
	close(f);
		return (tmp);
}
