/*
 * (C) Copyright 2009 STMicroelectronics.
 *
 * Sean McGoogan <Sean.McGoogan@st.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/stx5197reg.h>
#include <asm/io.h>
#include <asm/pio.h>


#define PIO_BASE  0xfd120000	/* Base of PIO block in COMMs block */


	/* Alternate Function Output Selection accessors */
#define ALT_SELn(n,alt)		( (((alt)>>(n))&1)<<(8*(n)) )
#define ALT_SEL(alt)		( ALT_SELn(1,(alt)) | ALT_SELn(0,(alt)) )
#define ALT_MASK(port,pin,alt)	( ALT_SEL(alt) << ((((port)&1)?16:0)+((pin)&7)) )
#define ALTFOP(reg,port,pin,alt)			\
	do {						\
		reg &= ~ALT_MASK((port),(pin), 0x3);	\
		reg |=  ALT_MASK((port),(pin),(alt));	\
	} while(0)


extern void flashWriteEnable (void)
{
	/* Enable Vpp for writing to flash */
}

extern void flashWriteDisable (void)
{
	/* Disable Vpp for writing to flash */
}


#ifdef CONFIG_STM_ASC_SERIAL
static void configSerial (void)
{
	unsigned long sysconf;

#if (CFG_STM_ASC_BASE == CFG_STM_ASC2_BASE)
	/* Setup PIO of ASC device */
	SET_PIO_ASC(PIO_PORT(1), 2, 3, 5, 4);  /* UART2 - AS0 */
	/* Route UART2 via PIO1 for TX, RX, CTS & RTS */
	sysconf = *STX5197_HD_CONF_MON_CONFIG_CONTROL_F;
	ALTFOP(sysconf,1,2,1);		/* PIO1[2] AltFunction = 1 */
	ALTFOP(sysconf,1,3,1);		/* PIO1[3] AltFunction = 1 */
	ALTFOP(sysconf,1,4,1);		/* PIO1[4] AltFunction = 1 */
	ALTFOP(sysconf,1,5,1);		/* PIO1[5] AltFunction = 1 */
	*STX5197_HD_CONF_MON_CONFIG_CONTROL_F = sysconf;
#elif (CFG_STM_ASC_BASE == CFG_STM_ASC3_BASE)
	/* Setup PIO of ASC device */
	SET_PIO_ASC(PIO_PORT(2), 0, 1, 2, 5);  /* UART3 - AS1 */
	/* Route UART3 via PIO2 for TX, RX, CTS & RTS */
	sysconf = *STX5197_HD_CONF_MON_CONFIG_CONTROL_G;
	ALTFOP(sysconf,2,0,1);		/* PIO2[0] AltFunction = 1 */
	ALTFOP(sysconf,2,1,1);		/* PIO2[1] AltFunction = 1 */
	ALTFOP(sysconf,2,2,1);		/* PIO2[2] AltFunction = 1 */
	ALTFOP(sysconf,2,5,1);		/* PIO2[5] AltFunction = 1 */
	*STX5197_HD_CONF_MON_CONFIG_CONTROL_G = sysconf;
#else
#error Unknown serial port configuration!
#endif
}
#endif /* CONFIG_STM_ASC_SERIAL */


extern int board_init (void)
{
#ifdef CONFIG_STM_ASC_SERIAL
	configSerial ();
#endif /* CONFIG_STM_ASC_SERIAL */

	return 0;
}


extern int checkboard (void)
{
	printf ("\n\nBoard: STx5197-Mboard (MB704)"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

	return 0;
}
