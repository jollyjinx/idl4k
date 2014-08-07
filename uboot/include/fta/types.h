/*****************************************************************************

File Name   : stddefs.h

Description : Contains a number of generic type declarations and defines.

 COPYRIGHT (C) FTA Communication techniologies 2007.

*****************************************************************************/
/* Define to prevent recursive inclusion ---------------------------------- */
#ifndef DEFS_H__
#define DEFS_H__

#include <config.h>

/* Exported Types --------------------------------------------------------- */
#ifndef NULL
#define NULL ((void*) 0)
#endif

/* --- Common unsigned types ---------------------------------------------- */
typedef unsigned char    BYTE;
typedef unsigned char    U8 ;
typedef unsigned short   U16;
typedef unsigned int     U32;
typedef unsigned long    UL32;

/* --- Common signed types   ---------------------------------------------- */
typedef signed   char    S8 ;
typedef signed   short   S16;
typedef signed   int     S32;
typedef signed   long    SL32;

/* --- Common default types ----------------------------------------------- */
typedef          char    I8 ;
typedef          short   I16;
typedef          int     I32;
typedef          long    IL32;

/* --- specific calls ----------------------------------------------------- */
typedef          float    FLOAT;
typedef          double   DOUBLE;

/* --- Common unsigned types ( device access ) ---------------------------- */
typedef volatile unsigned int   DU32;
typedef volatile unsigned short DU16;
typedef volatile unsigned char  DU8 ;

/* --- Common signed types ( device access )  ----------------------------- */
typedef volatile signed int     DS32;
typedef volatile signed short   DS16;
typedef volatile signed char    DS8 ;

/* --- Common default types ( device access ) ----------------------------- */
typedef volatile        int     DI32;
typedef volatile        short   DI16;
typedef volatile        char    DI8 ;

/* Boolean type (values should be among TRUE and FALSE constants only) ---- */
#ifndef BOOL
    typedef int BOOL;
#endif

/* BOOL type constant values ---------------------------------------------- */
#ifndef TRUE
    #define TRUE (1 == 1)
#endif
#ifndef FALSE
    #define FALSE (!TRUE)
#endif

typedef struct
{
    U32 address;
    U32 data   ;
}   Register;

/* Front Panel keys type constant values ---------------------------------------------- */
#define FP_KEY_NONE    0
#define FP_KEY_UP      1
#define FP_KEY_DOWN    2
#define FP_KEY_POWER   4

#define FP_KEY_OK      16
#define FP_KEY_MENU    32
#define FP_KEY_RIGHT   64
#define FP_KEY_LEFT    128
#define FP_KEY_EXIT    256

#define ST40_OPTIM

#define SELFTEST_OFF    '0'
#define SELFTEST_ON     '1'

#define SELFSTART_V2F   '0'
#define SELFSTART_UBOOT '1'
#define SELFSTART_NFS   '2'

#define JFFS2_MAGIC_BITMASK 0x1985
#define V2FS_MAGIC_BITMASK  0xA198

#endif  /* #ifndef DEFS_H__ */

/* End of defs.h  ----------------------------------------------------------- */

