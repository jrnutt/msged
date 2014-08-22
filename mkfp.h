/* PUBLIC DOMAIN */

#if !defined(MK_FP)
#define MK_FP(seg,ofs)      ((void far *) (((unsigned long)(seg) << 16) | \
                            (unsigned)(ofs)))
#endif
