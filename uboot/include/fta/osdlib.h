/*****************************************************************************

File name   :  osdlib.h

Description :  header for osdlib library

 COPYRIGHT (C) FTA Communication techniologies 2007.

Date               Modification                 Name
----               ------------                 ----
 11/23/00          Created                      FR

*****************************************************************************/
/* --- prevents recursive inclusion --------------------------------------- */
#ifndef OSDLIB_H
#define OSDLIB_H

/* --- includes ----------------------------------------------------------- */
#include "fta/types.h"

/* --- local defines ------------------------------------------------------ */
/* --- local options and variables ---------------------------------------- */
/* --- local enumerations ------------------------------------------------- */
/* =======================================================================
   Defines the fonts to use
   ======================================================================== */
typedef enum
{
    OSD_FONT8                   =0,     /* use font of height 8 pixels                */
    OSD_FONT13                  =1,     /* use font of height 13 pixels               */
    OSD_FONT16                  =2,     /* use font of height 16 pixels               */
    OSD_FONT20                  =3,     /* use font of height 20 pixels               */
    OSD_FONT24                  =4      /* use font of height 24 pixels               */
}   STOSDLIB_Font_t;

/* ========================================================================
   Defines resolution to use
   ======================================================================== */
typedef enum
{
    OSD_RGB444                  =0,     /* use RGB 444 and trash 4 upper bytes                                  */
    OSD_RGB555                  =1,     /* use RGB i555 (the most commonly used) 16 bits                        */
    OSD_RGB565                  =2,     /* use RGB 565 (from 8810)               16 bits                        */
    OSD_RGB888                  =3,     /* use RGB 24 on 24 bits data slots      24 bits                        */
    OSD_RGB8888                 =4,     /* use RGB 24 on 32 bits data slots      32 bits                        */
    OSD_YUV                     =5      /* use YUV interleaved                   16 bits but 32 bits cluts      */
}   STOSDLIB_Resolution_t;

/* ========================================================================
   Defines Main Params to open and create a region
   ======================================================================== */
typedef struct
{
    U32                         OSDAddress          ;   /* inform about address             */
    U16                         OSDWidth            ;   /* width of region                  */
    U16                         OSDHeight           ;   /* height of region                 */
    U32                         OSDFormat           ;   /* format of OSD - adaptation       */
}   STOSDLIB_OpenParams_t;


/* =======================================================================
   What is visible from the handle for the application
   ======================================================================== */
typedef struct
{
    /* --- these are the 4 key parameters --------------------------------- */
    U32                         OSDWidth            ;   /* width of region                  */
    U32                         OSDHeight           ;   /* height of region                 */
    U32                         OSDBase             ;   /* base address of region           */
    U32                         OSDPitch            ;   /* pitch in bytes                   */
    /* --- these are additionnal parameters, for the driver mainly -------- */
    /* --- they should not be used by the application, but ....    -------- */
    U32                         OSDId               ;   /* internal Id                      */
    U32                         OSDPrivateBase      ;   /* internal private base            */
    U32                         OSDPrivateNumber    ;   /* internal proprietary number      */
    U32                         OSDBS               ;   /* number of bits per pixels        */
    U32                         OSDFormat           ;   /* OSD format& conversion           */
}   STOSDLIB_Handle;

/* =======================================================================
   How to compute the pixel address for OSD ....( can be used externally )
   ======================================================================== */
#define OXYBase(Handle,X,Y)     (Handle->OSDBase)+((U32)(X)*(Handle->OSDBS))+(((U32)(Y))*(Handle->OSDPitch))

typedef STOSDLIB_Handle * STOSDLIB_Handle_t;

/*
 * Function prototypes.
 */
I32 STOSDLIB_Init    (void);
I32 STOSDLIB_Open    (STOSDLIB_OpenParams_t *OpenParams_p, STOSDLIB_Handle_t *Handle_p);
I32 STOSDLIB_TextInit( STOSDLIB_Font_t    Font, U32 ClutInner, U32 ClutOuter, U32 ClutBorder );
I32 STOSDLIB_SetClut ( STOSDLIB_Handle_t  Handle, U8 R, U8 G, U8 B , S32 Clut, U8 Mix );
U32 STOSDLIB_TextSize( U8* Text );
I32 STOSDLIB_TextXY  ( STOSDLIB_Handle_t  Handle, U16 X, U16 Y, U8 * Text , U32 ClutInner, U32 ClutOuter, U32 ClutBorder );
U32 getCharWidth     ( U8 Val );
U32 getCharHeight    (void);

#endif	/* OSDLIB_H__ */
