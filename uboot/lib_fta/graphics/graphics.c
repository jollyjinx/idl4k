/*****************************************************************************

File name   :  vfd.c

Description :  small VFD driver

 COPYRIGHT (C) FTA Communication technologies 2007.

Date               Modification                                          Name
----               ------------                                          ----
07/04/08           Created                                               FR

*****************************************************************************/
/* --- include ------------------------------------------------------------ */
#include "fta/osdlib.h"
#include <common.h>
#include <devices.h>
#include <version.h>
#include "../iboot/iboot.h"

/* --- Include the bitmap  ------------------------------------------------ */
#include "bmp/inverto.h"
#include "bmp/sat.h"
#include "bmp/selevision.h"
#include "bmp/hdplus.h"

/* --- local structures --------------------------------------------------- */
typedef struct
{
    I8  msg[32];
    I32 value;
}   ProgressBar_t;

/* --- local variables ---------------------------------------------------- */
static STOSDLIB_Handle_t	OSDMain;
static U8					SamplePattern[768*8] ALIGN(4);
static BOOL 				useProgressBar = FALSE;
static U32					Y_VER_COORD = 90;
static U32					Y_BMP_COORD;
static ProgressBar_t		ProgressBar;


/* --- defines ------------------------------------------------------------ */
/* when using the TV to display the bitmap */
#define OSD_BMP_FONT		OSD_FONT20

#define TRANSPARENT_CLUT	0
#define BAR_BACKGROUND_CLUT	1
#define BAR_FOREGROUND_CLUT	2
#define BAR_PROGRESS_CLUT	3
#define BAR_TEXT_CLUT		4
#define VERSION_TEXT_CLUT	5

#define BAR_BORDER_X		2*60	/* must be 2 aligned */
#define BAR_BORDER_Y		2*10	/* must be 2 aligned */
#define FILL_BAR_BORDER_X	2*2	/* must be 2 aligned */
#define FILL_BAR_BORDER_Y	2*4	/* must be 2 aligned */
#define BMP_MESSAGE			"Booting"

/* bitmap variables and prototypes ---------------------------------------- */
#define GetByte(Length,Result) {    BBB = 32-Length; \
    if ((BPos + Length) > 31) {    \
        Result =((*Bitmap++)&(0xFFFFFFFF>>BPos))<<(BPos-BBB);      \
        if ((BPos - BBB) != 0) Result |= ((*Bitmap) >> (BBB + 32 - BPos)); \
        BPos-=BBB;   } else {    \
        Result = ((*Bitmap) >> (BBB - BPos))&(0xFFFFFFFF >> (BBB)); \
        BPos += Length; } }

#define ST20Fill1D(Format,Dest,Width) { memcpy((void*)(Dest),(void*)(((U32)&SamplePattern[0])+((Format)==OSD_YUV?(Dest)&3:0)),(Width)); }

/* =======================================================================
   prototypes of functions in this driver
   ======================================================================== */
static I32  STOSDLIB_CompPixmap    ( STOSDLIB_Handle_t  Handle, U16 X, U32 Y, const U32 * Pixmap );
static void fillrect(STOSDLIB_Handle_t OSD, U32 XDO, U32 YDO, U32 width, U32 height, U32 color);
static void progress_bar_init(STOSDLIB_Handle_t OSD);
static void blitText(STOSDLIB_Handle_t OSD, U8 *message);
static void load_bmp ( U32 customer );
static void osd_progress_bar_fill(I8* message, I32 percent);
static void progress_bar_fill(STOSDLIB_Handle_t OSD, I8* message, I32 percent);
static void STOSDLIB_ClrScr ( STOSDLIB_Handle_t  Handle, U32 Clut );
static void splash_version(void);

/* ========================================================================
   Name:        init_osd
   Description: initialize OSD driver and font processor (unpack)

   ======================================================================== */
