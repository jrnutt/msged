/**
 *
 * support header for quickbbs style message bases
 *
 *	PUBLIC DOMAIN
**/

int   _pascal quick_delete(int n);
MSG  *pascal quick_readheader(int n);
int   _pascal quick_writetext(char *t, int n);
int   _pascal quick_writeheader(MSG *m);
char *pascal quick_readtext(int n);
int   _pascal quick_setlast(AREA a);
int   _pascal quick_scan(AREA *a);
