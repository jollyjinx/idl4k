/*****************************************************************************

File name   :  osdlib3.c

    Description :  Main file for OSDLIB version driver.

 COPYRIGHT (C) FTA Communication technologies 2007.

Date               Modification                 Name
----               ------------                 ----
 12/07/00          Created                      FR

*****************************************************************************/
/* --- local defines ------------------------------------------------------ */
#define MAX_REG         32
#define TEXT_MODE_BACKGROUND    2

/* ----- define the fonts you want to include ------------------------------*/
#define FONT8
#define FONT13
#define FONT16
#define FONT20
#define FONT24

/* --- includes ----------------------------------------------------------- */
#include "fta/osdlib.h"
#include "font.h"
#include "osdclut.h"
#include "malloc.h"
#include "fta/definition.h"

#define FULL_VERSION    1


/* =======================================================================
   The main OSD Structure
   ======================================================================== */
typedef struct
{   /* --- standard parameters -------------------------------------------- */
    U32                         MagicNumber             ;/* like PI         */
    U32                         MaxOpen                 ;/* max region      */
    /* --- region status -------------------------------------------------- */
    U32                         MaskOpened              ;/* mask of opened  */
    U32                         OpenedHandles           ;/* num of opened r */
    /* --- font characteristics ------------------------------------------- */
    U8                          *FontW[5]               ;/* preprocessed    */
    U32                         *FontP[5]               ;/* preprocessed    */
    U16                         *FontI[5]               ;/* preprocessed    */
    U16                         *FontO[5]               ;/* preprocessed    */
    U32                         FontH[5]                ;/* preprocessed    */
    U32                         FontJ[5]                ;/* preprocessed    */
    U8                          *FontWidth              ;/* current font    */
    U32                         *FontPos                ;/* current pod     */
    U16                         *FontArrayInner         ;/* current inner   */
    U16                         *FontArrayOuter         ;/* current outer   */
    U32                         FontHeight              ;/* current height  */
    U32                         FontJump                ;/* curren char jump*/
    S16                         FontClutInner           ;/* for composition */
    S16                         FontClutOuter           ;/* for composition */
    S16                         FontClutBorder          ;/* for composition */
    U8                          FontClutMethod          ;/* for composition */
    U32                         FontPosX                ;/* for text box    */
    U32                         FontPosY                ;/* for text box    */
    STOSDLIB_Handle_t           FontHandle              ;/* for text box    */
    U32                         FontWinX                ;/* for text box    */
    U32                         FontWinY                ;/* for text box    */
    U32                         FontWinXStop            ;/* for text box    */
    U32                         FontWinYStop            ;/* for text box    */
    /* --- the external visible handles ----------------------------------- */
    STOSDLIB_Handle             OSDRegHandle[MAX_REG];  /* handle list      */
}   OSDHandle_t;


/* --- local variables ---------------------------------------------------- */
static  OSDHandle_t OSDHndl;    /* the main handle                          */

/* ========================================================================
   Name:        STOSDLIB_Init
   Description: Main initialisation phase

   ======================================================================== */
I32  STOSDLIB_Init          ()
{
    BOOL                            Loop;

    if ( OSDHndl.MagicNumber != 0x31415926 )
    {   /* first init time */
        OSDHndl.MagicNumber           = 0x31415926;
        OSDHndl.MaxOpen               = 1;
        OSDHndl.MaskOpened            = 0;
        OSDHndl.OpenedHandles         = 0;
        for (Loop=0;Loop<5;Loop++)
        {   /* mark the fonts as unused                                     */
            OSDHndl.FontW[Loop] = NULL;
        }
    }

    /* at that stage, there is no memory allocation, done in OPEN view port */
    return 0;
}

/****************************************************************************/
/**********************     REGION CONTROL    *******************************/
/****************************************************************************/
/* ========================================================================
   Name:        STOSDLIB_Open
   Description: Create a region, may be virtual ( in ST20 memory ), or visible
                (in SDRAM mpeg + header included )
   ======================================================================== */