void init_osd()
{
    STOSDLIB_OpenParams_t Open  = { RAM_DISPLAY_ADDRESS, 720, 576, OSD_RGB8888 };
    STOSDLIB_Init       ();
    STOSDLIB_Open       (&Open  , &OSDMain);

    load_bmp( board_getConfig(1) & PIO_CONFIG_BMP_MASK );

    useProgressBar = TRUE;
    ProgressBar.value = 1;
    progress_bar_init(OSDMain);

    /* blit our message in the middle of the screen */
    splash_version();
    splash_text((I8*)BMP_MESSAGE);

    sh_flush_cache_all();
}

/* ========================================================================
   Name:        splash_load
   Description: loads the pre-defined splash screens according to pre-defined
                settings
   ======================================================================== */
static void load_bmp ( U32 customer )
{
    /* Select and init the font size */
    STOSDLIB_TextInit   ( OSD_BMP_FONT, 1,2,3 );

    if (customer==CFG_BMP_HDPLUS)
    {   /* --- hdplus ----- */
        STOSDLIB_SetClut       ( OSDMain, 0x00,0x00,0x00,0,255);
        STOSDLIB_SetClut       ( OSDMain, 0xF0,0xF0,0xF0,1,255);
        STOSDLIB_ClrScr        ( OSDMain, 1 );
        STOSDLIB_CompPixmap    ( OSDMain, 135 , 154, hdplus );

        Y_BMP_COORD = 500;
    }
    else if (customer==CFG_BMP_BRANDLESS)
    {   /* --- sat ---- */
        STOSDLIB_SetClut       ( OSDMain, 0xFF,0xFF,0xFF,0,255);
        STOSDLIB_ClrScr        ( OSDMain, 0 );
        STOSDLIB_CompPixmap    ( OSDMain, 215, 108, sat );

        Y_BMP_COORD = 500;
    }
    else if (customer==CFG_BMP_SELEVISION)
    {   /* --- selevision --- */
        STOSDLIB_SetClut       ( OSDMain, 0x00,0x00,0x00,0,255);   /* Clut[0] = new color           */
        STOSDLIB_ClrScr        ( OSDMain, 0 );                     /* clear screen with Clut[0]     */
        STOSDLIB_SetClut       ( OSDMain, 255,255,255,1,255);      /* Clut[1] used when transparent */
        STOSDLIB_CompPixmap    ( OSDMain, 113, 140, selevision );  /* display the bitmap            */

        Y_BMP_COORD = 450;
    }
    else
    {   /* --------- inverto ----------- */
        STOSDLIB_SetClut       ( OSDMain, 0x23,0x23,0x23,0,255);
        STOSDLIB_SetClut       ( OSDMain, 0x91,0x91,0x91,1,255);
        STOSDLIB_SetClut       ( OSDMain, 0x5A,0x5A,0x5A,2,255);
        STOSDLIB_SetClut       ( OSDMain, 0x9D,0x1D,0x32,3,255);
        STOSDLIB_SetClut       ( OSDMain, 0xB6,0x56,0x65,4,255);
        STOSDLIB_SetClut       ( OSDMain, 0xE6,0xBC,0xCC,5,255);
        STOSDLIB_SetClut       ( OSDMain, 0xF0,0xF0,0xF0,6,255);
        STOSDLIB_SetClut       ( OSDMain, 0x23,0x23,0x23,7,255);
        STOSDLIB_ClrScr        ( OSDMain, 6 );
        STOSDLIB_CompPixmap    ( OSDMain, 280, 205, inverto );
        STOSDLIB_SetClut       ( OSDMain, 0xF0,0xF0,0xF0,3,255);

        Y_BMP_COORD = 450;
    }

    /* Set the different Cluts */
    if(customer==CFG_BMP_SELEVISION)
    {
        STOSDLIB_SetClut       ( OSDMain, 0x00,0x00,0x00,TRANSPARENT_CLUT,   255);
        STOSDLIB_SetClut       ( OSDMain, 0x48,0x48,0x48,BAR_BACKGROUND_CLUT,255);
        STOSDLIB_SetClut       ( OSDMain, 0x80,0x80,0x80,BAR_FOREGROUND_CLUT,255);
        STOSDLIB_SetClut       ( OSDMain, 0xFD,0xA5,0x22,BAR_PROGRESS_CLUT,  255);
        STOSDLIB_SetClut       ( OSDMain, 0x00,0x00,0x00,BAR_TEXT_CLUT,      255);
        STOSDLIB_SetClut       ( OSDMain, 0x80,0x80,0x80,VERSION_TEXT_CLUT,  255);
    }
    else
    {
        STOSDLIB_SetClut       ( OSDMain, 0x00,0x00,0x00,TRANSPARENT_CLUT,   255);
        STOSDLIB_SetClut       ( OSDMain, 0x80,0x80,0x80,BAR_BACKGROUND_CLUT,255);
        STOSDLIB_SetClut       ( OSDMain, 0xB8,0xB8,0xB8,BAR_FOREGROUND_CLUT,255);
        STOSDLIB_SetClut       ( OSDMain, 0xFF,0xFF,0xFF,BAR_PROGRESS_CLUT,  255);
        STOSDLIB_SetClut       ( OSDMain, 0x00,0x00,0x00,BAR_TEXT_CLUT,      255);
        STOSDLIB_SetClut       ( OSDMain, 0x00,0x00,0x00,VERSION_TEXT_CLUT,  255);
    }
}

