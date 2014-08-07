/*
 * STM SATA initialization
 *
 * Copyright (C) 2007,2009-2010 STMicroelectronics Limited
 * Author: Stuart Menefy <stuart.menefy@st.com>
 * Sean McGoogan <Sean.McGoogan@st.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * This file is based on "arch/sh/kernel/cpu/sh4/sata-init.c"
 * in STMicroelectronics' release "stm23_0119" of the linux kernel.
 */

#include <common.h>


#if defined(CONFIG_SH_STM_SATA)


#include <asm/errno.h>
#include <asm/st40reg.h>
#include <asm/io.h>
#include <asm/soc.h>
#include <asm/socregs.h>


#if defined(CONFIG_SH_STX7105)		/* Cut 3.x (or later) */
#define SYS_STA00	STX7105_SYSCONF_SYS_STA00
#define SYS_CFG33	STX7105_SYSCONF_SYS_CFG33
#define soft_jtag_en	(1<<6)
#elif defined(CONFIG_SH_STX7141)	/* Cut 2.x (or later) */
#define SYS_STA00	STX7141_SYSCONF_SYS_STA00
#define SYS_CFG33	STX7141_SYSCONF_SYS_CFG33
#define soft_jtag_en	(0<<6)		/* STx7141 has this bit inverted */
#elif defined(CONFIG_SH_STX7200)	/* Cut 3.x (or later) */
#define SYS_STA00	STX7200_SYSCONF_SYS_STA00
#define SYS_CFG33	STX7200_SYSCONF_SYS_CFG33
#define soft_jtag_en	(1<<6)
#else
#	error Missing Device Definitions!
#endif


/* sysconf status 0 */
#define sata_tdo	(1<<1)

/* sysconf config 33 */
#define tms_sata_en	(1<<5)
#define trstn_sata	(1<<4)
#define tdi_high	(1<<1)
#define tdi_low		(0<<1)
#define tck_high	(1<<0)
#define tck_low		(0<<0)


/*
 * to minimize (future) maintenance with the linux sources,
 * we "fake" the sysconf_field struct.
 */
typedef int sysconf_field;

/* when reading, only interested in SYS_STA00[2:0] */
#define sysconf_read(sc)	((readl((sc))) & 0x07)

/* when writing, only interested in SYS_CFG33[6:0] */
#define sysconf_write(sc, val)	\
	do { writel((((readl(sc))&(~0x7F))|(val)),(sc)); } while (0)


static void SATA_JTAG_IR_Select_MIPHY(
	const sysconf_field sc)
{
	/* Flush trst synchronizer with two tck clocks */
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);

	/* Set tap into idle state */
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);

	/* Set tap into shift ir state */
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);

	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);

	/* Shift in MacroMicroBusAccess TDI = 101 */
	sysconf_write(sc, soft_jtag_en | trstn_sata | tdi_high | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tdi_high | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tdi_low | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tdi_low | tck_high);

	/* Set tap into idle mode */
	sysconf_write(sc,
		      soft_jtag_en | tms_sata_en | trstn_sata | tdi_high |
		      tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tdi_high | tck_high);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
}

static void SATA_JTAG_DR_Write_MIPHY(
	const sysconf_field sc,
	const u8 regno,
	const u8 data)
{
	int k;
	u8 x;

	/* Set TAP into shift dr state */
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);

	/* Shift in DR=[17:10]=data;[9:2]address=regno;[1]rd=0;[0]wr=1  */
	sysconf_write(sc, soft_jtag_en | trstn_sata | tdi_high | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tdi_high | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);

	/* Push in the register address */
	for (k = 0; k < 8; k++) {
		x = ((regno >> k) & 0x1);
		x = x << 1;
		sysconf_write(sc, soft_jtag_en | trstn_sata | x | tck_low);
		sysconf_write(sc, soft_jtag_en | trstn_sata | x | tck_high);
	}

	/* Push in the data to be written */
	x = 0;
	for (k = 0; k < 7; k++) {
		x = ((data >> k) & 0x1);
		x = x << 1;
		sysconf_write(sc, soft_jtag_en | trstn_sata | x | tck_low);
		sysconf_write(sc, soft_jtag_en | trstn_sata | x | tck_high);
	}

	/* Set TAP back round into SHIFT DR STATE
	 * (exit1_dr,update_dr,idle,idle,select_dr,capture_dr) */
	x = ((data >> 7) & 0x1);
	x = x << 1;
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | x | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | x | tck_high);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);

	/*Set TAP into idle mode */
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
}

