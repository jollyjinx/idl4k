/*
 * (C) Copyright 2008-2011 STMicroelectronics.
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
#include <asm/stx7105reg.h>
#include <asm/io.h>
#include <asm/pio.h>
#include <i2c.h>



void flashWriteEnable(void)
{
	/* Enable Vpp for writing to flash */
	/* Nothing to do! */
}

void flashWriteDisable(void)
{
	/* Enable Vpp for writing to flash */
	/* Nothing to do! */
}

static void configEthernet(void)
{
	unsigned long sysconf;

	/* Configure & Reset the Ethernet PHY */

	/* Set the GMAC in MII mode */
	sysconf = *STX7105_SYSCONF_SYS_CFG07;
	sysconf &= ~0x060f0000ul;
	sysconf |=  0x08010000ul;
	*STX7105_SYSCONF_SYS_CFG07 = sysconf;

	sysconf = *STX7105_SYSCONF_SYS_CFG37;
	/* PIO7[4] CFG37[12,4]  AltFunction = 1 */
	/* PIO7[5] CFG37[13,5]  AltFunction = 1 */
	/* PIO7[6] CFG37[14,6]  AltFunction = 1 */
	/* PIO7[7] CFG37[15,7]  AltFunction = 1 */
	sysconf &= ~0xf0f0ul;	/* 3,3,3,3,0,0,0,0 */
	*STX7105_SYSCONF_SYS_CFG37 = sysconf;

	sysconf = *STX7105_SYSCONF_SYS_CFG46;
	/* PIO8[0] CFG46[8,0]   AltFunction = 1 */
	/* PIO8[1] CFG46[9,1]   AltFunction = 1 */
	/* PIO8[2] CFG46[10,2]  AltFunction = 1 */
	/* PIO8[3] CFG46[11,3]  AltFunction = 1 */
	/* PIO8[4] CFG46[12,4]  AltFunction = 1 */
	/* PIO8[5] CFG46[13,5]  AltFunction = 1 */
	/* PIO8[6] CFG46[14,6]  AltFunction = 1 */
	/* PIO8[7] CFG46[15,7]  AltFunction = 1 */
	sysconf &= ~0xfffful;	/* 3,3,3,3,3,3,3,3 */
	*STX7105_SYSCONF_SYS_CFG46 = sysconf;

	sysconf = *STX7105_SYSCONF_SYS_CFG47;
	/* PIO9[0] CFG47[8,0]   AltFunction = 1 */
	/* PIO9[1] CFG47[9,1]   AltFunction = 1 */
	/* PIO9[2] CFG47[10,2]  AltFunction = 1 */
	/* PIO9[3] CFG47[11,3]  AltFunction = 1 */
	/* PIO9[4] CFG47[12,4]  AltFunction = 1 */
	/* PIO9[5] CFG47[13,5]  AltFunction = 1 */
	/* PIO9[6] CFG47[14,6]  AltFunction = 1 */
	sysconf &= ~0x7f7ful;	/* 0,3,3,3,3,3,3,3 */
	*STX7105_SYSCONF_SYS_CFG47 = sysconf;

	/* Setup PIO for the Ethernet's MII bus */
	SET_PIO_PIN(PIO_PORT(7),4,STPIO_IN);
	SET_PIO_PIN(PIO_PORT(7),5,STPIO_IN);
	SET_PIO_PIN(PIO_PORT(7),6,STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(7),7,STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(8),0,STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(8),1,STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(8),2,STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(8),3,STPIO_ALT_BIDIR);
	SET_PIO_PIN(PIO_PORT(8),4,STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(8),5,STPIO_IN);
	SET_PIO_PIN(PIO_PORT(8),6,STPIO_IN);
	SET_PIO_PIN(PIO_PORT(8),7,STPIO_IN);
	SET_PIO_PIN(PIO_PORT(9),0,STPIO_IN);
	SET_PIO_PIN(PIO_PORT(9),1,STPIO_IN);
	SET_PIO_PIN(PIO_PORT(9),2,STPIO_IN);
	SET_PIO_PIN(PIO_PORT(9),3,STPIO_IN);
	SET_PIO_PIN(PIO_PORT(9),4,STPIO_IN);
	SET_PIO_PIN(PIO_PORT(9),5,STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(9),6,STPIO_IN);

	/* Setup PIO for the PHY's reset */
//	SET_PIO_PIN(PIO_PORT(15), 5, STPIO_OUT);
	SET_PIO_PIN(PIO_PORT(4), 2, STPIO_OUT);

//General power on.
	SET_PIO_PIN(PIO_PORT(4), 3, STPIO_OUT);
	STPIO_SET_PIN(PIO_PORT(4), 3, 1);

	/* Finally, toggle the PHY Reset pin ("RST#") */
//	STPIO_SET_PIN(PIO_PORT(15), 5, 0);
	STPIO_SET_PIN(PIO_PORT(4), 2, 0);
//	udelay(100);	/* small delay */
	udelay(10000);	/* small delay */
//	STPIO_SET_PIN(PIO_PORT(15), 5, 1);
	STPIO_SET_PIN(PIO_PORT(4), 2, 1);
}