/* ========================================================================
   Name:        blitText
   Description: blit our message in the middle of the screen
   ======================================================================== */
static void blitText(STOSDLIB_Handle_t OSD, U8 *message)
{
    if(!message)
        message = (U8*)BMP_MESSAGE;
    U32 textSize;
    textSize = STOSDLIB_TextSize( (U8*)message );
    STOSDLIB_TextXY ( OSD, (OSD->OSDWidth-textSize)>>1, Y_BMP_COORD, (U8*)(U32)message , BAR_TEXT_CLUT, 0, 0 );
    sh_flush_cache_all();
}

/* ========================================================================
   Name:        progress_bar_init
   Description: display an empty progress bar on the TV screen
   ======================================================================== */
static void progress_bar_init(STOSDLIB_Handle_t OSD)
{
    if(useProgressBar){
        fillrect(OSD,
                 (OSD->OSDWidth)/2-BAR_BORDER_X,
                 Y_BMP_COORD-getCharHeight()/4,
                 BAR_BORDER_X*OSDMain->OSDBS/2,
                 getCharHeight()+BAR_BORDER_Y/2,
                 BAR_BACKGROUND_CLUT);
        fillrect( OSD,
                 (OSD->OSDWidth)/2-BAR_BORDER_X+FILL_BAR_BORDER_X,
                 Y_BMP_COORD-getCharHeight()/4+FILL_BAR_BORDER_Y/2,
                 (BAR_BORDER_X-FILL_BAR_BORDER_X)*OSDMain->OSDBS/2,
                 getCharHeight()+BAR_BORDER_Y/2-FILL_BAR_BORDER_Y,
                 BAR_FOREGROUND_CLUT);
    }
}

/* ========================================================================
   Name:        osd_progress_bar_fill
   Description: fill the empty progress bar on the TV screen
   ======================================================================== */
static void osd_progress_bar_fill(I8* message, I32 percent)
{
	progress_bar_fill(OSDMain, message, percent);
    sh_flush_cache_all();
}

/* ========================================================================
   Name:        progress_bar_fill
   Description: fill the empty progress bar on the TV screen
   ======================================================================== */
static void progress_bar_fill(STOSDLIB_Handle_t OSD, I8* message, I32 percent)
{
	if(useProgressBar){
		progress_bar_init(OSD);
		if(percent > 255)
			percent = 255;
		fillrect( OSD,
				 (OSD->OSDWidth)/2-BAR_BORDER_X+FILL_BAR_BORDER_X,
				 Y_BMP_COORD-getCharHeight()/4+FILL_BAR_BORDER_Y/2,
				 (BAR_BORDER_X-FILL_BAR_BORDER_X)*percent/255*OSDMain->OSDBS/2,
				 getCharHeight()+BAR_BORDER_Y/2-FILL_BAR_BORDER_Y,
				 BAR_PROGRESS_CLUT);
		blitText(OSD, (U8*)message);
	}
}

