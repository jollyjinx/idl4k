/*
 * (C) Copyright 2007, 2009 STMicroelectronics.
 *
 * Stuart Menefy <stuart.menefy@st.com>
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
#include <asm/soc.h>
#include <asm/stx7200reg.h>
#include <asm/io.h>
#include <asm/pio.h>
#include <asm/stbus.h>
#include <ata.h>

#define PIO_BASE  ST40_PIO0_REGS_BASE

static void stx7200_clocks(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	/*
	 * FIXME
	 * Gross hack to get the serial port working.
	 * See the defintion of PCLK in drivers/stm-asc.c
	 * for where this is used.
	 */
	bd->bi_emifrq = 100;
}

#ifdef CONFIG_DRIVER_NETSTMAC

#define MII_MODE		(1<<0)
#define PHY_CLK_EXT             (1<<2)
#define MAC_SPEED               (1<<4)
#define VCI_ACK_SOURCE          (1<<6)
#define RESET                   (1<<8)
#define DISABLE_MSG_READ        (1<<12)
#define DISABLE_MSG_WRITE       (1<<14)
/* Remaining bits define pad functions, default appears to work */

extern int stmac_default_pbl(void)
{
	return 32;
}

extern void stmac_set_mac_speed(int speed)
{
#if defined(CONFIG_STMAC_MAC0)
	const int mac = 0;    /* First MAC */
#elif defined(CONFIG_STMAC_MAC1)
	const int mac = 1;    /* Second MAC */
#endif
	unsigned long sysconf = *STX7200_SYSCONF_SYS_CFG41;

	if (speed == 100)
		sysconf |= (MAC_SPEED << mac);
	else if (speed == 10)
		sysconf &= ~(MAC_SPEED << mac);

	*STX7200_SYSCONF_SYS_CFG41 = sysconf;
}

/* ETH MAC pad configuration */
extern void stx7200_configure_ethernet(
	int mac, int rmii, int ext_clk, int phy_bus)
{
	unsigned long sysconf;

	sysconf = *STX7200_SYSCONF_SYS_CFG41;

	/* Route Ethernet pins to output */
	/* bit26-16: conf_pad_eth(10:0) */
	if (mac == 0) {
		/* MII0: conf_pad_eth(0) = 0 (ethernet) */
		sysconf &= ~(1<<16);
	} else {
		/* MII1: conf_pad_eth(2) = 0, (3)=0, (4)=0, (9)=0, (10)=0 (eth)
		 * MII1: conf_pad_eth(6) = 0 (MII1TXD[0] = output)
		 * (remaining bits have no effect in ethernet mode */
		sysconf &= ~( (1<<(16+2)) | (1<<(16+3)) | (1<<(16+4)) |
			      (1<<(16+5)) | (1<<(16+6)) | (1<<(16+7)) |
			      (1<<(16+8)) | (1<<(16+9)) | (1<<(16+10))  );
	}

	/* DISABLE_MSG_FOR_WRITE=0 */
	sysconf &= ~(DISABLE_MSG_WRITE << mac);

	/* DISABLE_MSG_FOR_READ=0 */
	sysconf &= ~(DISABLE_MSG_READ << mac);

	/* VCI_ACK_SOURCE = 0 */
        sysconf &= ~(VCI_ACK_SOURCE << mac);

	/* ETHERNET_INTERFACE_ON (aka RESET) = 1 */
        sysconf |= (RESET << mac);

	/* RMII_MODE */
	if (rmii)
		sysconf |= (MII_MODE << mac);
	else
		sysconf &= ~(MII_MODE << mac);

	/* PHY_CLK_EXT */
	if (ext_clk)
		sysconf |= (PHY_CLK_EXT << mac);
	else
		sysconf &= ~(PHY_CLK_EXT << mac);

	*STX7200_SYSCONF_SYS_CFG41 = sysconf;
}
#endif	/* CONFIG_DRIVER_NETSTMAC */

int soc_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	stx7200_clocks();

	bd->bi_devid = *STX7200_SYSCONF_DEVICEID_0;

	/*  Make sure reset period is shorter than WDT timeout */
	*STX7200_SYSCONF_SYS_CFG09 = (*STX7200_SYSCONF_SYS_CFG09 & 0xFF000000) | 0x000A8C;

	return 0;
}


#if defined(CONFIG_USB_OHCI_NEW)