I32  STOSDLIB_Open          (STOSDLIB_OpenParams_t *OpenParams_p, STOSDLIB_Handle_t *Handle_p)
{
    U32                         Id=0,Base;
    U32                         *ClutBase=NULL;
    STOSDLIB_Handle_t           Hndl;
    U8                        * PLoc;

    Base = 0;

    /* first find a free id for the handle */
    if ( OSDHndl.OpenedHandles >= OSDHndl.MaxOpen )
    {
        return -1;
    }
    while ( OSDHndl.MaskOpened & (1<<Id))
    {
        Id ++;
    }
    /* now Id is the identity of the new handle to create */
    /* allocates the required memory, can be virtual or in MPEG memory */
    Hndl                    = &OSDHndl.OSDRegHandle[Id];
    *Handle_p               = &OSDHndl.OSDRegHandle[Id];
    Hndl->OSDId             = Id;
    Hndl->OSDWidth          = OpenParams_p->OSDWidth            ;
    Hndl->OSDHeight         = OpenParams_p->OSDHeight           ;
    Hndl->OSDFormat         = OpenParams_p->OSDFormat           ;
    switch ( OpenParams_p->OSDFormat )
    {
        case OSD_RGB8888:       /* RGB 888 on 32 bits slots */
            ClutBase            = &OSDClut888   [0];
            Hndl->OSDBS         = 4;    /* bit shift is 4   : entity is 32 bits */
            break;
        case OSD_YUV    :       /* in fact is CbYCrY */
            ClutBase            = &OSDClutCbYCrY[0];
            Hndl->OSDBS         = 2;    /* bit shift is 2   : entity is 16 bits */
            break;
    }
    Hndl->OSDPitch  = (((Hndl->OSDWidth*Hndl->OSDBS)+3)&~3);   /* align region on 32 bits */

    /* allocates memory only for clut, since region is already allocated by application */
    PLoc                = malloc (  1024 );
    Hndl->OSDBase       = OpenParams_p->OSDAddress ;

    Hndl->OSDPrivateBase    = ((U32)PLoc);
    memcpy                  ( (void*) (Hndl->OSDPrivateBase),(void*) ClutBase, 1024 );   /* copy the default clut at base address */
    Hndl->OSDPrivateNumber  = (U32)Hndl;
    OSDHndl.MaskOpened      |= ( 1<<Id );    /* register the structure */
    OSDHndl.OpenedHandles   ++;           /* number of regions is one more */
    return 0;
}

/* ========================================================================
   Name:        STOSDLIB_SetClut
   Description: Sets clut entry. In clut is negative, means a YCbCr clut

   ======================================================================== */
