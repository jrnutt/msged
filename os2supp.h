/*
	Various standard defines used in OS2 interfacing

*/

#ifndef OS2_SUPPORT
#define OS2_SUPPORT

/* ----------- OS/2 library (readability) types ----------- */

typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef int INT;
typedef unsigned short USHORT;
typedef void VOID;

typedef unsigned char UCHAR;	/* uch */
typedef unsigned long ULONG;	/* ul  */
typedef unsigned int  UINT; 	/* ui  */
/* typedef UINT 	 unsigned; */	/* us  */

typedef unsigned char BYTE; 	/* b   */

typedef BYTE   far *PBYTE;
typedef BYTE	   *NPBYTE;

typedef CHAR   far *PCHAR;
typedef SHORT  far *PSHORT;
typedef LONG   far *PLONG;
typedef INT    far *PINT;

typedef UCHAR  far *PUCHAR;
typedef USHORT far *PUSHORT;
typedef ULONG  far *PULONG;
typedef UINT   far *PUINT;

typedef VOID   far *PVOID;

typedef char far *PSZ;
typedef char	 *NPSZ;

typedef char far *PCH;
typedef char	 *NPCH;

/* ---------- Vio/VFOSSIL support types ------------ */

typedef struct {
   int vfossil_size;
   int vfossil_major;
   int vfossil_revision;
   int vfossil_highest;
} VFOSSIL, *VFOSSILP;

typedef struct {
   int	 cur_start;
   int	 cur_end;
   int	 cur_wid;
   int	 cur_attr;
} CURSOR, *CURSORP;
typedef CURSOR far *PVIOCURSORINFO;

typedef struct _VIOCONFIGINFO { /* vioin */
		USHORT	cb	   ;
		USHORT	adapter;
		USHORT	display;
		ULONG	cbMemory;
		} VIOCONFIGINFO;
typedef VIOCONFIGINFO far *PVIOCONFIGINFO;

/* structure for VioSet/GetMode() */
typedef struct _VIOMODEINFO {	/* viomi */
		USHORT cb;
		UCHAR  fbType;
		UCHAR  color;
		USHORT col;
		USHORT row;
		USHORT hres;
		USHORT vres;
		UCHAR  fmt_ID;
		UCHAR  attrib;
		} VIOMODEINFO;
typedef VIOMODEINFO far *PVIOMODEINFO;

/*----------- only include the rest of this for real OS/2 compiles ----------*/

#ifdef __OS2__

typedef struct _KBDKEYINFO {	/* kbci */
		UCHAR	 chChar;
		UCHAR	 chScan;
		UCHAR	 fbStatus;
		UCHAR	 bNlsShift;
		USHORT	 fsState;
		ULONG	 time;
		}KBDKEYINFO;
typedef KBDKEYINFO far *PKBDKEYINFO;

/* KBDINFO structure, for KbdSet/GetStatus */
typedef struct _KBDINFO {		/* kbst */
		USHORT cb;
		USHORT fsMask;
		USHORT chTurnAround;
		USHORT fsInterim;
		USHORT fsState;
		}KBDINFO;
typedef KBDINFO far *PKBDINFO;

#define KBD_BINARY	 4

#define IO_WAIT 	0
#define IO_NOWAIT	1

/* File time and date types */

typedef struct _FTIME { 		/* ftime */
	unsigned twosecs : 5;
	unsigned minutes : 6;
	unsigned hours	 : 5;
} FTIME;
typedef FTIME far *PFTIME;

typedef struct _FDATE { 		/* fdate */
	unsigned day	 : 5;
	unsigned month	 : 4;
	unsigned year	 : 7;
} FDATE;
typedef FDATE far *PFDATE;

typedef struct _FILEFINDBUF {	/* findbuf */
	FDATE  fdateCreation;
	FTIME  ftimeCreation;
	FDATE  fdateLastAccess;
	FTIME  ftimeLastAccess;
	FDATE  fdateLastWrite;
	FTIME  ftimeLastWrite;
	ULONG  cbFile;
	ULONG  cbFileAlloc;
	USHORT attrFile;
	UCHAR  cchName;
	CHAR   achName[13];
} FILEFINDBUF;
typedef FILEFINDBUF far *PFILEFINDBUF;

USHORT pascal far DosFindFirst (PSZ, PUSHORT, USHORT, PFILEFINDBUF, USHORT, PUSHORT, ULONG);
USHORT pascal far DosFindNext (USHORT, PFILEFINDBUF, USHORT, PUSHORT);
USHORT pascal far DosFindClose (USHORT);

#endif

#endif

