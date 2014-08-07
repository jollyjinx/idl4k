/*
 * (C) Copyright 2009-2010 STMicroelectronics.
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
#include <asm/fli7510reg.h>
#include <asm/io.h>
#include <asm/pio.h>
#include <asm/soc.h>
#include <i2c.h>


extern void flashWriteEnable (void)
{
	/* Enable Vpp for writing to flash */
	/* Nothing to do! */
}

extern void flashWriteDisable (void)
{
	/* Disable Vpp for writing to flash */
	/* Nothing to do! */
}


#ifdef CONFIG_STM_ASC_SERIAL
static void configSerial (void)
{
#if (CFG_STM_ASC_BASE == ST40_ASC0_REGS_BASE)	/* UART #1 */
	/* Route UART #1 via PIO9 for TX, RX, CTS & RTS */
	SET_PIO_ASC(PIO_PORT(9), 3, 2, 1, 0);
#elif (CFG_STM_ASC_BASE == ST40_ASC1_REGS_BASE)	/* UART #2 */
	/* Route UART #2 via PIO25 for TX, RX, CTS & RTS */
	SET_PIO_ASC(PIO_PORT(25), 5, 4, 3, 2);
#elif (CFG_STM_ASC_BASE == ST40_ASC2_REGS_BASE)	/* UART #3 */
	/* Route UART #3 via PIO25 for TX & RX */
	SET_PIO_ASC(PIO_PORT(25), 7, 6, STPIO_NO_PIN, STPIO_NO_PIN);
#else
#error Unknown serial port configuration!
#endif
}
#endif /* CONFIG_STM_ASC_SERIAL */


#if defined(CONFIG_SPI)
static void configSpi(void)
{
	unsigned long sysconf;

	/*
	 * CFG_COMMS_CONFIG_2[13] = spi_enable = 0
	 * i.e. disable the SPI boot-controller.
	 */
	sysconf = readl(CFG_COMMS_CONFIG_2);
	sysconf &= ~(1ul<<13);
	writel(sysconf, CFG_COMMS_CONFIG_2);

	/*
	 *	For both S/W "bit-banging" and H/W SSC, the SPI is on PIO17[5:0].
	 *	Now, we set up the PIO pins correctly.
	 */
	SET_PIO_PIN(PIO_PORT(17),5,STPIO_IN);	/* SPI_MISO */
	SET_PIO_PIN(PIO_PORT(17),4,STPIO_OUT);	/* SPI_CSN */
#if defined(CONFIG_SOFT_SPI)	/* Configure SPI Serial Flash for PIO "bit-banging" */
	SET_PIO_PIN(PIO_PORT(17),2,STPIO_OUT);	/* SPI_CLK */
	SET_PIO_PIN(PIO_PORT(17),3,STPIO_OUT);	/* SPI_MOSI */
#elif defined(CONFIG_STM_SSC_SPI)		/* Use the H/W SSC for SPI */
	SET_PIO_PIN(PIO_PORT(17),2,STPIO_ALT_OUT);/* SPI_CLK */
	SET_PIO_PIN(PIO_PORT(17),3,STPIO_ALT_OUT);/* SPI_MOSI */
#endif	/* CONFIG_SOFT_SPI */

	/* drive (non-SSC) outputs with sensible initial values */
	STPIO_SET_PIN(PIO_PORT(17), 4, 1);	/* deassert SPI_CSN */
#if defined(CONFIG_SOFT_SPI)
	STPIO_SET_PIN(PIO_PORT(17), 2, 1);	/* assert SPI_CLK */
	STPIO_SET_PIN(PIO_PORT(17), 3, 0);	/* deassert SPI_MOSI */
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
	 *	 1	PIO10[2]	PIO10[3]
	 *	 2	PIO9[4]		PIO9[5]
	 *	 3	PIO9[6]		PIO9[7]
	 */
#if defined(CONFIG_I2C_BUS_1)			/* Use I2C Bus "1" */
	SET_PIO_PIN(PIO_PORT(10),2,STPIO_BIDIR);/* I2C1_SCL */
	SET_PIO_PIN(PIO_PORT(10),3,STPIO_BIDIR);/* I2C1_SDA */
#elif defined(CONFIG_I2C_BUS_2)			/* Use I2C Bus "2" */
	SET_PIO_PIN(PIO_PORT(9),4,STPIO_BIDIR);	/* I2C2_SCL */
	SET_PIO_PIN(PIO_PORT(9),5,STPIO_BIDIR);	/* I2C2_SDA */
#elif defined(CONFIG_I2C_BUS_3)			/* Use I2C Bus "3" */
	SET_PIO_PIN(PIO_PORT(9),6,STPIO_BIDIR);	/* I2C3_SCL */
	SET_PIO_PIN(PIO_PORT(9),7,STPIO_BIDIR);	/* I2C3_SDA */
#else
#error Unknown I2C Bus!
#endif
}

extern void fli7510_i2c_scl(const int val)
{
#if defined(CONFIG_I2C_BUS_1)			/* Use I2C Bus "1" */
	STPIO_SET_PIN(PIO_PORT(10), 2, (val) ? 1 : 0);
#elif defined(CONFIG_I2C_BUS_2)			/* Use I2C Bus "2" */
	STPIO_SET_PIN(PIO_PORT(9), 4, (val) ? 1 : 0);
#elif defined(CONFIG_I2C_BUS_3)			/* Use I2C Bus "3" */
	STPIO_SET_PIN(PIO_PORT(9), 6, (val) ? 1 : 0);
#endif
}

extern void fli7510_i2c_sda(const int val)
{
#if defined(CONFIG_I2C_BUS_1)			/* Use I2C Bus "1" */
	STPIO_SET_PIN(PIO_PORT(10), 3, (val) ? 1 : 0);
#elif defined(CONFIG_I2C_BUS_2)			/* Use I2C Bus "2" */
	STPIO_SET_PIN(PIO_PORT(9), 5, (val) ? 1 : 0);
#elif defined(CONFIG_I2C_BUS_3)			/* Use I2C Bus "3" */
	STPIO_SET_PIN(PIO_PORT(9), 7, (val) ? 1 : 0);
#endif
}

extern int fli7510_i2c_read(void)
{
#if defined(CONFIG_I2C_BUS_1)			/* Use I2C Bus "1" */
	return STPIO_GET_PIN(PIO_PORT(10), 3);
#elif defined(CONFIG_I2C_BUS_2)			/* Use I2C Bus "2" */
	return STPIO_GET_PIN(PIO_PORT(9), 5);
#elif defined(CONFIG_I2C_BUS_3)			/* Use I2C Bus "3" */
	return STPIO_GET_PIN(PIO_PORT(9), 7);
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


extern int board_init (void)
{
#ifdef CONFIG_STM_ASC_SERIAL
	configSerial ();
#endif /* CONFIG_STM_ASC_SERIAL */

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	fli7510_configure_ethernet (fli7510_ethernet_mii, 0, 0);
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

#if defined(CONFIG_SOFT_I2C)
	/* Configuration for the I2C bus */
	configI2c();
#endif	/* CONFIG_SOFT_I2C */

	return 0;
}


extern int checkboard (void)
{
	printf ("\n\nBoard: FLI7510 Development Board"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

#if defined(CONFIG_SPI)
	/*
	 * Configure for the SPI Serial Flash.
	 * Note: for CFG_BOOT_FROM_SPI + CFG_ENV_IS_IN_EEPROM, this
	 * needs to be done after env_init(), hence it is done
	 * here, and not in board_init().
	 */
	configSpi();
#endif	/* CONFIG_SPI */

	return 0;
}
