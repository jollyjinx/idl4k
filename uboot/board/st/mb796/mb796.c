/*
 * (C) Copyright 2008-2010 STMicroelectronics.
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
#include <asm/soc.h>
#include <asm/stx5206reg.h>
#include <asm/io.h>
#include <asm/pio.h>


#if defined(CONFIG_SH_MB705)
	/*
	 * More recent EPLD versions have the EPLD in EMI space,
	 * using CSCn (EMI Bank #2), nominally at physical 0x04800000.
	 */
#if !defined(CFG_EPLD_PHYSICAL_BASE)
#	define CFG_EPLD_PHYSICAL_BASE	0x04800000	/* CSCn (EMI Bank #2) */
#endif /* CFG_EPLD_PHYSICAL_BASE */
	/* map the physical address to UN-cached virtual address */
#if !defined(CFG_EPLD_BASE)
#	define CFG_EPLD_BASE		( 0xa0000000 | (CFG_EPLD_PHYSICAL_BASE) )
#endif /* CFG_EPLD_BASE */
	/*
	 * following are the offsets within the EMI EPLD (IC21),
	 * for the MB705 Peripheral board.
	 */
#define EPLD_IDENT		0x00	/* EPLD Identifier Register */
#define EPLD_TEST		0x02	/* EPLD Test Register */
#define EPLD_SWITCH		0x04	/* EPLD Switch Register */
#define EPLD_MISC		0x0a	/* Miscellaneous Control Register */
#endif	/* CONFIG_SH_MB705 */


#if defined(CONFIG_SH_MB705)
static inline void epld_write(unsigned long value, unsigned long offset)
{
	/* 16-bit write to EPLD registers */
	writew(value, CFG_EPLD_BASE + offset);
}

static inline unsigned long epld_read(unsigned long offset)
{
	/* 16-bit read from EPLD registers */
	return readw(CFG_EPLD_BASE + offset);
}

static int mb705_init_epld(void)
{
	const unsigned short test_value = 0x1234u;
	unsigned short epld_reg;
	unsigned short epld_version, board_version;

	/* write (anything) to the test register */
	epld_write(test_value, EPLD_TEST);
	/* verify we got back an inverted result */
	epld_reg = epld_read(EPLD_TEST);
	if (epld_reg != (test_value ^ 0xffffu)) {
		printf("Failed EPLD test (offset=%02x, result=%04x)\n",
			EPLD_TEST, epld_reg);
		return 1;
		}

	/* Assume we can trust the version register */
	epld_reg = epld_read(EPLD_IDENT);
	board_version = epld_reg >> 4 & 0xfu;
	epld_version = epld_reg & 0xfu;

	/* display the board revision, and EPLD version */
	printf("MB705: revision %c, EPLD version %02d\n",
		board_version + 'A' - 1,
		epld_version);

	/* return a "success" result */
	return 0;
}
#endif	/* CONFIG_SH_MB705 */

void flashWriteEnable(void)
{
#if defined(CONFIG_SH_MB705)
	unsigned short epld_reg;

	/* Enable Vpp for writing to flash */
	epld_reg = epld_read(EPLD_MISC);
	epld_reg |= 1u << 3;	/* NandFlashWP = MISC[3] = 1 */
	epld_reg |= 1u << 2;	/* NorFlashVpp = MISC[2] = 1 */
	epld_write(epld_reg, EPLD_MISC);
#endif	/* CONFIG_SH_MB705 */
}

void flashWriteDisable(void)
{
#if defined(CONFIG_SH_MB705)
	unsigned short epld_reg;

	/* Disable Vpp for writing to flash */
	epld_reg = epld_read(EPLD_MISC);
	epld_reg &= ~(1u << 3);	/* NandFlashWP = MISC[3] = 0 */
	epld_reg &= ~(1u << 2);	/* NorFlashVpp = MISC[2] = 0 */
	epld_write(epld_reg, EPLD_MISC);
#endif	/* CONFIG_SH_MB705 */
}


