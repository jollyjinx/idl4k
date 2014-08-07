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
#include <asm/stx7108reg.h>
#include <asm/io.h>
#include <asm/pio.h>


	/*
	 * WARNING!  WARNING!  WARNING!  WARNING!  WARNING!
	 * ------------------------------------------------
	 *
	 * The ethernet PHYs may be reset by one of two mechanisms:
	 *
	 *	a) the EPLD on the MB705 peripheral board
	 *
	 *	b) via the PCF8575 (IC12) PIO Extender.
	 *
	 * This code does *not* (currently) make any attempt to
	 * reset the PHYs, but relies on the MB705 to reset the
	 * PHYs on power-on, which (thus far) demonstrably works well!
	 *
	 * If an MB705 is *not* connected, then additional work
	 * is required, including fitting J14-A, and J14-B.
	 * We currently just issue a diagnostic to warn about this ...
	 */
#if !defined(CONFIG_SH_MB705)
#warning Ethernet PHYs not reset without a MB705 present!
#endif


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


#define PIOALT(port, pin, alt, dir)			\
do							\
{							\
	stx7108_pioalt_select((port), (pin), (alt));	\
	stx7108_pioalt_pad((port), (pin), (dir));	\
} while(0)

static void configPIO(void)
{
	/* Setup PIOs for ASC device */

#if CFG_STM_ASC_BASE == ST40_ASC2_REGS_BASE

	/* Route UART2 via PIO14 for TX, RX, CTS & RTS (Alternative #1) */
	PIOALT(14, 4, 1, &stx7108_pioalt_pad_out);	/* UART2-TX */
	PIOALT(14, 5, 1, &stx7108_pioalt_pad_in);	/* UART2-RX */
	PIOALT(14, 6, 1, &stx7108_pioalt_pad_out);	/* UART2-RTS */
	PIOALT(14, 7, 1, &stx7108_pioalt_pad_in);	/* UART2-CTS */

#elif CFG_STM_ASC_BASE == ST40_ASC3_REGS_BASE

	/* Route UART3 via PIO21 for TX, RX, CTS & RTS (Alternative #2) */
	PIOALT(21, 0, 2, &stx7108_pioalt_pad_out);	/* UART3-TX */
	PIOALT(21, 1, 2, &stx7108_pioalt_pad_in);	/* UART3-RX */
	PIOALT(21, 3, 2, &stx7108_pioalt_pad_out);	/* UART3-RTS */
	PIOALT(21, 4, 2, &stx7108_pioalt_pad_in);	/* UART3-CTS */

#else
#error Unknown ASC port selected!
#endif	/* CFG_STM_ASC_BASE == ST40_ASCx_REGS_BASE */
}

extern int board_init(void)
{
	configPIO();

#ifdef QQQ	/* QQQ - DELETE */
#if defined(CONFIG_SH_STM_SATA)
	stx7105_configure_sata ();
#endif	/* CONFIG_SH_STM_SATA */
#endif		/* QQQ - DELETE */

#ifdef CONFIG_DRIVER_NET_STM_GMAC	
#if CFG_STM_STMAC_BASE == CFG_STM_STMAC0_BASE		/* MII0, on CN18 */
	stx7108_configure_ethernet(0, &(struct stx7108_ethernet_config) {
			.mode = stx7108_ethernet_mode_mii,
			.ext_clk = 1,
			.phy_bus = 0, });
#elif CFG_STM_STMAC_BASE == CFG_STM_STMAC1_BASE		/* MII1, on CN19 */
	stx7108_configure_ethernet(1, &(struct stx7108_ethernet_config) {
			.mode = stx7108_ethernet_mode_mii,
			.ext_clk = 1,
			.phy_bus = 1, });
#else
#error Unknown base address for the STM GMAC
#endif
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

#if defined(CONFIG_CMD_I2C)
	stx7108_configure_i2c();
#endif	/* CONFIG_CMD_I2C */

	/*
	 * For accessing the EPLD (on EMI Bank #2 (CSnC)), we need to
	 * elongate the rising CSn time, otherwise we risk the FET switches
	 * not being enabled for long enough.
	 * Failure to do this correctly, can result in the
	 * mb705_init_epld() function failing to pass the TEST.
	 *
	 * The following is (now) known to work:
	 *
	 *	set CSE2_TIME_WRITE = 0x1  [19:16] (rising-edge of CSn)
	 *
	 * Note with CSE2_TIME_WRITE=0x0, or CSE2_TIME_WRITE=0x2
	 * then, the EPLD *write* can fail, and this is
	 * (bizarrely) a function of the PC's offset in the I$!
	 *
	 * An alternative solution may be to enable the FET switches
	 * permanently, by setting J5A and J5B as follows:
	 * 	J5A: remove	(default is 2-3)
	 * 	J5B: 1-2	(default is 2-3)
	 */
#if 1
	*ST40_EMI_BANK2_EMICONFIGDATA2 &= ~(0xfu << 16);
	*ST40_EMI_BANK2_EMICONFIGDATA2 |=  (0x1u << 16);
#endif

	return 0;
}

int checkboard (void)
{
	printf ("\n\nBoard: STx7108-Mboard (MB837)"
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
