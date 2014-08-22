/*
 * released into the PUBLIC DOMAIN 30 jul 1990 by jim nutt
*/

#include <stdlib.h>
#include <errno.h>
#include <dos.h>

#define _pascal pascal

#ifdef __OS2__
#include <sys\types.h>
#define INCL_DOSPROCESS
#include <os2.h>
#include <sys\stat.h>
#include <ctype.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>

static struct _FILEFINDBUF InfoBuf;
static struct find_t dta;

HDIR	hDir;
USHORT	cSearch;
USHORT	usAttrib;

#define FILENAMELEN 13

int _pascal dir_findfirst(char * filename, int attribute, struct find_t * dta)
{
	hDir	 = 0x0001;
	usAttrib = attribute;
	cSearch  = 1;

#ifdef DEBUG
	printf("\nDIR_FINDFIRST Inputs: '%s' %d.\n", filename, attribute);
#endif /* of DEBUG */

	if (DosFindFirst( filename
					, &hDir
					, usAttrib
					, &InfoBuf
					, (USHORT)( sizeof(InfoBuf) * cSearch )
					, &cSearch
					, (ULONG)NULL ) != 0 )
	{
#ifdef DEBUG
	printf("DIR_FINDFIRST:  DosFindFirst returned <>0.\n");
#endif /* of DEBUG */
		DosFindClose( hDir );
		errno = ENOENT;
		return (-1);
	} else {
#ifdef DEBUG
	printf("DIR_FINDFIRST:  DosFindFirst returned 0.\n");
	printf("DIR_FINDFIRST:  attrFile = %d.\n", InfoBuf.attrFile);
#endif /* of DEBUG */
		dta->attrib    = (char)InfoBuf.attrFile;		   /**OS2**/
		dta->size	   = InfoBuf.cbFile;
		strcpy( dta->name, InfoBuf.achName);
		errno = 0;
		return (0);
	}
}

int _pascal dir_findnext(struct find_t * dta)
{

	if ((DosFindNext( hDir
					, &InfoBuf
					, (USHORT)(FILENAMELEN + 23)
					, &cSearch)
					) || (cSearch != 1))
	{
		DosFindClose( hDir );
		errno = ENOENT;
		return (-1);
	} else {
		dta->attrib    = (char)InfoBuf.attrFile;		   /**OS2**/
		dta->size	   = InfoBuf.cbFile;
		strcpy(  dta->name, InfoBuf.achName);
		errno = 0;
		return (0);
	}
}

#else /* of __OS2__ */

struct _dta   {
	char		reserved[21];
	char		attribute;
	unsigned	time;
	unsigned	date;
	long		size;
	char		name[13];
};

static int olddta = 0;

int _pascal dir_findnext(struct _dta * dta);
int _pascal dir_findfirst(char * filename, int attribute, struct _dta * dta);

int _pascal dir_findfirst(char * filename, int attribute, struct _dta * dta)

{
	union REGS	ir;
	union REGS	or;
#ifndef SPTR
	struct SREGS	sr;
#endif

	ir.h.ah = 0x1a;
#ifdef SPTR
	ir.x.dx = (unsigned int) dta;
	intdos(&ir,&or);
#else
	ir.x.dx = FP_OFF(dta);
	sr.ds = FP_SEG(dta);
	intdosx(&ir, &or, &sr);
#endif
	ir.h.ah = 0x4e;
	ir.x.cx = (unsigned int) attribute;
#ifdef SPTR
	ir.x.dx = (unsigned int) filename;
	intdos(&ir,&or);
#else
	ir.x.dx = FP_OFF(filename);
	sr.ds = FP_SEG(filename);
	intdosx(&ir, &or, &sr);
#endif
	if (or.x.cflag) {
		errno = ENOENT;
		return (-1);
	}

	errno = 0;
	return (0);
}

int _pascal dir_findnext(struct _dta * dta)

{
	union REGS ir, or;
#ifndef SPTR
	struct SREGS	sr;
#endif

	ir.h.ah = 0x1a;

	if (!olddta) {

#ifdef SPTR
		ir.x.dx = (unsigned int) dta;
		intdos(&ir,&or);
#else
		ir.x.dx = FP_OFF(dta);
		sr.ds = FP_SEG(dta);
		intdosx(&ir, &or, &sr);
#endif
		olddta = 1;
	}

	ir.h.ah = 0x4f;
	intdos(&ir,&or);

	if (or.x.cflag) {
		errno = ENOENT;
		return (-1);
	} else {
		errno = 0;
		return (0);
	}
}
#endif /* of __OS2__ */