#ifdef CONFIG_DRIVER_NET_STM_GMAC
extern void stmac_phy_reset(void)
{
	/*
	 * Reset the Ethernet PHY.
	 *
	 *	PIO0[6] = notPioResetMii
	 *
	 * The following works, with the jumpers connected as:
	 * 	J18-G:1-2, J22-B:1-2
	 */
	STPIO_SET_PIN(PIO_PORT(0), 6, 0);
	udelay(15000);				/* 15 ms */
	STPIO_SET_PIN(PIO_PORT(0), 6, 1);
}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

static void configPIO(void)
{
	unsigned long sysconf;

	/* Route the GPIO block to the PIOx pads, as by
	 * default the SATFE owns them... (why, oh why???) */
	sysconf = *STX5206_SYSCONF_SYS_CFG10;
	sysconf |= 1ul << 13;
	*STX5206_SYSCONF_SYS_CFG10 = sysconf;

#if CFG_STM_ASC_BASE == ST40_ASC2_REGS_BASE	/* UART2 = AS0 */

	/* Setup PIO of ASC device */
	SET_PIO_ASC(PIO_PORT(1), 2, 1, 4, 3);  /* UART2 - AS0 */

	/* Route UART2 via PIO1 for TX, RX, CTS & RTS */
	sysconf = *STX5206_SYSCONF_SYS_CFG16;
	/* PIO1[1] CFG16[9,1]   AltFunction = 1 */
	/* PIO1[2] CFG16[10,2]  AltFunction = 1 */
	/* PIO1[3] CFG16[11,3]  AltFunction = 1 */
	/* PIO1[4] CFG16[12,4]  AltFunction = 1 */
	sysconf &= ~(0x0f0ful<<1);	/* 3,3,3,3 */
	sysconf |=  (0x0000ul<<1);	/* 0,0,0,0 */
	*STX5206_SYSCONF_SYS_CFG16 = sysconf;

#elif CFG_STM_ASC_BASE == ST40_ASC3_REGS_BASE	/* UART3 = AS1 */

	/* Setup PIO of ASC device */
	SET_PIO_ASC(PIO_PORT(2), 4, 3, 5, 6);  /* UART3 - AS1 */

	/* Route UART3 via PIO2 for TX, RX, CTS & RTS */
	sysconf = *STX5206_SYSCONF_SYS_CFG16;
	/* PIO2[3] CFG16[16+11,16+3]  AltFunction = 1 */
	/* PIO2[4] CFG16[16+12,16+4]  AltFunction = 1 */
	/* PIO2[5] CFG16[16+13,16+5]  AltFunction = 1 */
	/* PIO2[6] CFG16[16+14,16+6]  AltFunction = 1 */
	sysconf &= ~(0x0f0ful<<(16+3));	/* 3,3,3,3 */
	sysconf |=  (0x0000ul<<(16+3));	/* 0,0,0,0 */
	*STX5206_SYSCONF_SYS_CFG16 = sysconf;

#else
#error Unknown ASC port selected!
#endif	/* CFG_STM_ASC_BASE */

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	/*
	 * Configure the Ethernet PHY Reset signal
	 *	PIO0[6] = notPioResetMii
	 */
	SET_PIO_PIN(PIO_PORT(0), 6, STPIO_OUT);
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */
}

extern int board_init(void)
{
	configPIO();

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	/* Reset the PHY */
	stmac_phy_reset();

	/*
	 * The following works, with the jumpers connected as:
	 * 	J38:2-3, J39-A:remove, J39-B:2-3
	 */
	stx5206_configure_ethernet(stx5206_ethernet_rmii, 0, 0);
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

	return 0;
}

int checkboard (void)
{
	printf ("\n\nBoard: STx5289-Mboard (MB796)"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

#if defined(CONFIG_SH_MB705)
	/*
	 * initialize the EPLD on the MB705.
	 */
	mb705_init_epld();
#endif	/* CONFIG_SH_MB705 */

	return 0;
}
