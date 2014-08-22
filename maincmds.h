/* PUBLIC DOMAIN */

static void _near _pascal left(void);
static void _near _pascal right(void);
static void _near _pascal newarea(void);
static void _near _pascal go_last(void);
static void _near _pascal link_to(void);
static void _near _pascal link_from(void);
static void _near _pascal view(void);
static void _near _pascal go_root(void);
static void _near _pascal go_dos(void);
static void _near _pascal rundos(void);
static void _near _pascal search(void);
static void _near _pascal quit(void);
static void _near _pascal delete(void);                        /* maintmsg.c   */
static void _near _pascal new(void);                           /* makemsg.c    */
static void _near _pascal doreply(void);                    /* makemsg.c    */
static void _near _pascal doquote(void);                    /* makemsg.c    */
static void _near _pascal move(void);                          /* maintmsg.c   */
static void _near _pascal outtxt(void);                         /* textfile.c   */
static void _near _pascal set(void);                         /* settings.c   */
static void _near _pascal list(void);                             /* msglist.c    */
static void _near _pascal dochange(void);                           /* makemsg.c    */
static void _near _pascal rotate(void);                           /* msged.c      */
static void _near _pascal next_area(void);
static void _near _pascal prev_area(void);
static void _near _pascal scan_areas(void);
static void _near _pascal nada(void);

void (_near _pascal *mainckeys[256])(void) =
{
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /*  0 */
NULL,   NULL,   NULL,   NULL,   NULL,   right,  NULL,   NULL, /*  8 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 10 */
NULL,   NULL,   NULL,   quit,   NULL,   NULL,   NULL,   NULL, /* 18 */
NULL,   rundos, NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 20 */
NULL,   NULL,   scan_areas,next_area,NULL,prev_area,NULL,NULL,/* 28 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 30 */
NULL,   NULL,   NULL,   NULL,   rotate, NULL,   rotate, NULL, /* 38 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 40 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 48 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 50 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 58 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 60 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 68 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 70 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,  /* 78 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 80 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 88 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 90 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 98 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* A0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* A8 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* B0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* B8 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* C0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* C8 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* D0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* D8 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* E0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* E8 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* F0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL  /* F8 */
};

void (_near _pascal *mainakeys[256])(void) =
{
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /*  0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /*  8 */
doquote,outtxt, new,    doreply,NULL,   NULL,   NULL,   NULL, /* 10 */
go_dos, NULL,   NULL,   NULL,   NULL,   NULL,   newarea,set,/* 18 */
delete, search, NULL,   NULL,   NULL,   NULL,   list,   NULL, /* 20 */
NULL,   NULL,   NULL,   NULL,   NULL,   quit,   dochange,view, /* 28 */
NULL,   NULL,   move,   NULL,   NULL,   NULL,   NULL,   NULL, /* 30 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 38 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   go_root, /* 40 */
NULL,   NULL,   NULL,   left,   NULL,   right,  NULL,   go_last, /* 48 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 50 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 58 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 60 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 68 */
NULL,   NULL,   NULL,   link_from,link_to,NULL, NULL,   NULL, /* 70 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,  /* 78 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 80 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 88 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 90 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* 98 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* A0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* A8 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* B0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* B8 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* C0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* C8 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* D0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* D8 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* E0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* E8 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL, /* F0 */
NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL  /* F8 */
};

const struct _command maincmds[] = {
	{"next_area",next_area},{"previous",left},      {"areas",newarea},
	{"last",go_last},       {"link_to",link_to},    {"link_from",link_from},
	{"view",view},          {"home",go_root},       {"shell",go_dos},
	{"search",search},      {"delete",delete},      {"newmsg",new},
	{"reply",doreply},      {"quote",doquote},      {"move",move},
	{"export",outtxt},      {"config",set},         {"dos", rundos},
	{"list",list},          {"change",dochange},    {"null",nada},
	{"exit",quit},       {"prev_area",prev_area},
	{"scan",scan_areas},    {"next",right},
	{NULL,NULL}
};