#if defined(CONFIG_SPI)
static void configSpi(void)
{
#if defined(CONFIG_STM_SSC_SPI)		/* Use the H/W SSC for SPI */
	unsigned long sysconf;
#endif	/* CONFIG_STM_SSC_SPI */

	/*
	 * On the PDK-7105 board, the following 4 pairs of PIO
	 * pins are each connected together with a 3K3 resistor.
	 *
	 *	SPI_NOTCS PIO15[2] <-> PIO2[4] COM_NOTCS
	 *	SPI_CLK   PIO15[0] <-> PIO2[5] COM_CLK
	 *	SPI_DOUT  PIO15[1] <-> PIO2[6] COM_DOUT
	 *	SPI_DIN   PIO15[3] <-> PIO2[7] COM_DIN
	 *
	 * To minimise drive "contention", we may set associated
	 * pins on the "other" PIO bank to be simple inputs.
	 */
#if defined(CONFIG_SOFT_SPI)
	SET_PIO_PIN(PIO_PORT(2),5,STPIO_IN);	/* COM_CLK */
	SET_PIO_PIN(PIO_PORT(2),6,STPIO_IN);	/* COM_DOUT */
	SET_PIO_PIN(PIO_PORT(2),7,STPIO_IN);	/* COM_DIN */
#elif defined(CONFIG_STM_SSC_SPI)		/* Use the H/W SSC for SPI */
	SET_PIO_PIN(PIO_PORT(15),0,STPIO_IN);	/* SPI_CLK */
	SET_PIO_PIN(PIO_PORT(15),1,STPIO_IN);	/* SPI_DOUT */
	SET_PIO_PIN(PIO_PORT(15),3,STPIO_IN);	/* SPI_DIN */
#endif	/* CONFIG_SOFT_SPI */

	/*
	 * Because of the above resistors, we can control
	 * the CSn line, *either* through PIO15[2] or PIO2[4].
	 * This decision is orthogonal to whither we are
	 * using the H/W SSC, or the S/W PIO bit-banging.
	 * So, for simplicity, we will use exclusively
	 * use PIO15[2] for both choices.
	 */
#if 1	/* Use PIO15[2] for SPI CSn */
	SET_PIO_PIN(PIO_PORT(2),4,STPIO_IN);	/* COM_NOTCS */
	SET_PIO_PIN(PIO_PORT(15),2,STPIO_OUT);	/* SPI_NOTCS */
	STPIO_SET_PIN(PIO_PORT(15), 2, 1);	/* deassert SPI_NOTCS */
#else	/* Use PIO2[4] for SPI CSn */
	SET_PIO_PIN(PIO_PORT(15),2,STPIO_IN);	/* SPI_NOTCS */
	SET_PIO_PIN(PIO_PORT(2),4,STPIO_OUT);	/* COM_NOTCS */
	STPIO_SET_PIN(PIO_PORT(2), 4, 1);	/* deassert COM_NOTCS */
#endif

#if defined(CONFIG_SOFT_SPI)	/* Configure SPI Serial Flash for PIO "bit-banging" */
	/* SPI is on PIO15[3:0] */
	SET_PIO_PIN(PIO_PORT(15),3,STPIO_IN);	/* SPI_DIN */
	SET_PIO_PIN(PIO_PORT(15),0,STPIO_OUT);	/* SPI_CLK */
	SET_PIO_PIN(PIO_PORT(15),1,STPIO_OUT);	/* SPI_DOUT */

	/* drive outputs with sensible initial values */
	STPIO_SET_PIN(PIO_PORT(15), 0, 1);	/* assert SPI_CLK */
	STPIO_SET_PIN(PIO_PORT(15), 1, 0);	/* deassert SPI_DOUT */
#elif defined(CONFIG_STM_SSC_SPI)		/* Use the H/W SSC for SPI */
	/* Set PIO2_ALTFOP to AltFunction #3 (SSC) */
	sysconf = *STX7105_SYSCONF_SYS_CFG21;
	/* PIO2[5] CFG21[13,5]  AltFunction = 3 */
	/* PIO2[6] CFG21[14,6]  AltFunction = 3 */
	/* PIO2[7] CFG21[15,7]  AltFunction = 3 */
	sysconf &= ~0xe0e0ul;	/* 3,3,3,0,0,0,0,0 */
	sysconf |=  0xe000ul;	/* 2,2,2,0,0,0,0,0 */
	*STX7105_SYSCONF_SYS_CFG21 = sysconf;

	/* SPI is on PIO2[7:5] */
	SET_PIO_PIN(PIO_PORT(2),7,STPIO_IN);		/* COM_DIN */
	SET_PIO_PIN(PIO_PORT(2),5,STPIO_ALT_OUT);	/* COM_CLK */
	SET_PIO_PIN(PIO_PORT(2),6,STPIO_ALT_OUT);	/* COM_DOUT */

	/* route MRST to PIO2[7], for SSC #1 */
	sysconf = *STX7105_SYSCONF_SYS_CFG16;
	sysconf |= (1u<<3);	/* CFG16[3] = SSC1_MRST_IN_SEL = 1 */
	*STX7105_SYSCONF_SYS_CFG16 = sysconf;
#endif	/* CONFIG_SOFT_SPI */
}
#endif	/* CONFIG_SPI */