static u32 SATA_JTAG_DR_Read_MIPHY(
	const sysconf_field sc,
	const sysconf_field status_sc,
	const u8 regno)
{
	int k;
	u8 x;
	u32 ctrlbit, regvalue = 0;

	/* Set TAP into shift DR state */
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);

	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);

	/* Shift in DR=[17:10]dummy_data;[9:2]address=regno;[1]rd=1;[0]wr=0  */
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tdi_high | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tdi_high | tck_high);

	/*  push in the register address */
	for (k = 0; k < 8; k++) {
		x = ((regno >> k) & 0x1);
		x = x << 1;
		sysconf_write(sc, soft_jtag_en | trstn_sata | x | tck_low);
		sysconf_write(sc, soft_jtag_en | trstn_sata | x | tck_high);
	}

	/* Push in 0 in the data field */
	for (k = 0; k < 7; k++) {
		sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
		sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	}

	/* Set TAP back round into SHIFT DR STATE
	 * (exit1_dr,update_dr,idle,idle,select_dr,capture_dr) */
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);

	/* Shift out the MIPHY register contents */

	/* Discard first 10 bits */
	for (k = 0; k < 10; k++) {
		sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
		sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	}

	for (k = 0; k < 7; k++) {
		sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
		sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
		ctrlbit = sysconf_read(status_sc);
		ctrlbit = ctrlbit & sata_tdo;
		ctrlbit = ctrlbit >> 1;
		regvalue = ((ctrlbit & 0x1) << k) | regvalue;
	}

	/* Set TAP into idle mode */
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	ctrlbit = sysconf_read(status_sc);
	ctrlbit = ctrlbit & sata_tdo;
	ctrlbit = ctrlbit >> 1;
	regvalue = ((ctrlbit & 0x1) << 7) | regvalue;

	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | tms_sata_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_low);
	sysconf_write(sc, soft_jtag_en | trstn_sata | tck_high);

	return (regvalue);
}

extern void stm_sata_miphy_init(void)
{
	const sysconf_field sc        = (sysconf_field)SYS_CFG33;	/* SYS_CFG33[6:0] */
	const sysconf_field status_sc = (sysconf_field)SYS_STA00;	/* SYS_STA00[2:0] */

	SATA_JTAG_IR_Select_MIPHY(sc);

	/* Force Macro1 in reset and request PLL calibration reset */

	/* Force PLL calibration reset, PLL reset and assert
	 * Deserializer Reset */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x00, 0x16);
	SATA_JTAG_DR_Write_MIPHY(sc, 0x11, 0x0);
	/* Force macro1 to use rx_lspd, tx_lspd (by default rx_lspd
	 * and tx_lspd set for Gen1)  */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x10, 0x1);

	/* Force Recovered clock on first I-DLL phase & all
	 * Deserializers in HP mode */

	/* Force Rx_Clock on first I-DLL phase on macro1 */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x72, 0x40);
	/* Force Des in HP mode on macro1 */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x12, 0x00);

	/* Wait for HFC_READY = 0 */
	while (SATA_JTAG_DR_Read_MIPHY(sc, status_sc, 0x1) & 0x3)
		;

	/* Restart properly Process compensation & PLL Calibration */

	/* Set properly comsr definition for 30 MHz ref clock */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x41, 0x1E);
	/* comsr compensation reference */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x42, 0x33);
	/* Set properly comsr definition for 30 MHz ref clock */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x41, 0x1E);
	/* comsr cal gives more suitable results in fast PVT for comsr
	   used by TX buffer to build slopes making TX rise/fall fall
	   times. */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x42, 0x33);
	/* Force VCO current to value defined by address 0x5A */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x51, 0x2);
	/* Force VCO current to value defined by address 0x5A */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x5A, 0xF);
	/* Enable auto load compensation for pll_i_bias */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x47, 0x2A);
	/* Force restart compensation and enable auto load for
	 * Comzc_Tx, Comzc_Rx & Comsr on macro1 */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x40, 0x13);

	/* Wait for comzc & comsr done */
	while ((SATA_JTAG_DR_Read_MIPHY(sc, status_sc, 0x40) & 0xC) != 0xC)
		;

	/* Recommended settings for swing & slew rate FOR SATA GEN 1
	 * from CPG */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x20, 0x00);
	/* (Tx Swing target 500-550mV peak-to-peak diff) */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x21, 0x2);
	/* (Tx Slew target120-140 ps rising/falling time) */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x22, 0x4);

	/* Force Macro1 in partial mode & release pll cal reset */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x00, 0x10);
	udelay(10);

	SATA_JTAG_DR_Write_MIPHY(sc, 0x50, 0x8D);
	SATA_JTAG_DR_Write_MIPHY(sc, 0x50, 0x8D);

	/*  Wait for phy_ready */
	/*  When phy is in ready state ( register 0x01 of macro1 to 0x13) */
	while ((SATA_JTAG_DR_Read_MIPHY(sc, status_sc, 0x01) & 0x03) != 0x03)
		;

	/* Enable macro1 to use rx_lspd  & tx_lspd from link interface */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x10, 0x00);
	/* Release Rx_Clock on first I-DLL phase on macro1 */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x72, 0x00);

	/* Assert deserializer reset */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x00, 0x10);
	/* des_bit_lock_en is set */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x02, 0x08);

	/* bit lock detection strength */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x86, 0x61);
}

extern void stm_sata_miphy_deassert_des_reset(void)
{
	const sysconf_field sc = (sysconf_field)SYS_CFG33;	/* SYS_CFG33[6:0] */

	/* Deassert deserializer reset */
	SATA_JTAG_DR_Write_MIPHY(sc, 0x00, 0x00);
}

#endif	/* CONFIG_SH_STM_SATA */


