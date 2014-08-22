/* PUBLIC DOMAIN */

#define ASM 1

void _pascal dcls(void);
void _pascal dputc(int x, int y, int c);
int  _pascal dputs(int x, int y, char *s);
void _pascal dclrwnd(int x1, int y1, int x2, int y2);
void _pascal dscrollup(int x1, int y1, int x2, int y2);
void _pascal dscrolldn(int x1, int y1, int x2, int y2);