#if defined(CONFIG_SOFT_I2C)
static void configI2c(void)
{
	/*
	 * The I2C busses are routed as follows:
	 *
	 *	Bus	  SCL		  SDA
	 *	---	  ---		  ---
	 *	 A	PIO2[2]		PIO2[3]
	 *	 B	PIO2[5]		PIO2[6]		Used only for SPI
	 *	 C	PIO3[4]		PIO3[5]
	 *	 D	PIO3[6]		PIO3[7]
	 */
#if defined(CONFIG_I2C_BUS_A)			/* Use I2C Bus "A" */
	SET_PIO_PIN(PIO_PORT(2),2,STPIO_BIDIR);	/* I2C_SCLA */
	SET_PIO_PIN(PIO_PORT(2),3,STPIO_BIDIR);	/* I2C_SDAA */
#elif defined(CONFIG_I2C_BUS_C)			/* Use I2C Bus "C" */
	SET_PIO_PIN(PIO_PORT(3),4,STPIO_BIDIR);	/* I2C_SCLC */
	SET_PIO_PIN(PIO_PORT(3),5,STPIO_BIDIR);	/* I2C_SDAC */
#elif defined(CONFIG_I2C_BUS_D)			/* Use I2C Bus "D" */
	SET_PIO_PIN(PIO_PORT(3),6,STPIO_BIDIR);	/* I2C_SCLD */
	SET_PIO_PIN(PIO_PORT(3),7,STPIO_BIDIR);	/* I2C_SDAD */
#else
#error Unknown I2C Bus!
#endif
}

extern void stx7105_i2c_scl(const int val)
{
#if defined(CONFIG_I2C_BUS_A)			/* Use I2C Bus "A" */
	STPIO_SET_PIN(PIO_PORT(2), 2, (val) ? 1 : 0);
#elif defined(CONFIG_I2C_BUS_C)			/* Use I2C Bus "C" */
	STPIO_SET_PIN(PIO_PORT(3), 4, (val) ? 1 : 0);
#elif defined(CONFIG_I2C_BUS_D)			/* Use I2C Bus "D" */
	STPIO_SET_PIN(PIO_PORT(3), 6, (val) ? 1 : 0);
#endif
}

extern void stx7105_i2c_sda(const int val)
{
#if defined(CONFIG_I2C_BUS_A)			/* Use I2C Bus "A" */
	STPIO_SET_PIN(PIO_PORT(2), 3, (val) ? 1 : 0);
#elif defined(CONFIG_I2C_BUS_C)			/* Use I2C Bus "C" */
	STPIO_SET_PIN(PIO_PORT(3), 5, (val) ? 1 : 0);
#elif defined(CONFIG_I2C_BUS_D)			/* Use I2C Bus "D" */
	STPIO_SET_PIN(PIO_PORT(3), 7, (val) ? 1 : 0);
#endif
}

extern int stx7105_i2c_read(void)
{
#if defined(CONFIG_I2C_BUS_A)			/* Use I2C Bus "A" */
	return STPIO_GET_PIN(PIO_PORT(2), 3);
#elif defined(CONFIG_I2C_BUS_C)			/* Use I2C Bus "C" */
	return STPIO_GET_PIN(PIO_PORT(3), 5);
#elif defined(CONFIG_I2C_BUS_D)			/* Use I2C Bus "D" */
	return STPIO_GET_PIN(PIO_PORT(3), 7);
#endif
}
#endif	/* CONFIG_SOFT_I2C */

#if defined(CONFIG_I2C_CMD_TREE)
extern unsigned int i2c_get_bus_speed(void)
{
	return CFG_I2C_SPEED;
}
extern int i2c_set_bus_speed(unsigned int speed)
{
	return -1;
}
#endif	/* CONFIG_I2C_CMD_TREE */