/****************************************************************************/
/**********************   PIXMAP MANIPULATION  ******************************/
/****************************************************************************/
/* ========================================================================
   Name:        PatternFill
   Description: fill pattern

   ======================================================================== */
static void PatternFill(U32 *Pattern, U32 Clut,U32 Size,U32 Bs)
{
    U32 Loopx;
    U8  *Pat;
    if (Bs==3)
    {
        Pat = (U8*)(U32)Pattern;
        for(Loopx=0;Loopx<Size+7;Loopx+=3)
        {
            *Pat++ = Clut>> 0;
            *Pat++ = Clut>> 8;
            *Pat++ = Clut>>16;
        }
    }
    else
    {
        for(Loopx=0;Loopx<(Size+7)>>2;Loopx++)
        {
            *Pattern++ =Clut;
        }
    }
}

/* ========================================================================
   Name:        STOSDLIB_ClrScr
   Description: Clears a screen wih a specified color entry.
   ======================================================================== */
static void STOSDLIB_ClrScr ( STOSDLIB_Handle_t  Handle, U32 Clut )
{
    Clut = *(U32*) (Handle->OSDPrivateBase+(Clut<<2));
    U32 addr = Handle->OSDBase;
    U32 endaddr = addr + Handle->OSDWidth*(Handle->OSDHeight)*Handle->OSDBS;
    while(addr < endaddr){
        *(DU32 *)(addr) = Clut;
        addr += 4;
    }
}

/* ========================================================================
   Name:        fillrect
   Description: Fill a rectangle with the specified parameter
   ======================================================================== */
static void fillrect(STOSDLIB_Handle_t OSD, U32 XDO, U32 YDO, U32 width, U32 height, U32 Clut)
{
	U32 i,j;
	U32 addr = OSD->OSDBase;
	addr += (XDO*OSD->OSDBS + YDO*OSD->OSDWidth*OSD->OSDBS);

	Clut = *(U32*) (OSD->OSDPrivateBase+(Clut<<2));

	for(j=0;j<height;j++){
		for(i=0;i<width;i++){
			*(DU32 *)(addr) = Clut;
			addr += 4;
		}
		addr += OSD->OSDWidth*OSD->OSDBS - 4*width;
	}

}

/* ========================================================================
   Name:        STOSDLIB_CompPixmap
   Description: function allowing to display a compressed bitmap. Note that
                the syntax of the bitmap is completely specific to OSD lib.
                Syntax is explained in the corresponding documentation
                for info : we have :
                1. the header
                2. eventually the palette and/or the table of correspondancies
                   of the colors
                3. the compressed bitmap data, using a variable length
                   decoding, defined for the whole bitmap, or for each
                   each line
   ======================================================================== */