I32  STOSDLIB_SetClut       ( STOSDLIB_Handle_t  Handle, U8 R, U8 G, U8 B , S32 Clut, U8 Mix )
{
    U32 *PTop,Y,Cr,Cb,i;

    if ( Mix == 0xFE )
    {   /* strange value that resets the mix and sets default clut */
        switch ( Handle->OSDFormat )
        {
            case OSD_RGB8888 :  for(i=0;i<256;i++)
                                    /* Or the color is opaque */
                                    *(U32*)(Handle->OSDPrivateBase + 4*i)= *(OSDClut888 + i) | 0xFF000000ul;
                                break;
            case OSD_YUV     :  memcpy ((void*)(Handle->OSDPrivateBase),(void*)&OSDClutCbYCrY [0], 1024 );    break;
        }
        return 0;
    }

    if ( Clut < 0)
    {
        PTop = (U32*)(Handle->OSDPrivateBase-(Clut<<2));    /* generate clut address */
    }
    else
    {
        PTop = (U32*)(Handle->OSDPrivateBase+(Clut<<2));    /* generate clut address */
    }
    switch ( Handle->OSDFormat )
    {
        case OSD_RGB444     :    *PTop = (((U32)R>>4)<< 8)|(((U32)G>>4)<<4)|((U32)B>>4);       break;
        case OSD_RGB555     :    *PTop = (((U32)R>>3)<<10)|(((U32)G>>3)<<5)|((U32)B>>3);       break;
        case OSD_RGB565     :    *PTop = (((U32)R>>3)<<11)|(((U32)G>>2)<<5)|((U32)B>>3);       break;
        case OSD_RGB888     :    *PTop = ( (U32)R<<16)    |( (U32)G<<8)    |((U32)B<<8);       break;
        case OSD_RGB8888    :    *PTop = ((U32)Mix<<24)   | ((U32)R<<16)   |((U32)G<<8)|((U32)B<<0); break;
        case OSD_YUV        :
        if ( Clut < 0)
        {
            *PTop = (R<<24) | (G<<16) | (R<<8) | (B<<0);
        }
        else
        {
            Y  = (U8) (16  + (( (257*(S32)R)+(504*(S32)G)+(98 *(S32)B))/1000) );
            Cr = (U8) (128 + ((-(148*(S32)R)-(291*(S32)G)+(439*(S32)B))/1000) );
            Cb = (U8) (128 + (( (439*(S32)R)-(368*(S32)G)-(71 *(S32)B))/1000) );
                                 *PTop = (Y<<24) | (Cb<<16) | (Y<<8) | (Cr<<0);        break;
        }
    }
    return 0;
}


/****************************************************************************/
/**********************     TEXT PROCESSING    ******************************/
/****************************************************************************/
/* ========================================================================
   Name:        STOSDLIB_TextInit
   Description: Initializes a text font.

   ======================================================================== */
