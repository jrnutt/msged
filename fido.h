/**
 *
 * support header for fido/opus style message bases
 *
 * 	PUBLIC DOMAIN
**/

int   _pascal fido_delete(int n);
MSG  *pascal fido_readheader(int n);
int   _pascal fido_writetext(char *text, int n);
int   _pascal fido_writeheader(MSG *m);
char *pascal fido_readtext(int n);
MSG  *pascal fido_read(int n);
int   _pascal fido_setlast(AREA a);
int   _pascal fido_scan(AREA *a);