#ifdef CONFIG_USB_STI7200_CUT1_SOFT_JTAG_RESET_WORKAROUND
/*
 * The following function *may* be required for boards
 * with cut 1.x of the STi7200 chip. This function must
 * *not* be used on cut 2.x (or later) of that chip.
 * There is a board modifiction comprising a R-C delay
 * which if applied negates the requirement to use this
 * workaround, so it is optional. However, it should be safe
 * to use this workaround with any cut 1.x silicon,
 * irrespective of the presence of the R-C delay board fix.
 *
 * NOTE: Register reads and USB_tdo variable:
 * All the reads are for USB_tdo, which is not used in this code.
 * They were commented-out debugging prints following each read,
 * so it's entirely possible that the reads could be dropped. But
 * they will affect timing and so might be significant. In the
 * absence of any understanding of how exactly this "black box"
 * code works, leave them in for safety.
 */
static void usb_soft_jtag_reset(void)
{
	int i, j;
	unsigned long USB_tdo;

	/* ENABLE SOFT JTAG */
	writel(0x00000040, STX7200_SYSCONF_SYS_CFG33);

	/* RELEASE TAP RESET */
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO IDLE STATE */
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO SHIFT IR STATE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SHIFT DATA IN TDI = 101 select TCB*/
	writel(0x00000046, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000047, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004E, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004F, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO IDLE MODE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO SHIFT DR STATE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SHIFT DATA IN TCB */
	for (i = 0; i <= 53; i++)
	{
		if ((i == 0) || (i == 1) || (i == 19) || (i == 36))
		{
			writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
			writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
		}
		if ((i == 53))
		{
			writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
			writel(0x0000004D, STX7200_SYSCONF_SYS_CFG33);
		}
		writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
		writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	}

	/* SET TAP INTO IDLE MODE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	for (i = 0; i <= 53; i++)
	{
		writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
		writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
		USB_tdo = readl(STX7200_SYSCONF_SYS_CFG00);
	}

	writel(0x00000040, STX7200_SYSCONF_SYS_CFG33);

	/* RELEASE TAP RESET */
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO IDLE STATE */
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO SHIFT IR STATE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SHIFT DATA IN TDI = 110 select TPR */
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000046, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000047, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004E, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004F, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO IDLE MODE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO SHIFT DR STATE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SHIFT DATA IN TDO */
	for (i = 0; i <= 366; i++)
	{
		writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
		writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
		USB_tdo = readl(STX7200_SYSCONF_SYS_CFG00);
	}

	for (j = 0; j < 2; j++)
	{
		for (i = 0; i <= 365; i++)
		{
			if ((i == 71) || (i == 192) || (i == 313))
			{
				writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
				writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
			}
			writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
			writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
			if ((i == 365))
			{
				writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
				writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
			}
		}
	}

	for (i = 0; i <= 366; i++)
	{
		writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
		writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
		USB_tdo = readl(STX7200_SYSCONF_SYS_CFG00);
	}

	/* SET TAP INTO IDLE MODE */
	writel(0x0000004C, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004D, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004C, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004D, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO SHIFT IR STATE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SHIFT DATA IN TDI = 101 select TCB */
	writel(0x00000046, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000047, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004E, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004F, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO IDLE MODE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO SHIFT DR STATE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SHIFT DATA IN TCB */
	for (i = 0; i <= 53; i++)
	{
		if ((i == 0) || (i == 1) || (i == 18) || (i == 19)
		    || (i == 36) || (i == 37))
		{
			writel(0x00000046, STX7200_SYSCONF_SYS_CFG33);
			writel(0x00000047, STX7200_SYSCONF_SYS_CFG33);
		}
		if ((i == 53))
		{
			writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
			writel(0x0000004D, STX7200_SYSCONF_SYS_CFG33);
		}
		writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
		writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	}

	/* SET TAP INTO IDLE MODE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	for (i = 0; i <= 53; i++)
	{
		writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
		writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
		USB_tdo = readl(STX7200_SYSCONF_SYS_CFG00);
	}

	/* SET TAP INTO SHIFT IR STATE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SHIFT DATA IN TDI = 110 select TPR */
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000046, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000047, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004E, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004F, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO IDLE MODE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* SET TAP INTO SHIFT DR STATE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	for (i = 0; i <= 366; i++)
	{
		writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
		writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);
		USB_tdo = readl(STX7200_SYSCONF_SYS_CFG00);
	}

	/* SET TAP INTO IDLE MODE */
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004c, STX7200_SYSCONF_SYS_CFG33);
	writel(0x0000004d, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000044, STX7200_SYSCONF_SYS_CFG33);
	writel(0x00000045, STX7200_SYSCONF_SYS_CFG33);

	/* 20ms delay */
	udelay(20000);

	/* ENABLE SOFT JTAG */
	writel(0x00000040, STX7200_SYSCONF_SYS_CFG33);
}
#endif	/* CONFIG_USB_STI7200_CUT1_SOFT_JTAG_RESET_WORKAROUND */

extern void stx7200_usb_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	const bd_t * const bd = gd->bd;
	unsigned long reg;
	const unsigned char power_pins[3] = {1, 3, 4};
	const unsigned char oc_pins[3] = {0, 2, 5};
#if CFG_USB_BASE == CFG_USB0_BASE
	const size_t port = 0;
#elif CFG_USB_BASE == CFG_USB1_BASE
	const size_t port = 1;
#elif CFG_USB_BASE == CFG_USB2_BASE
	const size_t port = 2;
#else
#error Unknown USB Host Controller Base Address
#endif

	/* ClockgenB powers up with all the frequency synths bypassed.
	 * Enable them all here.  Without this, USB 1.1 doesn't work,
	 * as it needs a 48MHz clock which is separate from the USB 2
	 * clock which is derived from the SATA clock. */
	writel(0, STX7200_CLOCKGENB_OUT_MUX_CFG);

	/* route USB and parts of MAFE instead of DVO.*/
	/* DVO output selection (probably ignored). */
	reg = readl(STX7200_SYSCONF_SYS_CFG07);
	reg &= ~(1ul<<26); /* conf_pad_pio[2] = 0 */
	reg &= ~(1ul<<27); /* conf_pad_pio[3] = 0 */
	writel(reg, STX7200_SYSCONF_SYS_CFG07);

	/* Enable soft JTAG mode for USB and SATA */
	reg = readl(STX7200_SYSCONF_SYS_CFG33);
	reg |= (1ul<<6);    /* soft_jtag_en = 1 */
	reg &= ~(0xful<<0); /* tck = tdi = trstn_usb = tms_usb = 0 */
	writel(reg, STX7200_SYSCONF_SYS_CFG33);

#ifdef CONFIG_USB_STI7200_CUT1_SOFT_JTAG_RESET_WORKAROUND
	/* reset USB HC via the JTAG scan path */
	usb_soft_jtag_reset();
#endif

	/* USB power */
	SET_PIO_PIN(PIO_PORT(7), power_pins[port], STPIO_ALT_OUT);
	STPIO_SET_PIN(PIO_PORT(7), power_pins[port], 1);

	/* USB Over-Current */
 	if (STX7200_DEVICEID_CUT(bd->bi_devid) < 2)
		SET_PIO_PIN(PIO_PORT(7), oc_pins[port], STPIO_ALT_BIDIR);
	else
		SET_PIO_PIN(PIO_PORT(7), oc_pins[port], STPIO_IN);

	/* tusb_powerdown_req[port] = 0 */
	reg = readl(STX7200_SYSCONF_SYS_CFG22);
	reg &= ~(1ul<<(port+3));
	writel(reg, STX7200_SYSCONF_SYS_CFG22);

	/* Set strap mode */
#define STRAP_MODE	AHB2STBUS_STRAP_16_BIT
	reg = readl(AHB2STBUS_STRAP);
#if STRAP_MODE == 0
	reg &= ~AHB2STBUS_STRAP_16_BIT;
#else
	reg |= STRAP_MODE;
#endif
	writel(reg, AHB2STBUS_STRAP);

	/* Start PLL */
	reg = readl(AHB2STBUS_STRAP);
	writel(reg | AHB2STBUS_STRAP_PLL, AHB2STBUS_STRAP);
	udelay(100000);	/* QQQ: can this delay be shorter ? */
	writel(reg & (~AHB2STBUS_STRAP_PLL), AHB2STBUS_STRAP);
	udelay(100000);	/* QQQ: can this delay be shorter ? */

	/* Set the STBus Opcode Config for 32-bit access */
	writel(AHB2STBUS_STBUS_OPC_32BIT, AHB2STBUS_STBUS_OPC);

	/* Set the Message Size Config to 4 packets per message */
	writel(AHB2STBUS_MSGSIZE_4, AHB2STBUS_MSGSIZE);

	/* Set the Chunk Size Config to 4 packets per chunk */
	writel(AHB2STBUS_CHUNKSIZE_4, AHB2STBUS_CHUNKSIZE);
}

#endif /* defined(CONFIG_USB_OHCI_NEW) */


#if defined(CONFIG_SH_STM_SATA)
extern void stx7200_configure_sata(void)
{
	static int initialised_phy = 0;

	if (!initialised_phy)
	{
		/* initialize the SATA PHY */
		stm_sata_miphy_init();

		/* configure the SATA host controller */
		stm_sata_probe();

		initialised_phy = 1;
	}
}
#endif	/* CONFIG_SH_STM_SATA */


