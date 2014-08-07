/*****************************************************************************

File name   :  display.h

Description :  define the STI7100 video addresses and interrupt

 COPYRIGHT (C) FTA Communication techniologies 2007.

Date               Modification                                          Name
----               ------------                                          ----
07/04/08           Created                                               FR

*****************************************************************************/
/* --- prevents recursive inclusion --------------------------------------- */
#ifndef DISPLAY_H__
#define DISPLAY_H__

/* --- include ------------------------------------------------------------ */
#include "fta/definition.h"

/* --- defines ------------------------------------------------------------ */

#if defined(CONFIG_SH_STX7105)
#include <asm/stx7105reg.h>

#ifndef ST7105_HDMI_REGS_BASE
#define ST7105_HDMI_REGS_BASE			0xfd104000
#endif
#ifndef ST7105_HD_TVOUT_REGS_BASE
#define ST7105_HD_TVOUT_REGS_BASE		0xfe030000
#endif
#ifndef ST7105_COMPOSITOR_REGS_BASE
#define ST7105_COMPOSITOR_REGS_BASE		0xfe20a000
#endif

/* Video registers */
#define STX7105_HDMI_CFG				SH4_DWORD_REG(ST7105_HDMI_REGS_BASE + 0x00)
#define STX7105_HDMI_STA				SH4_DWORD_REG(ST7105_HDMI_REGS_BASE + 0x24)

#define ST7105_HD_TVOUT_DENC			(ST7105_HD_TVOUT_REGS_BASE + 0x000)
#define ST7105_HD_TVOUT_VTG_AUX			(ST7105_HD_TVOUT_REGS_BASE + 0x200)
#define ST7105_HD_TVOUT_VTG_MAIN		(ST7105_HD_TVOUT_REGS_BASE + 0x300)
#define ST7105_HD_TVOUT_TOP_MAIN_GLUE	(ST7105_HD_TVOUT_REGS_BASE + 0x400)
#define ST7105_HD_TVOUT_HDF1			(ST7105_HD_TVOUT_REGS_BASE + 0x800)

#define ST7105_COMPOSITOR_GDP1			(ST7105_COMPOSITOR_REGS_BASE + 0x100)
#define ST7105_COMPOSITOR_GDP2			(ST7105_COMPOSITOR_REGS_BASE + 0x200)
#define ST7105_COMPOSITOR_GDP3			(ST7105_COMPOSITOR_REGS_BASE + 0x300)
#define ST7105_COMPOSITOR_MX1			(ST7105_COMPOSITOR_REGS_BASE + 0xc00)
#define ST7105_COMPOSITOR_MX2			(ST7105_COMPOSITOR_REGS_BASE + 0xd00)

#endif

#endif	/* DISPLAY_H__ */
