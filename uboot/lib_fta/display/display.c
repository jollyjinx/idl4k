/*****************************************************************************

File name   :  display.c

Description :  configure the video registers

 COPYRIGHT (C) FTA Communication technologies 2007.

Date               Modification                                          Name
----               ------------                                          ----
07/04/08           Created                                               FR

*****************************************************************************/
/* --- include ------------------------------------------------------------ */
#include "fta/display.h"
#include "data.h"
#include <asm/addrspace.h>


/* ========================================================================
   Name:        init_display
   Description: initialize display with default parameters

   ======================================================================== */
#if defined(CONFIG_SH_STX7105)

#include <asm/stx7105reg.h>

void init_display ( void )
{
    I32 loop;
    I32 temp;

/************* clocks for 576p and 576 i **************************************/
    *STX7105_CLOCKGENB_LOCK = 0xC0DE;           /* unblock clock gen registers    */
    *STX7105_CLOCKGENB_DISPLAY_CFG = 0x3051;    /* display configuration          */
    *STX7105_CLOCKGENB_FS_SELECT = 0x7;         /* clock relation ship            */
    *STX7105_HDMI_CFG = 0x80000000;
    udelay ( 300 );
    *STX7105_HDMI_CFG = 0x80000000;
    *STX7105_SYSCONF_SYS_CFG03 = 0x19644000;    /* disable PLL and keep reset     */
    udelay ( 300 );
    while (*STX7105_HDMI_STA != 0);             /* wait for stop                  */
    *STX7105_SYSCONF_SYS_CFG03 = 0x32645000;    /* enable rejection PLL           */
    udelay ( 300 );
    *STX7105_SYSCONF_SYS_CFG03 = 0x32645800;    /* release serializer             */
    *STX7105_HDMI_CFG = 0x00000001;             /* enable HDMi interface          */
/******************************************************************************/
    for (loop=0;loop<sizeof(MAINVTG) / 8 ;loop++)
    {   /* program VTG A */
        temp = *(I32*) (ST7105_HD_TVOUT_VTG_MAIN  + MAINVTG[loop].address);
        *(I32*) (ST7105_HD_TVOUT_VTG_MAIN  + MAINVTG[loop].address) = MAINVTG[loop].data;
        MAINVTG[loop].data = temp;
    }
/******************************************************************************/
    for (loop=0;loop<sizeof(AUXVTG) / 8 ;loop++)
    {   /* program VTG B */
        temp = *(I32*) (ST7105_HD_TVOUT_VTG_AUX  + AUXVTG [loop].address);
        *(I32*) (ST7105_HD_TVOUT_VTG_AUX  + AUXVTG [loop].address) = AUXVTG  [loop].data;
        AUXVTG  [loop].data = temp;
    }
/******************************************************************************/
    for (loop=0;loop<sizeof(DENCReg) / 8 ;loop++)
    {   /* program DENC registers   */
        temp = *(I32*) (ST7105_HD_TVOUT_DENC + DENCReg[loop].address);
        *(I32*) (ST7105_HD_TVOUT_DENC + DENCReg[loop].address) = DENCReg[loop].data;
        DENCReg[loop].data = temp;
    }
/************************************ Video GDP 1 + 3 *************************/
    memset ( (void*)RAM_DISPLAY_NODE_ADDRESS, 0 , 0x3000 );
    for (loop=0;loop<sizeof(GDP1Reg)/8 ;loop++)
    {
        *(I32*) ( RAM_DISPLAY_NODE_ADDRESS + 0x0000 + GDP1Reg [loop].address) = GDP1Reg [loop].data;
    }
    for (loop=0;loop<sizeof(GDP2Reg)/8 ;loop++)
    {
        *(I32*) ( RAM_DISPLAY_NODE_ADDRESS + 0x1000 + GDP2Reg [loop].address) = GDP2Reg [loop].data;
        *(I32*) ( RAM_DISPLAY_NODE_ADDRESS + 0x2000 + GDP2Reg [loop].address) = GDP2Reg [loop].data;
    }
    *(I32*) ( RAM_DISPLAY_NODE_ADDRESS + 0x0014 ) = PHYSADDR(RAM_DISPLAY_ADDRESS);               /* pixmap location */
    *(I32*) ( RAM_DISPLAY_NODE_ADDRESS + 0x0024 ) = PHYSADDR(RAM_DISPLAY_NODE_ADDRESS + 0x0000); /* next node location */
    *(I32*) ( RAM_DISPLAY_NODE_ADDRESS + 0x1014 ) = PHYSADDR(RAM_DISPLAY_ADDRESS);               /* pixmap location  top  */
    *(I32*) ( RAM_DISPLAY_NODE_ADDRESS + 0x1024 ) = PHYSADDR(RAM_DISPLAY_NODE_ADDRESS + 0x2000); /* next node location */
    *(I32*) ( RAM_DISPLAY_NODE_ADDRESS + 0x2014 ) = PHYSADDR(RAM_DISPLAY_ADDRESS + 720*4);       /* pixmap location  bot  */
    *(I32*) ( RAM_DISPLAY_NODE_ADDRESS + 0x2024 ) = PHYSADDR(RAM_DISPLAY_NODE_ADDRESS + 0x1000); /* next node location */

    *(I32*) ( ST7105_COMPOSITOR_GDP1 + 0xfc ) = 0;
    *(I32*) ( ST7105_COMPOSITOR_GDP3 + 0xfc ) = 0;
    *(I32*) ( ST7105_COMPOSITOR_GDP1 + 0x24 ) = PHYSADDR(RAM_DISPLAY_NODE_ADDRESS + 0x0000);
    *(I32*) ( ST7105_COMPOSITOR_GDP3 + 0x24 ) = PHYSADDR(RAM_DISPLAY_NODE_ADDRESS + 0x1000);

    sh_flush_cache_all();

/******************************************************************************/
    for (loop=0;loop<sizeof(MIX1Reg) / 8 ;loop++)
    {   /* MIXER 1 : 576 p for HDMI     */
        temp = *(I32*) (ST7105_COMPOSITOR_MX1 + MIX1Reg[loop].address);
        *(I32*) (ST7105_COMPOSITOR_MX1 + MIX1Reg[loop].address) = MIX1Reg[loop].data;
        MIX1Reg[loop].data = temp;
    }
/******************************************************************************/
    for (loop=0;loop<sizeof(MIX2Reg) / 8 ;loop++)
    {   /* MIXER 2   576 i  PAL         */
        temp = *(I32*) (ST7105_COMPOSITOR_MX2 + MIX2Reg[loop].address);
        *(I32*) (ST7105_COMPOSITOR_MX2 + MIX2Reg[loop].address) = MIX2Reg[loop].data;
        MIX2Reg[loop].data = temp;
    }

    /***************  install HDMI handler + interrupt*************  */
    for (loop=0;loop<sizeof(HDFormatter)/8 ;loop++)
    {
        temp = *(I32*) ( ST7105_HD_TVOUT_HDF1 + HDFormatter [loop].address);
        *(I32*) ( ST7105_HD_TVOUT_HDF1 + HDFormatter [loop].address) = HDFormatter [loop].data;
        HDFormatter [loop].data = temp;
    }
    for (loop=0;loop<sizeof(HDGlue)/8 ;loop++)
    {
        temp = *(I32*) ( ST7105_HD_TVOUT_TOP_MAIN_GLUE + HDGlue [loop].address);
        *(I32*) ( ST7105_HD_TVOUT_TOP_MAIN_GLUE + HDGlue [loop].address) = HDGlue [loop].data;
        HDGlue [loop].data = temp;
    }
    for (loop=0;loop<sizeof(HDMIReg)/8 ;loop++)
    {
        temp = *(I32*) ( ST7105_HDMI_REGS_BASE + HDMIReg [loop].address);
        *(I32*) ( ST7105_HDMI_REGS_BASE + HDMIReg [loop].address) = HDMIReg [loop].data;
        HDMIReg [loop].data = temp;
    }
}

void test_display(void)
{
    *(I32*) ( ST7105_HD_TVOUT_DENC + 0x00 ) = 0x38;
    *(I32*) ( ST7105_HD_TVOUT_DENC + 0x18 ) = 0x80;
}

I32 term_display(void)
{
    /* Remove access to the display */
    term_graphics ();

    init_display();
    *(DI32*) (ST7105_HD_TVOUT_VTG_AUX  + 0x9C ) = 0x18;  /* you must acknowledge the interrupt */

    /* clear the video Node */
    memset((void*)RAM_DISPLAY_NODE_ADDRESS, 0, 0x100000);

    return 0;
}
#endif

/* ------------------------------- End of file ---------------------------- */


