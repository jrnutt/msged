/* PUBLIC DOMAIN */

#if defined(__TURBOC__)

#define _near near
#define _far far
#define _cdecl cdecl
#define _pascal pascal

#endif

#ifdef __OS2__

#define _near far
#define _far far
#define _cdecl cdecl
#define _pascal pascal

#endif

