/* PUBLIC DOMAIN */

typedef struct {
	int vfossil_size;
	int vfossil_major;
	int vfossil_revision;
	int vfossil_highest;
} VFOSSIL, *VFOSSILP;

typedef struct {
	int   cur_start;
	int   cur_end;
	int   cur_wid;
	int   cur_attr;
} CURSOR, *CURSORP;

typedef CURSOR far *PVIOCURSORINFO;

typedef struct _VIOCONFIGINFO { /* vioin */
	unsigned int  cb     ;
	unsigned int  adapter;
	unsigned int  display;
	unsigned long	cbMemory;
} VIOCONFIGINFO;

typedef VIOCONFIGINFO far *PVIOCONFIGINFO;

typedef struct _VIOMODEINFO {   /* viomi */
	unsigned int cb;
	unsigned char  fbType;
	unsigned char  color;
	unsigned int col;
	unsigned int row;
	unsigned int hres;
	unsigned int vres;
	unsigned char  fmt_ID;
	unsigned char  attrib;
} VIOMODEINFO;

typedef VIOMODEINFO far *PVIOMODEINFO;

struct vfossil_hooks {	/* VFossil calls structure */
	unsigned int (_pascal far *GetMode)(PVIOMODEINFO, unsigned int);
	unsigned int (_pascal far *SetMode)(PVIOMODEINFO, unsigned int);
	unsigned int (_pascal far *GetConfig)(unsigned int, PVIOCONFIGINFO, unsigned int);
	unsigned int (_pascal far *WrtTTY)(char far *, unsigned int, unsigned int);
	unsigned int (_pascal far *GetAnsi)(unsigned int far *, unsigned int);
	unsigned int (_pascal far *SetAnsi)(unsigned int, unsigned int);
	unsigned int (_pascal far *GetCurPos)(unsigned int far *, unsigned int far *, unsigned int);
	unsigned int (_pascal far *SetCurPos)(unsigned int, unsigned int, unsigned int);
	unsigned int (_pascal far *GetCurType)(PVIOCURSORINFO, unsigned int);
	unsigned int (_pascal far *SetCurType)(PVIOCURSORINFO, unsigned int);
	unsigned int (_pascal far *ScrollUp)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned char far *,  unsigned int);
	unsigned int (_pascal far *ScrollDn)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned char far *,  unsigned int);
	unsigned int (_pascal far *ReadCellStr)(char far *, unsigned int far *, unsigned int, unsigned int, unsigned int);
	unsigned int (_pascal far *ReadCharStr)(char far *, unsigned int far *, unsigned int, unsigned int, unsigned int);
	unsigned int (_pascal far *WrtCellStr)(unsigned int far *, unsigned int, unsigned int, unsigned int, unsigned int);
	unsigned int (_pascal far *WrtCharStr)(char far *, unsigned int, unsigned int, unsigned int, unsigned int);
	unsigned int (_pascal far *WrtCharStrAtt)(char far *, unsigned int, unsigned int, unsigned int, unsigned char far *, unsigned int);
	unsigned int (_pascal far *WrtNAttr)(unsigned char far *, unsigned int, unsigned int, unsigned int, unsigned int);
	unsigned int (_pascal far *WrtNCell)(unsigned int far *, unsigned int, unsigned int, unsigned int, unsigned int);
	unsigned int (_pascal far *WrtNChar)(char far *, unsigned int, unsigned int, unsigned int, unsigned int);
} vfossil_funcs;

/* Now for readability (and portability) ... */
#define VioGetMode(a,b)			((*vfossil_funcs.GetMode)(a,b))
#define VioSetMode(a,b)			((*vfossil_funcs.SetMode)(a,b))
#define VioGetConfig(a,b,c)		(*vfossil_funcs.GetConfig)(a,b,c))
#define VioWrtTTY(a,b,c)		((*vfossil_funcs.WrtTTY)(a,b,c))
#define VioGetANSI(a,b)			((*vfossil_funcs.GetANSI)(a,b))
#define VioSetANSI(a,b)			((*vfossil_funcs.SetANSI)(a,b))
#define VioGetCurPos(a,b,c)		((*vfossil_funcs.GetCurPos)(a,b,c))
#define VioSetCurPos(a,b,c)		((*vfossil_funcs.SetCurPos)(a,b,c))
#define VioGetCurType(a,b)		((*vfossil_funcs.GetCurType)(a,b))
#define VioSetCurType(a,b)		((*vfossil_funcs.SetCurType)(a,b))
#define VioScrollUp(a,b,c,d,e,f,g)	((*vfossil_funcs.ScrollUp)(a,b,c,d,e,f,g))
#define VioScrollDn(a,b,c,d,e,f,g)	((*vfossil_funcs.ScrollDn)(a,b,c,d,e,f,g))
#define VioReadCellStr(a,b,c,d,e)	((*vfossil_funcs.ReadCellStr)(a,b,c,d,e))
#define VioReadCharStr(a,b,c,d,e)	((*vfossil_funcs.ReadCharStr)(a,b,c,d,e))
#define VioWrtCellStr(a,b,c,d,e)	((*vfossil_funcs.WrtCellStr)(a,b,c,d,e))
#define VioWrtCharStr(a,b,c,d,e)	((*vfossil_funcs.WrtCharStr)(a,b,c,d,e))
#define VioWrtCharStrAtt(a,b,c,d,e,f)	((*vfossil_funcs.WrtCharStrAtt)(a,b,c,d,e,f))
#define VioWrtNAttr(a,b,c,d,e)		((*vfossil_funcs.WrtNAttr)(a,b,c,d,e))
#define VioWrtNCell(a,b,c,d,e)		((*vfossil_funcs.WrtNCell)(a,b,c,d,e))
#define VioWrtNChar(a,b,c,d,e)		((*vfossil_funcs.WrtNChar)(a,b,c,d,e))