static void configPIO(void)
{
	unsigned long sysconf;

	/* Setup PIO of ASC device */
#if CFG_STM_ASC_BASE == ST40_ASC0_REGS_BASE	/* UART #0 */
	SET_PIO_ASC(PIO_PORT(0), 0, 1, 4, 3);  /* UART0 */
#elif CFG_STM_ASC_BASE == ST40_ASC2_REGS_BASE	/* UART #2 */
	//SET_PIO_ASC(PIO_PORT(4), 0, 1, 2, 3);  /* UART2 */
	SET_PIO_ASC(PIO_PORT(4), 0, 1, STPIO_NO_PIN, STPIO_NO_PIN);  /* UART2 */
#elif CFG_STM_ASC_BASE == ST40_ASC3_REGS_BASE	/* UART #3 */
	SET_PIO_ASC(PIO_PORT(5), 0, 1, 3, 2);  /* UART3 */
#else
#error Unsure which UART to configure!
#endif	/* CFG_STM_ASC_BASE == ST40_ASCx_REGS_BASE */

#if CFG_STM_ASC_BASE == ST40_ASC0_REGS_BASE	/* UART #0 */
	/* Route UART0 via PIO0 for TX, RX, CTS & RTS */
	sysconf = *STX7105_SYSCONF_SYS_CFG19;
	/* PIO0[0] CFG19[16,8,0]   AltFunction = 4 */
	/* PIO0[1] CFG19[17,9,1]   AltFunction = 4 */
	/* PIO0[3] CFG19[19,11,3]  AltFunction = 4 */
	/* PIO0[4] CFG19[20,12,4]  AltFunction = 4 */
	sysconf &= ~0x1b1b1bul;	/* 7,7,0,7,7 */
	sysconf |=  0x001b1bul;	/* 3,3,0,3,3 */
	*STX7105_SYSCONF_SYS_CFG19 = sysconf;
#elif CFG_STM_ASC_BASE == ST40_ASC2_REGS_BASE	/* UART #2 */
	/* Select UART2 via PIO4 */
	sysconf = *STX7105_SYSCONF_SYS_CFG07;
	/* CFG07[1] = UART2_RXD_SRC_SELECT = 0 */
	/* CFG07[2] = UART2_CTS_SRC_SELECT = 0 */
	sysconf &= ~(1ul<<2 | 1ul<<1);
	*STX7105_SYSCONF_SYS_CFG07 = sysconf;
	/* Route UART2 via PIO4 for TX, RX, CTS & RTS */
	sysconf = *STX7105_SYSCONF_SYS_CFG34;
	/* PIO4[0] CFG34[8,0]   AltFunction = 3 */
	/* PIO4[1] CFG34[9,1]   AltFunction = 3 */
	/* PIO4[2] CFG34[10,2]  AltFunction = 3 */
	/* PIO4[3] CFG34[11,3]  AltFunction = 3 */
	sysconf &= ~0x0f0ful;	/* 3,3,3,3 */
	sysconf |=  0x0f00ul;	/* 2,2,2,2 */
	*STX7105_SYSCONF_SYS_CFG34 = sysconf;
#elif CFG_STM_ASC_BASE == ST40_ASC3_REGS_BASE	/* UART #3 */
	/* Route UART3 via PIO5 for TX, RX, CTS & RTS */
	sysconf = *STX7105_SYSCONF_SYS_CFG35;
	/* PIO5[0] CFG35[8,0]   AltFunction = 3 */
	/* PIO5[1] CFG35[9,1]   AltFunction = 3 */
	/* PIO5[2] CFG35[10,2]  AltFunction = 3 */
	/* PIO5[3] CFG35[11,3]  AltFunction = 3 */
	sysconf &= ~0x0f0ful;	/* 3,3,3,3 */
	sysconf |=  0x000ful;	/* 1,1,1,1 */
	*STX7105_SYSCONF_SYS_CFG35 = sysconf;
#else
#error Unsure which UART to configure!
#endif	/* CFG_STM_ASC_BASE == ST40_ASCx_REGS_BASE */

	/* Configure & Reset the Ethernet PHY */
	configEthernet();

#if defined(CONFIG_SPI)
	/* Configure for SPI Serial Flash */
	configSpi();
#endif	/* CONFIG_SPI */

#if defined(CONFIG_SOFT_I2C)
	/* Configuration for the I2C bus */
	configI2c();
#endif	/* CONFIG_SOFT_I2C */
}

extern int board_init(void)
{
	configPIO();

#if defined(CONFIG_SH_STM_SATA)
	stx7105_configure_sata ();
#endif	/* CONFIG_SH_STM_SATA */

	return 0;
}

int checkboard (void)
{
	printf ("\n\nBoard: HDK7106"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

	return 0;
}
