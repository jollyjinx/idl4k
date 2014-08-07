/*
 * (C) Copyright 2008-2009 STMicroelectronics.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/stx7105reg.h>
#include <asm/io.h>
#include <asm/pio.h>

#if defined(CONFIG_USE_FTA_LIBRARY)
#include "fta/definition.h"
#include "../lib_fta/iboot/iboot.h"
#endif

void flashWriteEnable(void)
{
}

void flashWriteDisable(void)
{
}

#if defined(CONFIG_STMAC_KSZ8041)
static void phy_reset_ksz8041(void)
{
  unsigned long sysconf;

  // configure and activate phy reset pin
  SET_PIO_PIN(PIO_PORT(4), 5, STPIO_OUT);
  STPIO_SET_PIN(PIO_PORT(4), 5, 0);

  // strap-pin setup
  SET_PIO_PIN(PIO_PORT(7), 4, STPIO_OUT); /* MIIRX_DV  - CFG2 */
  STPIO_SET_PIN(PIO_PORT(7), 4, 0);
  SET_PIO_PIN(PIO_PORT(7), 5, STPIO_OUT); /* MIIRX_ER  - ISOLATE */
  STPIO_SET_PIN(PIO_PORT(7), 5, 0);
  SET_PIO_PIN(PIO_PORT(8), 6, STPIO_OUT); /* MIIRXD[0] - DUPLEX */
  STPIO_SET_PIN(PIO_PORT(8), 6, 0);
  SET_PIO_PIN(PIO_PORT(8), 7, STPIO_OUT); /* MIIRXD[1] - PHYAD2 */
  STPIO_SET_PIN(PIO_PORT(8), 7, 0);
  SET_PIO_PIN(PIO_PORT(9), 0, STPIO_OUT); /* MIIRXD[2] - PHYAD1 */
  STPIO_SET_PIN(PIO_PORT(9), 0, 0);
  SET_PIO_PIN(PIO_PORT(9), 1, STPIO_OUT); /* MIIRXD[3] - PHYAD0 */
  STPIO_SET_PIN(PIO_PORT(9), 1, 0);
  SET_PIO_PIN(PIO_PORT(9), 3, STPIO_OUT); /* MIICOL    - CFG0 */
  STPIO_SET_PIN(PIO_PORT(9), 3, 0);
  SET_PIO_PIN(PIO_PORT(9), 4, STPIO_OUT); /* MIICRS    - CFG1 */
  STPIO_SET_PIN(PIO_PORT(9), 4, 0);

  // leave reset
  udelay(100);
  STPIO_SET_PIN(PIO_PORT(4), 5, 1);
  udelay(100);

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
}
#endif  /* CONFIG_STMAC_KSZ8041 */

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
#elif defined(CONFIG_I2C_BUS_B)			/* Use I2C Bus "B" */
	SET_PIO_PIN(PIO_PORT(2),5,STPIO_BIDIR);	/* I2C_SCLB */
	SET_PIO_PIN(PIO_PORT(2),6,STPIO_BIDIR);	/* I2C_SDAB */
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
	while( STPIO_GET_PIN(PIO_PORT(2), 2) != val )
#elif defined(CONFIG_I2C_BUS_B)			/* Use I2C Bus "B" */
	STPIO_SET_PIN(PIO_PORT(2), 5, (val) ? 1 : 0);
	while( STPIO_GET_PIN(PIO_PORT(2), 5) != val )
#elif defined(CONFIG_I2C_BUS_C)			/* Use I2C Bus "C" */
	STPIO_SET_PIN(PIO_PORT(3), 4, (val) ? 1 : 0);
	while( STPIO_GET_PIN(PIO_PORT(3), 4) != val )
#elif defined(CONFIG_I2C_BUS_D)			/* Use I2C Bus "D" */
	STPIO_SET_PIN(PIO_PORT(3), 6, (val) ? 1 : 0);
	while( STPIO_GET_PIN(PIO_PORT(3), 6) != val )
#endif
	{
		/* NOP */;
	}
}