I32  STOSDLIB_TextInit      ( STOSDLIB_Font_t    Font, U32 ClutInner, U32 ClutOuter, U32 ClutBorder )
{
    I32  ApiError=0;
    U32  Height,Jump,Loop,Sum,Alloc,X,Y,BitShift;
    U16  *FI,*FO,*FB;
    U8   *FW;
    U32  *Pix,*FP;
    U32  Base;
    U8   Array[26][26];

    if ( (U32) Font > (U32) OSD_FONT24 )
    {
        return -1;
    }
    if ( OSDHndl.FontW[Font] != NULL )
    {   /* the font was already allocated   */
        OSDHndl.FontWidth      = OSDHndl.FontW[Font];
        OSDHndl.FontPos        = OSDHndl.FontP[Font];
        OSDHndl.FontArrayInner = OSDHndl.FontI[Font];
        OSDHndl.FontArrayOuter = OSDHndl.FontO[Font];
        OSDHndl.FontHeight     = OSDHndl.FontH[Font];
        OSDHndl.FontJump       = OSDHndl.FontJ[Font];
    }
    else
    {   /* the font was not processed let's unzip it ! */
        switch ( Font )
        {
#ifdef FONT8
            case OSD_FONT8 :
                FW              = (U8*) Font8Width;
                Pix             = (U32*)Font8Pix;
                Height          = 8;
                Jump            = 12;
            break;
#endif
#ifdef FONT13
            case OSD_FONT13 :
                FW              = (U8*) Font13Width;
                Pix             = (U32*)Font13Pix;
                Height          = 16;
                Jump            = 18;
            break;
#endif
#ifdef FONT16
            case OSD_FONT16 :
                FW              = (U8*) Font16Width;
                Pix             = (U32*)Font16Pix;
                Height          = 20;
                Jump            = 20;
            break;
#endif
#ifdef FONT20
            case OSD_FONT20 :
                FW              = (U8*) Font20Width;
                Pix             = (U32*)Font20Pix;
                Height          = 20;
                Jump            = 24;
            break;
#endif
#ifdef FONT24
            case OSD_FONT24 :
                FW              = (U8*) Font24Width;
                Pix             = (U32*)Font24Pix  ;
                Height          = 24;
                Jump            = 30;
            break;
#endif
        default :
            ApiError = -1;
            break;
        }
        if ( ApiError == 0 )
        {
            Sum = 0;
            for (Loop=0;Loop<128;Loop++)
            {
                Sum += FW[Loop];
            }
            Alloc = 128 + (Sum*Height*4);
            Base = (U32) malloc(  Alloc );
            if ( Base == 0 )
            {
                return -1;
            }
            FP                  = (U32*)(Base);
            FI                  = (U16*)(Base+128);
            FO                  = (U16*)(Base+128+(Sum*Height*2));
            OSDHndl.FontW[Font] = FW;
            OSDHndl.FontP[Font] = FP;
            OSDHndl.FontI[Font] = FI;
            OSDHndl.FontO[Font] = FO;
            OSDHndl.FontH[Font] = Height;
            OSDHndl.FontJ[Font] = Jump;
            Sum                 = 0;
            for ( Loop = 0 ; Loop < 128 ; Loop ++ )
            {
                FP[Loop] = Sum;
                BitShift = 31;
                FB = FI + Sum;
                for (Y=1;Y<Height+1;Y++)
                {
                    for(X=1;X<FW[Loop]+1;X++)
                    {
                        if ( (*Pix)&(1<<BitShift) )
                        {
                            *FB = 0x0101;
                        }
                        else
                        {
                            *FB = 0x0000;
                        }
                        Array[X][Y] = *FB;
                        FB ++;
                        if ( BitShift == 0)
                        {
                            BitShift = 31;
                            Pix      ++;
                        }
                        else
                        {
                            BitShift --;
                        }
                    }
                }
                if ( BitShift != 31 )
                {
                    Pix++;
                }
                /* then process aliasing */
#ifdef FULL_VERSION
                FB = FO + (Sum/*>>1*/);
                for (Y=0;Y<Height+2;Y++)
                {
                    Array[0]         [Y]=0;
                    Array[FW[Loop]+1][Y]=0;
                }
                for (X=0;X<FW[Loop]+2;X++)
                {
                    Array[X]       [0]=0;
                    Array[X][Height+1]=0;
                }
                for (Y=1;Y<Height+1;Y++)
                {
                    for(X=1;X<FW[Loop]+1;X++)
                    {
                        if ( Array[X][Y] == 0 )
                        {
                            if ( (Array[X-1][Y-1])||(Array[X ][Y-1])||(Array[X+1][Y-1])||
                                 (Array[X-1][Y  ])                  ||(Array[X+1][Y  ])||
                                 (Array[X-1][Y+1])||(Array[X ][Y+1])||(Array[X+1][Y+1]) )
                            {
                                *FB = 0x0101;
                            }
                            else
                            {
                                *FB = 0x0000;
                            }
                        }
                        else
                        {
                            *FB = 0;
                        }
                        FB ++;
                    }
                }
#endif
                Sum += (Height * FW[Loop]);
            }
        }
		/* the font IS allocated now */
		OSDHndl.FontWidth      = OSDHndl.FontW[Font];
        OSDHndl.FontPos        = OSDHndl.FontP[Font];
        OSDHndl.FontArrayInner = OSDHndl.FontI[Font];
        OSDHndl.FontArrayOuter = OSDHndl.FontO[Font];
        OSDHndl.FontHeight     = OSDHndl.FontH[Font];
        OSDHndl.FontJump       = OSDHndl.FontJ[Font];
    }
    if ( (OSDHndl.FontClutInner != -1 ) && (OSDHndl.FontClutOuter!=-1) && (OSDHndl.FontClutBorder!=-1))
    {
        OSDHndl.FontClutInner   = ClutInner ;
        OSDHndl.FontClutOuter   = ClutOuter ;
        OSDHndl.FontClutBorder  = ClutBorder;
        OSDHndl.FontClutMethod  = ((OSDHndl.FontClutInner !=0 )<<2) |
                                  ((OSDHndl.FontClutOuter !=0 )<<1) |
                                   (OSDHndl.FontClutBorder!=0);
    }
    return ApiError;
}

