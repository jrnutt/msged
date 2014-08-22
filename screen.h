/* PUBLIC DOMAIN */

#define DIRECT          0
#define BIOS            1
#define FOSSIL		2
#define ANSI		3

#ifdef WHITE
#undef WHITE
#endif

#include "pascal.h"

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


#ifdef BLACK
#undef BLACK
#endif

#define BLACK 0
#define WHITE 7

int  *getrgn(int x1,int y1,int x2,int y2);
void putrgn(int x1,int y1,int x2,int y2,int *rgn);
void _pascal clreol(void);
void _pascal gotoxy(int x,int y);
int  _pascal wherex(void);
int  _pascal wherey(void);
void _pascal video_end(void);
void _pascal video_init(void);
void _pascal video_update(void);
unsigned int _pascal keyhit(void);
unsigned int _pascal getkey(void);
void _pascal cls(void);
void _pascal bputc(int c);
void _pascal bputs(char *s);
void _pascal clrwnd(int x1,int y1,int x2,int y2);
void _pascal scrollup(int x1,int y1,int x2,int y2,int lines);
void _pascal scrolldown(int x1,int y1,int x2,int y2,int lines);
int  _pascal getnum(int lo, int hi, int value);
int  _pascal bgets(char *s,int c, int w);
void _pascal set_color(unsigned int attr);
unsigned _pascal get_color(void);
int  cdecl bprintf(char *s, ...);

#define ON 	1
#define OFF 	0
#define TAB	0x0009		/* <Tab>	*/
#define PGUP    0x4900		/* <PgUp> 	*/
#define PGDN    0x5100		/* <PgDn>	*/
#define UP      0x4800		/* <up>		*/
#define DOWN    0x5000		/* <down>	*/
#define LEFT    0x4b00		/* <left>	*/
#define RIGHT   0x4d00		/* <right>	*/
#define WORDRT 	0x7400		/* <ctrl><right>*/
#define WORDLT 	0x7300		/* <ctrl><left>	*/
#define DELCHR  0x5300		/* <Del>	*/
#define DELLN	0x2000		/* <Alt><D>	*/
#define GOEOL	0x4f00		/* <End>	*/
#define GOBOL	0x4700		/* <Home>	*/
#define BKSPC   0x0008		/* <BkSpc>	*/
#define RUBOUT  0x007f
#define SAVE	0x1f00		/* <Alt><S>	*/
#define ABORT	0x001b		/* <Esc>	*/
#define INSERT  0x5200		/* <Ins>	*/
#define ENTER   0x000d		/* <enter>	*/
#define WRITE	0x1100		/* <Alt><W>	*/
#define IMPORT	0x1700		/* <Alt><I>	*/
#define ANCHOR	0x1e00		/* <Alt><A>	*/
#define CUT	0x2e00		/* <Alt><C>	*/
#define PASTE	0x1900		/* <Alt><P>	*/
#define FORMAT  0x2100          /* <Alt><F>     */
#define CANCEL  0x0018		/* <ctrl><x>	*/