extern void stx7105_i2c_sda(const int val)
{
#if defined(CONFIG_I2C_BUS_A)			/* Use I2C Bus "A" */
	STPIO_SET_PIN(PIO_PORT(2), 3, (val) ? 1 : 0);
#elif defined(CONFIG_I2C_BUS_B)			/* Use I2C Bus "B" */
	STPIO_SET_PIN(PIO_PORT(2), 6, (val) ? 1 : 0);
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
#elif defined(CONFIG_I2C_BUS_B)			/* Use I2C Bus "B" */
	return STPIO_GET_PIN(PIO_PORT(2), 6);
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

  /* leave light standby */
  SET_PIO_PIN(PIO_PORT(3), 7, STPIO_OUT);
  STPIO_SET_PIN(PIO_PORT(3), 7, 1);

  /* Setup PIO of ASC device */
  SET_PIO_ASC(PIO_PORT(1), 0, 1, 2, 3);  /* UART1 - AS0 */

  sysconf  = *STX7105_SYSCONF_SYS_CFG20;
  sysconf &= ~0x00000303;
  sysconf |= 0x00000303;
  *STX7105_SYSCONF_SYS_CFG20 = sysconf;

  /* Switch the LOGO on */
  SET_PIO_PIN(PIO_PORT(3), 1, STPIO_OUT);
  STPIO_SET_PIN(PIO_PORT(3), 1, 1);

  /* Switch the LED on */
  SET_PIO_PIN(PIO_PORT(3), 2, STPIO_OUT);
  STPIO_SET_PIN(PIO_PORT(3), 2, 1);
}

#if defined(CONFIG_USE_FTA_LIBRARY)
extern void setVideoPio(void)
{
	/* SCART Fast Blanking */
	STPIO_SET_PIN(PIO_PORT(4), 3, 0);
	SET_PIO_PIN(PIO_PORT(4), 3, STPIO_OUT);

	/* SCART Slow Blanking AV */
	STPIO_SET_PIN(PIO_PORT(4), 1, 0);
	SET_PIO_PIN(PIO_PORT(4), 1, STPIO_OUT);

	/* SCART Slow Blanking 16/9 */
	STPIO_SET_PIN(PIO_PORT(4), 2, 0);
	SET_PIO_PIN(PIO_PORT(4), 2, STPIO_OUT);
}

extern U32 getKeyPressed(void)
{
	U32 flag = FP_KEY_NONE;

	if( !STPIO_GET_PIN(PIO_PORT(5), 4) )
		flag |= FP_KEY_POWER;
	if( !STPIO_GET_PIN(PIO_PORT(5), 5) )
		flag |= FP_KEY_DOWN;
	if( !STPIO_GET_PIN(PIO_PORT(5), 6) )
		flag |= FP_KEY_UP;

	return flag;
}

extern U32 board_getConfig(int useEnv)
{
	/* Override the PIO configuration ? */
	if(useEnv)
	{
		I8 *ptr = getenv("config");
		if(ptr)
			return simple_strtoul(ptr, NULL, 16) & PIO_CONFIG_MASK;
	}

	return readl(PIO_PORT(15) + STPIO_PIN_OFFSET) & PIO_CONFIG_MASK;
}
#endif

extern int board_init(void)
{
  configPIO();

#if defined(CONFIG_SOFT_I2C)
  configI2c();
#endif

  /* Reset the PHY */
#if defined(CONFIG_STMAC_KSZ8041)
  phy_reset_ksz8041();
#endif  /* CONFIG_STMAC_KSZ8041 */

  return 0;
}


#define is_hexDigit(x)	(((x) >= '0') && ((x) <= '9'))
#define is_hexSmall(x)	(((x) >= 'a') && ((x) <= 'f'))
#define is_hexBig(x)	(((x) >= 'A') && ((x) <= 'F'))

#define is_hexChar(x)	(is_hexDigit(x) || is_hexSmall(x) || is_hexBig(x))

extern void platformSettingsInit(void)
{
  char macAddrString[18];
  const char *plsMac = (const char *)MAC_START;
  int i;
  int semicolon = 2;

  /* MAC is not valid (default) */
  setenv("ethaddr", NULL);

  /* Check validity of the MAC */
  for(i=0; i<17; i++)
  {
    if(i == semicolon)
    {
      if(plsMac[i] != ':')
        return;
      semicolon += 3;
    }
    else if( !is_hexChar(plsMac[i]) )
    {
      return;
    }
  }

  /* The MAC in the PLS is valid */
  memcpy(macAddrString, (void*)MAC_START, 17);
  macAddrString[17] = 0;
  setenv("ethaddr", macAddrString);
}

int checkboard (void)
{
  printf ("\n\nBoard: Inverto IDL52K (Volksbox-2)"
#ifdef CONFIG_SH_SE_MODE
    "  [32-bit mode]"
#else
    "  [29-bit mode]"
#endif
    "\n");

  return 0;
}
