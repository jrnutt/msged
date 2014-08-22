/*
 *	bmg.h ->	Boyer-Moore-Gosper search definitions
 *
 *	see bmg.c for documentation
 */

#include "pascal.h"

#define	bmgMAXPAT	64		/*  max pattern length	*/
typedef struct
	{
	char	delta[256];		/*  ASCII only deltas	*/
	char	pat[bmgMAXPAT + 1];	/*  the pattern		*/
	char	ignore;			/*  ignore case flag	*/
	}
	bmgARG;

void _pascal bmgCompile(char *s, bmgARG *pattern, int ignore_case);
char * _pascal bmgSearch(char *buffer, int buflen, bmgARG *pattern);


/*
 *	END of bmg.h
 */

