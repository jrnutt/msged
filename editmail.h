/* PUBLIC DOMAIN */

#define NOSPELL

static void _near _pascal rundos(void);
static void _near _pascal backspace(void);
static void _near _pascal rotate(void);
static void _near _pascal delete_character(void);
static void _near _pascal delword(void);
static void _near _pascal go_left(void);
static void _near _pascal go_right(void);
static void _near _pascal go_word_right(void);
static void _near _pascal go_word_left(void);
static void _near _pascal newline(void);
static void _near _pascal go_up(void);
static void _near _pascal go_down(void);
static void _near _pascal go_pgup(void);
static void _near _pascal go_pgdown(void);
static void _near _pascal delete_line(void);
static void _near _pascal go_eol(void);
static void _near _pascal cut(void);
static void _near _pascal paste(void);
static void _near _pascal anchor(void);
static void _near _pascal quit(void);
static void _near _pascal die(void);
static void _near _pascal imptxt(void);
static void _near _pascal outtext(void);
static void _near _pascal shellos(void);
static void _near _pascal go_bol(void);
static void _near _pascal toggle_ins(void);
static void _near _pascal tabit(void);
static void _near _pascal go_tos(void);
static void _near _pascal go_bos(void);
static void _near _pascal go_bom(void);
static void _near _pascal go_tom(void);
static void _near _pascal nada(void);
static void _near _pascal killeol(void);

#ifndef NOSPELL
#include "spell.h"
#endif

/* table of normal keystrokes */

void (_near _pascal *editckeys[256])(void) =
{
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /*  0 */
backspace,tabit,NULL,	NULL,	NULL,	newline,NULL,	NULL, /*  8 */
NULL,	NULL,	NULL,	NULL,	delword,NULL,	NULL,	NULL, /* 10 */
NULL,	NULL,	NULL,	die,	NULL,	NULL,	NULL,	NULL, /* 18 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 20 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 28 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 30 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 38 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 40 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 48 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 50 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 58 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 60 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 68 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 70 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	backspace, /* 78 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 80 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 88 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 90 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 98 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* A0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* A8 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* B0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* B8 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* C0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* C8 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* D0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* D8 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* E0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* E8 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* F0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL  /* F8 */
};

/* table of extended keystrokes */

void (_near _pascal *editakeys[256])(void) =
{
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /*  0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /*  8 */
NULL,   outtext,NULL,   rotate, NULL,   NULL,   NULL,   imptxt,/* 10 */
shellos,paste,	NULL,	NULL,	NULL,	NULL,	anchor, quit, /* 18 */
delete_line,NULL,NULL,	NULL,	NULL,	killeol,NULL,	NULL, /* 20 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	cut,	NULL, /* 28 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 30 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 38 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	go_bol, /* 40 */
go_up,	go_pgup,NULL,	go_left,NULL,	go_right,NULL,	go_eol, /* 48 */
go_down,go_pgdown,toggle_ins,delete_character,NULL,NULL,NULL,NULL, /* 50 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 58 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 60 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 68 */
NULL,NULL,NULL,go_word_left,go_word_right,go_bom,go_bos, go_tom,/* 70 */
rundos, NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 78 */
NULL,	NULL,	NULL,	NULL,	go_tos, NULL,	NULL,	NULL, /* 80 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 88 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 90 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* 98 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* A0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* A8 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* B0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* B8 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* C0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* C8 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* D0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* D8 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* E0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* E8 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL, /* F0 */
NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL  /* F8 */
};

const struct _command editcmds[] = {
	{"backspace",backspace},	{"deleol",killeol},
	{"left",go_left},		{"right",go_right},
	{"wordright",go_word_right},	{"wordleft",go_word_left},
	{"newline",newline},		{"up",go_up},
	{"down",go_down},		{"pgup",go_pgup},
	{"pgdn",go_pgdown},		{"delline",delete_line},
	{"goeol",go_eol},		{"cut",cut},
	{"anchor",anchor},		{"paste",paste},
	{"quit",quit},			{"abort",die},
	{"import",imptxt},		{"export",outtext},
	{"shell",shellos},		{"gobol",go_bol},
	{"insert",toggle_ins},
	{"tab",tabit},                  {"null",nada},
	{"top",go_tos}, 		{"bottom",go_bos},
	{"first",go_tom},		{"last",go_bom},
	{"del",delete_character},	{"killword",delword},
	{NULL,NULL}
};