static I32  STOSDLIB_CompPixmap    ( STOSDLIB_Handle_t  Handle, U16 X, U32 Y, const U32 * Pixmap )
{
    U32             Width, Height, PelLength,Loop,first;
    U32             NPel, PaletteHeader, Corres_Header, NOfVld, VldUsed, PelValue,Table[256];
    U32             Sample,SampleEnd,Temp,BBB,BPos=0,Wb;
    const           U32 *Bitmap;

    /* clear the Table of correspondancies */
    Bitmap          = Pixmap; /* point on the address of bitmap */
    NPel            = ((*Bitmap)>>28)&0x000F;
    PaletteHeader   = ((*Bitmap)>>27)&0x0001;
    Width           = ((*Bitmap)>>16)&0x03FF;
    NOfVld          = ((*Bitmap)>>12)&0x000F;
    VldUsed         = ((*Bitmap)>>11)&0x0001;
    Corres_Header   = ((*Bitmap)>>10)&0x0001;
    Height          = ((*Bitmap)    )&0x03FF;
    Bitmap++;
    if (PaletteHeader)
    {
        for (Loop = 0; Loop < (0x1 << NPel); Loop++)
        {
            Table[Loop] = (Corres_Header==1? (U8)((*Bitmap)>>24) : Loop);
            STOSDLIB_SetClut       ( Handle, (U8)((*Bitmap)>>16), (U8)((*Bitmap)>>8), (U8)(*Bitmap), Table[Loop], 0xFF ); /* don't modify the mix */
            Bitmap++;
        }
    }
    else
    {
        if (Corres_Header)
        {
            for (Loop = 0; Loop < (0x1 << NPel); Loop+=4)
            {
                Table[Loop  ] = (U8)((*Bitmap)>>24);
                Table[Loop+1] = (U8)((*Bitmap)>>16);
                Table[Loop+2] = (U8)((*Bitmap)>>8 );
                Table[Loop+3] = (U8) (*Bitmap);
                Bitmap++;
            }
        }
        else
        {
            for (Loop = 0; Loop < (0x1 << NPel); Loop++)
            {
                Table[Loop] = Loop;
            }
        }
    }
    for (Sample=(U32)OXYBase(Handle,X,Y), Loop = 0; Loop < Height; Loop++, Sample+=Handle->OSDPitch-(Width*Handle->OSDBS))
    {    /* now, at this point, extract data and create the pixmap */
        PelLength = 1;
        if (VldUsed == 1) /* if there is new vld code for new line */
        {
            GetByte(4,NOfVld);
        }
        for (SampleEnd=Sample+(Width*Handle->OSDBS),first=1; Sample < SampleEnd ; )
        {
            GetByte(NPel, Temp);
            PelValue = Table[Temp];
            if (NOfVld)
            {
                GetByte(NOfVld,PelLength);
                PelLength++;
            }

            Wb       = (PelLength*Handle->OSDBS);
            if (PelValue != 255)
            {
                PelValue    = *(U32*) (Handle->OSDPrivateBase+(PelValue<<2));
                PatternFill ((U32*)&SamplePattern[0],PelValue,Wb,Handle->OSDBS);
                if ( Handle -> OSDFormat != OSD_YUV )
                {
                    memcpy      ( (void*)Sample, (void*)&SamplePattern[0], Wb );
                }
                else
                {
                    if ( first == 0 ) { SamplePattern[Sample&2] = (SamplePattern[Sample&2] + *(U8*) (Sample-4))>>1; }
                    memcpy      ( (void*)Sample, (void*) ( (U32)(&SamplePattern[0]) | (Sample&2) ) , Wb );
                }
            }
            first =0;
            Sample += Wb;
        }
    }   /* end of for Loop < = Height */
    return 0;
}

/* ========================================================================
   Name:        splash_text
   Description: Set the text on the progress bar

   ======================================================================== */
void splash_text(I8 *text)
{
    U32 i;

    for(i=0;(i<32)&&(*text);i++)
    {
        ProgressBar.msg[i] = *text;
        text++;
    }

    if(!(*text))
        ProgressBar.msg[i] = '\0';
    ProgressBar.msg[31] = '\0';     /* in case of ... */

    ProgressBar.value = 0;

    if(OSDMain != NULL)
        osd_progress_bar_fill(ProgressBar.msg, ProgressBar.value);
}

/* ========================================================================
   Name:        splash_text
   Description: Set the text on the progress bar

   ======================================================================== */
static void splash_version(void)
{
  U32 textSize;
  U8 *message = (U8 *)U_BOOT_VERSION;

  textSize = STOSDLIB_TextSize( message );
  STOSDLIB_TextXY ( OSDMain, (OSDMain->OSDWidth-textSize)>>1, Y_VER_COORD, message , VERSION_TEXT_CLUT, 0, 0 );
  sh_flush_cache_all();
}

/* ========================================================================
   Name:        splash_update
   Description: Set the progress on the progress bar

   ======================================================================== */
void splash_update (I32 value)
{
    ProgressBar.value = value;
    if(OSDMain != NULL)
        osd_progress_bar_fill(ProgressBar.msg, ProgressBar.value);
}

/* ========================================================================
   Name:        splash_update
   Description: Set the progress on the progress bar

   ======================================================================== */
void term_graphics (void)
{
    OSDMain = NULL;
}

/* ------------------------------- End of file ---------------------------- */