/* ========================================================================
   Name:        STOSDLIB_TextXY
   Description: Generic text print, with no regards to the windowing
                parameters
   ======================================================================== */
I32  STOSDLIB_TextXY        ( STOSDLIB_Handle_t  Handle, U16 X, U16 Y, U8 * Text , U32 ClutInner, U32 ClutOuter, U32 ClutBorder )
{
    U16     Id = 0;
    U32     Base, ClutMethod;
    U8      Val;
    U32     Wb;
    U32     Width=0;
    U32                 X1,Y1,Ba;
#ifdef FULL_VERSION
    U16                 *O;
#endif
    U16                 *I;

    Base    = OXYBase   (Handle,X,Y);
    Val     = Text[Id];

    if ( ClutInner !=0)  ClutInner = *(U32*) (Handle->OSDPrivateBase+(ClutInner <<2));
    if ( ClutOuter !=0)  ClutOuter = *(U32*) (Handle->OSDPrivateBase+(ClutOuter <<2));
    if ( ClutBorder!=0)  ClutBorder= *(U32*) (Handle->OSDPrivateBase+(ClutBorder<<2));
    ClutMethod          = ((ClutInner!=0 )<<2) |((ClutOuter!=0 )<<1) |(ClutBorder!=0);

    while ((Val >= 32)||Val=='\n' /*&& ( Val < 128)*/ )
    {
        Width   = OSDHndl.FontWidth[Val];
        Wb      = OSDHndl.FontWidth[Val]*Handle->OSDBS;
/****************************************************************************/
        if ( Handle->OSDBS == 2 )
        {
            switch ( ClutMethod )
            {
            case 0 :    break;
            case 1 :    break;
            case 2 :
#ifdef FULL_VERSION
                O =     (U16*)&OSDHndl.FontArrayOuter[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=2)
                    {
                        if ( *O )
                        {
                            *(U16*)(Ba+X1)=ClutOuter;
                        }
                        O++;
                    }
                    Ba += Handle->OSDPitch;
                }
#endif
                break;
            case 3 :
#ifdef FULL_VERSION
                O =     (U16*)&OSDHndl.FontArrayOuter[OSDHndl.FontPos[Val]];
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=2)
                    {
                        if ( *O )
                        {
                            *(U16*)(Ba+X1)=ClutOuter;
                        }
                        else
                        {
                            /* if ( ! *I ) */
                            {
                                *(U16*)(Ba+X1)=ClutBorder;
                            }
                        }
                        O++;
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
#endif
                break;
            case 4 :
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=2)
                    {
                        if ( *I )
                        {
                            *(U16*)(Ba+X1)=ClutInner;
                        }
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            case 5 :
#ifdef FULL_VERSION
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=2)
                    {
                        if ( *I )
                        {
                            *(U16*)(Ba+X1)=ClutInner;
                        }
                        else
                        {
                            *(U16*)(Ba+X1)=ClutBorder;
                        }
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
#endif
                break;
            case 6 :
#ifdef FULL_VERSION
                O =     (U16*)&OSDHndl.FontArrayOuter[OSDHndl.FontPos[Val]];
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=2)
                    {
                        if ( *O )
                        {
                            *(U16*)(Ba+X1)=ClutOuter;
                        }
                        else
                        {
                            if ( *I )
                            {
                                *(U16*)(Ba+X1)=ClutInner;
                            }
                        }
                        O++;
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
#endif
                break;
            case 7 :
#ifdef FULL_VERSION
                O =     (U16*)&OSDHndl.FontArrayOuter[OSDHndl.FontPos[Val]];
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=2)
                    {
                        if ( *O )
                        {
                            *(U16*)(Ba+X1)=ClutOuter;
                        }
                        else
                        {
                            if ( *I )
                            {
                                *(U16*)(Ba+X1)=ClutInner;
                            }
                            else
                            {
                                *(U16*)(Ba+X1)=ClutBorder;
                            }
                        }
                        O++;
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
#endif
                break;
            }
        }
/****************************************************************************/
#ifdef FULL_VERSION
        else if ( Handle->OSDBS == 3 )
        {
            switch ( ClutMethod )
            {
            case 0 :    break;
            case 1 :    break;
            case 2 :
                O =     (U16*)&OSDHndl.FontArrayOuter[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=3)
                    {
                        if ( *O )
                        {
                            *(U8*)(Ba+X1+0)=(U8)(ClutOuter>> 0);
                            *(U8*)(Ba+X1+1)=(U8)(ClutOuter>> 8);
                            *(U8*)(Ba+X1+2)=(U8)(ClutOuter>>16);
                        }
                        O++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            case 3 :
                O =     (U16*)&OSDHndl.FontArrayOuter[OSDHndl.FontPos[Val]];
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=3)
                    {
                        if ( *O )
                        {
                            *(U8*)(Ba+X1+0)=(U8)(ClutOuter>> 0);
                            *(U8*)(Ba+X1+1)=(U8)(ClutOuter>> 8);
                            *(U8*)(Ba+X1+2)=(U8)(ClutOuter>>16);
                        }
                        else
                        {
                            *(U8*)(Ba+X1+0)=(U8)(ClutBorder>> 0);
                            *(U8*)(Ba+X1+1)=(U8)(ClutBorder>> 8);
                            *(U8*)(Ba+X1+2)=(U8)(ClutBorder>>16);
                        }
                        O++;
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            case 4 :    /* Draw inner only  */
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=3)
                    {
                        if ( *I )
                        {
                            *(U8*)(Ba+X1+0)=(U8)(ClutInner>> 0);
                            *(U8*)(Ba+X1+1)=(U8)(ClutInner>> 8);
                            *(U8*)(Ba+X1+2)=(U8)(ClutInner>>16);
                        }
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            case 5 :    /* Draw inner + border  */
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=3)
                    {
                        if ( *I )
                        {
                            *(U8*)(Ba+X1+0)=(U8)(ClutInner>> 0);
                            *(U8*)(Ba+X1+1)=(U8)(ClutInner>> 8);
                            *(U8*)(Ba+X1+2)=(U8)(ClutInner>>16);
                        }
                        else
                        {
                            *(U8*)(Ba+X1+0)=(U8)(ClutBorder>> 0);
                            *(U8*)(Ba+X1+1)=(U8)(ClutBorder>> 8);
                            *(U8*)(Ba+X1+2)=(U8)(ClutBorder>>16);
                        }
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            case 6 :    /* Draw outer + inner only  */
                O =     (U16*)&OSDHndl.FontArrayOuter[OSDHndl.FontPos[Val]];
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=3)
                    {
                        if ( *O )
                        {
                            *(U8*)(Ba+X1+0)=(U8)(ClutOuter>> 0);
                            *(U8*)(Ba+X1+1)=(U8)(ClutOuter>> 8);
                            *(U8*)(Ba+X1+2)=(U8)(ClutOuter>>16);
                        }
                        else
                        {
                            if ( *I )
                            {
                                *(U8*)(Ba+X1+0)=(U8)(ClutInner>> 0);
                                *(U8*)(Ba+X1+1)=(U8)(ClutInner>> 8);
                                *(U8*)(Ba+X1+2)=(U8)(ClutInner>>16);
                            }
                        }
                        O++;
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            case 7 :    /* Draw inner + outer + border only  */
                O =     (U16*)&OSDHndl.FontArrayOuter[OSDHndl.FontPos[Val]];
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=3)
                    {
                        if ( *O )
                        {
                            *(U8*)(Ba+X1+0)=(U8)(ClutOuter>> 0);
                            *(U8*)(Ba+X1+1)=(U8)(ClutOuter>> 8);
                            *(U8*)(Ba+X1+2)=(U8)(ClutOuter>>16);
                        }
                        else
                        {
                            if ( *I )
                            {
                                *(U8*)(Ba+X1+0)=(U8)(ClutInner>> 0);
                                *(U8*)(Ba+X1+1)=(U8)(ClutInner>> 8);
                                *(U8*)(Ba+X1+2)=(U8)(ClutInner>>16);
                            }
                            else
                            {
                                *(U8*)(Ba+X1+0)=(U8)(ClutBorder>> 0);
                                *(U8*)(Ba+X1+1)=(U8)(ClutBorder>> 8);
                                *(U8*)(Ba+X1+2)=(U8)(ClutBorder>>16);
                            }
                        }
                        O++;
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            }
        }
/****************************************************************************/
        else
        {
            switch ( ClutMethod )
            {
            case 0 :    break;
            case 1 :    break;
            case 2 :
                O =     (U16*)&OSDHndl.FontArrayOuter[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=4)
                    {
                        if ( *O )
                        {
                            *(U32*)(Ba+X1)=ClutOuter;
                        }
                        O++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            case 3 :
                O =     (U16*)&OSDHndl.FontArrayOuter[OSDHndl.FontPos[Val]];
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=4)
                    {
                        if ( *O )
                        {
                            *(U32*)(Ba+X1)=ClutOuter;
                        }
                        else
                        {
                            *(U32*)(Ba+X1)=ClutBorder;
                        }
                        O++;
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            case 4 :
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=4)
                    {
                        if ( *I )
                        {
                            *(U32*)(Ba+X1)=ClutInner;
                        }
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            case 5 :
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=4)
                    {
                        if ( *I )
                        {
                            *(U32*)(Ba+X1)=ClutInner;
                        }
                        else
                        {
                            *(U32*)(Ba+X1)=ClutBorder;
                        }
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            case 6 :
                O =     (U16*)&OSDHndl.FontArrayOuter[OSDHndl.FontPos[Val]];
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=4)
                    {
                        if ( *O )
                        {
                            *(U32*)(Ba+X1)=ClutOuter;
                        }
                        else
                        {
                            if ( *I )
                            {
                                *(U32*)(Ba+X1)=ClutInner;
                            }
                        }
                        O++;
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            case 7 :
                O =     (U16*)&OSDHndl.FontArrayOuter[OSDHndl.FontPos[Val]];
                I =     (U16*)&OSDHndl.FontArrayInner[OSDHndl.FontPos[Val]];
                Ba =    Base;
                for (Y1=0;Y1<OSDHndl.FontHeight;Y1++)
                {
                    for (X1=0;X1<Wb;X1+=4)
                    {
                        if ( *O )
                        {
                            *(U32*)(Ba+X1)=ClutOuter;
                        }
                        else
                        {
                            if ( *I )
                            {
                                *(U32*)(Ba+X1)=ClutInner;
                            }
                            else
                            {
                                *(U32*)(Ba+X1)=ClutBorder;
                            }
                        }
                        O++;
                        I++;
                    }
                    Ba += Handle->OSDPitch;
                }
                break;
            }
        }
#endif
/****************************************************************************/
        OSDHndl.FontPosX += Width;
        Base += (Width*Handle->OSDBS);
        Id++;
        Val = Text[Id];
    }
	return Width;
}

/* ========================================================================
   Name:        STOSDLIB_TextSize
   Description: Returns the size for the selected font

   ======================================================================== */
U32 STOSDLIB_TextSize ( U8* Text )
{
    U32 Size = 0,Id= 0;
    S16  Val;

    Val = Text[Id];
    while ((Val >= 32) && ( Val < 128) )
    {
        Size += OSDHndl.FontWidth[Val];
        Id++;
        Val = Text[Id];
    }
    return ( Size );
}

/* ========================================================================
   Name:        getCharWidth
   Description: get the width of a character
   ======================================================================== */
U32 getCharWidth (U8 Val)
{
	return	OSDHndl.FontWidth[Val];
}

/* ========================================================================
   Name:        getCharHeight
   Description: get the height of the current font
   ======================================================================== */
U32 getCharHeight (void)
{
	return	OSDHndl.FontHeight;
}


/* ------------------------------- End of file ---------------------------- */
