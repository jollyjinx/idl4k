/*****************************************************************************

File name   :  zutil.c

Description :  Some utility functions for compression, decompression engines

 COPYRIGHT (C) Inverto Digital Labs 2009.

Date               Modification                                          Name
----               ------------                                          ----
 02/03/10          Created                                               FR

*****************************************************************************/
/* --- includes ----------------------------------------------------------- */
#include "zlib.h"	/* main header */

/* --- local defines ------------------------------------------------------ */
#define BASE            65521UL    /* largest prime smaller than 65536 */
#define NMAX            5552       /* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

/* --- local variables ---------------------------------------------------- */
/* --- local enumerations ------------------------------------------------- */
/* --- prototypes of local functions and static --------------------------- */
/* --- functions in this driver ------------------------------------------- */

/* ========================================================================
   Name:        adler32f
   Description: perform adler32
                
   ======================================================================== */
unsigned long  adler32f(unsigned long adler, const unsigned char *buf, unsigned int len)
{
    unsigned long s1 = adler & 0xffff , s2 = (adler >> 16) & 0xffff, k;
#if defined(ST40_OPTIM)
    unsigned char * bufadv;
#endif
    if (buf == Z_NULL) return 1L;	/* initial value */
#if defined(ST40_OPTIM)
    bufadv = (unsigned char*) (((int)buf) );
    __asm__("pref @%0" : :"r"(bufadv) :"memory");
    bufadv+=32;
#endif
    while (len > 0) 
    {
        k = len < NMAX ? (int)len : NMAX;
        len -= k;
        while (k >= 8) 
        {
#if defined (ST40_OPTIM)
            __asm__("pref @%0" : :"r"(bufadv) :"memory"); /* prefetch without blocking the CPU */
            bufadv += 8;
#endif
            s1 += *buf++;    s2 += s1;
            s1 += *buf++;    s2 += s1;
            s1 += *buf++;    s2 += s1;
            s1 += *buf++;    s2 += s1;

            s1 += *buf++;    s2 += s1;
            s1 += *buf++;    s2 += s1;
            s1 += *buf++;    s2 += s1;
            s1 += *buf++;    s2 += s1;
            k -=8;
        }
        while (k--)	/* do the remaining ones */
        {
            s1 += *buf++;   s2 += s1;
        }
        s1 = s1 % BASE;	/* modulo */
        s2 = s2 % BASE;	/* modulo */
    }
    return (s2 << 16) | s1;
}
/* ------------------------------- End of file ---------------------------- */

