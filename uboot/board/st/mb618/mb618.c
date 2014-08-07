/*
 * (C) Copyright 2008 STMicroelectronics.
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
#include <asm/stx7111reg.h>
#include <asm/io.h>
#include <asm/pio.h>

#define PIO_BASE  0xfd020000	/* Base of PIO block in COMMs block */

#ifndef CONFIG_SH_NO_EPLD
#ifdef CONFIG_SH_SE_MODE
#define EPLD_BASE		0xb6000000	/* Phys 0x06000000 */
#else
#define EPLD_BASE		0xa6000000
#endif	/* CONFIG_SH_SE_MODE */

#define EPLD_IDENT		0x00	/* READ: EPLD Identifier Register */
#define EPLD_BANK		0x00	/* WRITE: EPLD Bank Register */
#define EPLD_TEST		0x04	/* EPLD Test Register (Banked) */
#define EPLD_CTRL		0x04	/* EPLD Control Register (Banked) */
#define EPLD_SET_BANK_TEST	0x00	/* Bank = EPLD_TEST */
#define EPLD_SET_BANK_CTRL	0x01	/* Bank = EPLD_CTRL */

static inline void epld_write(unsigned long value, unsigned long offset)
{
	/* 8-bit write to EPLD registers */
	writeb(value, EPLD_BASE + offset);
}

static inline unsigned long epld_read(unsigned long offset)
{
	/* 8-bit read from EPLD registers */
	return readb(EPLD_BASE + offset);
}
#endif	/* CONFIG_SH_NO_EPLD */

void flashWriteEnable(void)
{
	/* Enable Vpp for writing to flash */
	STPIO_SET_PIN(PIO_PORT(3), 4, 1);
}

void flashWriteDisable(void)
{
	/* Disable Vpp for writing to flash */
	STPIO_SET_PIN(PIO_PORT(3), 4, 0);
}

static void configPIO(void)
{
	/* Setup PIO of ASC device */
	SET_PIO_ASC(PIO_PORT(4), 3, 2, 4, 5);  /* UART2 - AS0 */
	SET_PIO_ASC(PIO_PORT(5), 0, 1, 2, 3);  /* UART3 - AS1 */

	/* Setup up Vpp for NOR FLASH */
	SET_PIO_PIN(PIO_PORT(3), 4, STPIO_OUT);
}

extern int board_init(void)
{
	configPIO();

	return 0;
}

#if defined(CONFIG_DRIVER_NET_STM_GMAC) && !defined(CONFIG_SH_NO_EPLD)
/*
 * Reset the Ethernet PHY, via the EPLD.
 * This code is only for EPLD version 06 or later.
 */
static inline void mb618_phy_reset06(void)
{
	/* set EPLD Bank = Ctrl */
	epld_write(EPLD_SET_BANK_CTRL, EPLD_BANK);

	/* Bring the PHY out of reset in MII mode */
	epld_write(0x4 | 0, EPLD_CTRL);
	epld_write(0x4 | 1, EPLD_CTRL);
}
#endif	/* defined(CONFIG_DRIVER_NET_STM_GMAC) && !defined(CONFIG_SH_NO_EPLD) */

/*
 * We have several EPLD versions, with slightly different memory
 * maps and features:
 *
 * version 04:
 * off  read        reset
 *  0   Status      undef  (unused)
 *  4   Ctrl        20     (unused)
 *  8   Test        33
 *  c   Ident       0      (should be 1 but broken)
 * (note writes are broken)
 *
 * version 05:
 * off  read     write       reset
 *  0   Ident    Ctrl        45
 *  4   Test     Test        55
 *  8   IntStat  IntMaskSet  -
 *  c   IntMask  IntMaskClr  0
 *
 * version 06:
 * off        read       write         reset
 *  0         Ident      Bank          46 (Bank register defaults to 0)
 *  4 bank=0  Test       Test          55
 *  4 bank=1  Ctrl       Ctrl          0e
 *  4 bank=2  IntPri0    IntPri0  f9
 *  4 bank=3  IntPri1    IntPri1  f0
 *  8         IntStat    IntMaskSet    -
 *  c         IntMask    IntMaskClr    00
 *
 * Ctrl register bits:
 *  0 = Ethernet Phy notReset
 *  1 = RMIInotMIISelect
 *  2 = Mode Select_7111 (ModeSelect when D0 == 1)
 *  3 = Mode Select_8700 (ModeSelect when D0 == 0)
 *
 *  The version 06 map is also applicable to later versions.
 *
 *  NOTE: U-Boot only supports version 06 or later of the EPLD.
 *  Versions of the EPLD prior to this version are NOT supported!
 */

static int mb618_init_epld(void)
{
#ifdef CONFIG_SH_NO_EPLD
	/* we ignore talking to the EPLD, tell the user */
	printf("info: Disregarding any EPLD\n");

#else	/* CONFIG_SH_NO_EPLD */
	const unsigned char test_values[2] = {0xa4u, 0x2fu};
	unsigned char epld_reg, inverted;
	unsigned char epld_version, board_version;
	int i;

	/* set EPLD Bank = Test */
	epld_write(EPLD_SET_BANK_TEST, EPLD_BANK);

	/* for each test value ... */
	for (i=0; i<sizeof(test_values)/sizeof(test_values[0]); i++) {
		/* write (anything) to the test register */
		epld_write(test_values[i], EPLD_TEST);
		/* calculate what we expect back */
		inverted = ~test_values[i];
		/* now read it back */
		epld_reg = epld_read(EPLD_TEST);
		/* verify we got back an inverted result */
		if (epld_reg != inverted) {
			printf("Failed EPLD test (offset=%02x, %02x!=%02x)\n",
				EPLD_TEST, epld_reg, inverted);
			return 1;	/* Failure! */
			}
	}

	/* Assume we can trust the ident register */
	epld_reg      = epld_read(EPLD_IDENT);
	board_version = (epld_reg >> 5) & 0x07u;
	epld_version  = (epld_reg >> 0) & 0x1fu;

	/* is it acceptable ? */
	if (epld_version < 6) {
		printf("Unsupported EPLD version (reg=0x%02x)\n",
			epld_reg);
		return 1;	/* Failure! */
	}

	/* display the board revision, and EPLD version */
	printf("MB618: revision %c, EPLD version %02d\n",
		board_version + 'A',
		epld_version);

	/* now perform the EPLD initializations we want */
#ifdef CONFIG_DRIVER_NET_STM_GMAC
	mb618_phy_reset06();
#endif

	/* set the Test register back to RESET conditions (for linux) */
	/* set EPLD Bank = Test */
	epld_write(EPLD_SET_BANK_TEST, EPLD_BANK);
	/* write inverted 0x55, so it reads back as 0x55 */
	epld_write(~0x55u, EPLD_TEST);

#endif	/* CONFIG_SH_NO_EPLD */

	/* return a "success" result */
	return 0;
}

int checkboard (void)
{
	printf ("\n\nBoard: STx7111-Mboard (MB618)"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

	/*
	 * initialize the EPLD.
	 */
	return mb618_init_epld();
}
