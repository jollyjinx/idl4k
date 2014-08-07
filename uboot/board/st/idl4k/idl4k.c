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

#ifdef CONFIG_PLATFORM_SETTINGS_INIT

typedef struct idl4k_platform_settings_s
{
  unsigned char hwid[16]; // hardware id
  unsigned char sn[16];   // serial number
  unsigned char mac[6];  // mac address
  unsigned char uuid[16]; // global unique id
} __attribute__((packed)) idl4k_platform_settings_t;

static idl4k_platform_settings_t idl4kS;
static const char mapHex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

#define IDL4K_PLATFORM_SETTINGS_BASE_ADDR 0xFFC00
//#define DEBUG_PLATFORM_SETTINGS
#endif /* CONFIG_PLATFORM_SETTINGS_INIT */

#ifdef CONFIG_DRIVER_NET_STM_GMAC
void stmac_set_mac_addr (unsigned char *Addr);
#endif


void flashWriteEnable(void)
{
	/* Enable Vpp for writing to flash */
	/* QQQ - TO DO */
}

void flashWriteDisable(void)
{
	/* Disable Vpp for writing to flash */
	/* QQQ - TO DO */
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

#if CFG_STM_ASC_BASE == ST40_ASC1_REGS_BASE

	/* Route UART1 via PIO5 for TX, RX, CTS & RTS (Alternative #1) */
	PIOALT(5, 1, 1, &stx7108_pioalt_pad_out);	/* UART1-TX */
	PIOALT(5, 2, 1, &stx7108_pioalt_pad_in);	/* UART1-RX */
	PIOALT(5, 4, 1, &stx7108_pioalt_pad_out);	/* UART1-RTS */
	PIOALT(5, 3, 1, &stx7108_pioalt_pad_in);	/* UART1-CTS */

#elif CFG_STM_ASC_BASE == ST40_ASC3_REGS_BASE

	/* Route UART3 via PIO24/25 for TX, RX (Alternative #1) */
	PIOALT(24, 4, 1, &stx7108_pioalt_pad_out);	/* UART3-TX */
	PIOALT(24, 5, 1, &stx7108_pioalt_pad_in);	/* UART3-RX */
//	PIOALT(24, 7, 1, &stx7108_pioalt_pad_out);	/* UART3-RTS */
//	PIOALT(25, 0, 1, &stx7108_pioalt_pad_in);	/* UART3-CTS */

#else
#error Unknown ASC port selected!
#endif	/* CFG_STM_ASC_BASE == ST40_ASCx_REGS_BASE */

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	/*
	 * Configure the Ethernet PHY Reset signal
	 *	PIO15[4] == POWER_ON_ETH (a.k.a. ETH_RESET)
	 */
	SET_PIO_PIN(ST40_PIO_BASE(15), 4, STPIO_OUT);
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

	/*
	 * Some of the peripherals are powered by regulators
	 * controlled by the following PIO line...
	 *	PIO5[0] == POWER_ON
	 */
	SET_PIO_PIN(ST40_PIO_BASE(5), 0, STPIO_OUT);
	STPIO_SET_PIN(ST40_PIO_BASE(5), 0, 1);

	/* setup led pins */
	SET_PIO_PIN(ST40_PIO_BASE(3), 0, STPIO_OUT);
	SET_PIO_PIN(ST40_PIO_BASE(3), 1, STPIO_OUT);
	SET_PIO_PIN(ST40_PIO_BASE(3), 2, STPIO_OUT);
	SET_PIO_PIN(ST40_PIO_BASE(3), 3, STPIO_OUT);
	SET_PIO_PIN(ST40_PIO_BASE(3), 4, STPIO_OUT);
	SET_PIO_PIN(ST40_PIO_BASE(3), 5, STPIO_OUT);
	SET_PIO_PIN(ST40_PIO_BASE(3), 6, STPIO_OUT);

	SET_PIO_PIN(PIO_PORT(26), 7, STPIO_IN); // factory-defaults-reset-pin
}

#ifdef CONFIG_DRIVER_NET_STM_GMAC
extern void stmac_phy_reset(void)
{
	/*
	 * Reset the Ethernet PHY.
	 * Note both PHYs share the *same* reset line.
	 *
	 *	PIO15[4] = POWER_ON_ETH (a.k.a. ETH_RESET)
	 */
	STPIO_SET_PIN(ST40_PIO_BASE(15), 4, 0);
	udelay(10000);				/* 10 ms */
	STPIO_SET_PIN(ST40_PIO_BASE(15), 4, 1);
}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

extern int board_init(void)
{
	configPIO();

	STPIO_SET_PIN(PIO_PORT(3), 0, 0);
	STPIO_SET_PIN(PIO_PORT(3), 1, 1);
	STPIO_SET_PIN(PIO_PORT(3), 2, 0);
	STPIO_SET_PIN(PIO_PORT(3), 3, 0);
	STPIO_SET_PIN(PIO_PORT(3), 4, 0);
	STPIO_SET_PIN(PIO_PORT(3), 5, 0);
	STPIO_SET_PIN(PIO_PORT(3), 6, 0);

#ifdef QQQ	/* QQQ - DELETE */
#if defined(CONFIG_SH_STM_SATA)
	stx7105_configure_sata ();
#endif	/* CONFIG_SH_STM_SATA */
#endif		/* QQQ - DELETE */

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	/* Reset the PHY */
	stmac_phy_reset();
#if CFG_STM_STMAC_BASE == CFG_STM_STMAC0_BASE		/* MII0, on MII JP2 */
	stx7108_configure_ethernet(0, &(struct stx7108_ethernet_config) {
			.mode = stx7108_ethernet_mode_mii,
			.ext_clk = 1,
			.phy_bus = 0, });
#elif CFG_STM_STMAC_BASE == CFG_STM_STMAC1_BASE		/* MII1, IC+ IP1001 (UP1) */
	stx7108_configure_ethernet(1, &(struct stx7108_ethernet_config) {
			.mode = stx7108_ethernet_mode_mii,
		//QQQ	.mode = stx7108_ethernet_mode_gmii_gtx,
			.ext_clk = 0,
			.phy_bus = 1, });
#else
#error Unknown base address for the STM GMAC
#endif

#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

#if defined(CONFIG_CMD_I2C)
	stx7108_configure_i2c();
#endif	/* CONFIG_CMD_I2C */

	return 0;
}

int checkboard (void)
{
	printf ("\n\nBoard: idl4k"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

	return 0;
}

#ifdef CONFIG_PLATFORM_SETTINGS_INIT

#ifdef DEBUG_PLATFORM_SETTINGS
static void dbgDumpMemory(const unsigned char* buffer, const unsigned int bufferLen)
{
  unsigned int i;
  for (i=0; i<bufferLen; i++) { printf("%02X", buffer[i]); }
  printf("\n");
}
#endif

void platformSettingsInit()
{
  memset(&idl4kS, 0, sizeof(idl4kS));
  if (!eeprom_read(0, IDL4K_PLATFORM_SETTINGS_BASE_ADDR, (uchar*)&idl4kS, sizeof(idl4kS)))
  {
#ifdef DEBUG_PLATFORM_SETTINGS
    printf("[idl4k] platform custom settings:\n");
    printf("          - hwid : "); dbgDumpMemory((unsigned char*)idl4kS.hwid, sizeof(idl4kS.hwid));
    printf("          - sn   : "); dbgDumpMemory((unsigned char*)idl4kS.sn, sizeof(idl4kS.sn));
    printf("          - mac  : "); dbgDumpMemory((unsigned char*)idl4kS.mac, sizeof(idl4kS.mac));
    printf("          - uuid : "); dbgDumpMemory((unsigned char*)idl4kS.uuid, sizeof(idl4kS.uuid));
#endif

    {
      char macAddrString[32] = {0};
      unsigned macAddrPos=0,i;
      for (i=0; i<sizeof(idl4kS.mac)-1; i++)
      {
        macAddrString[macAddrPos++] = mapHex[(idl4kS.mac[i]>>4)&0xf];
        macAddrString[macAddrPos++] = mapHex[idl4kS.mac[i]&0xf];
        macAddrString[macAddrPos++] = ':';
      }
      macAddrString[macAddrPos++] = mapHex[(idl4kS.mac[sizeof(idl4kS.mac)-1]>>4)&0xf];
      macAddrString[macAddrPos++] = mapHex[idl4kS.mac[sizeof(idl4kS.mac)-1]&0xf];
      macAddrString[macAddrPos++] = 0;

      setenv("ethaddr", macAddrString);
    }
  }
}

void addNumberParamToBootargs(char* extraBootargs, unsigned int extraBootargsLen, const char* paramName, const unsigned char* paramValue, unsigned int paramValueLen)
{
  unsigned int extraBootargsPos=strlen(extraBootargs);
  if (extraBootargsPos+strlen(paramName)+paramValueLen*2 + 1 < extraBootargsLen)
  {
    unsigned int i;
    strcat(extraBootargs, paramName);
    extraBootargsPos+=strlen(paramName);
    for (i=0; i<paramValueLen; i++)
    {
      extraBootargs[extraBootargsPos++] = mapHex[(paramValue[i]>>4)&0xf];
      extraBootargs[extraBootargsPos++] = mapHex[paramValue[i]&0xf];
    }
    extraBootargs[extraBootargsPos] = 0;
  }
}

void addStringParamToBootargs(char* extraBootargs, unsigned int extraBootargsLen, const char* paramName, const unsigned char* paramValue, unsigned int paramValueLen)
{
  unsigned int extraBootargsPos=strlen(extraBootargs);
  if (extraBootargsPos+strlen(paramName)+paramValueLen + 1 < extraBootargsLen)
  {
    strcat(extraBootargs, paramName);
    extraBootargsPos+=strlen(paramName);
    memcpy(extraBootargs+extraBootargsPos, paramValue, paramValueLen);
    extraBootargsPos += paramValueLen;
    extraBootargs[extraBootargsPos] = 0;
  }
}

void getExtraBootArgs(char* extraBootargs, unsigned int extraBootargsLen)
{
  addNumberParamToBootargs(extraBootargs, extraBootargsLen, " hw=", idl4kS.hwid, sizeof(idl4kS.hwid));
  addStringParamToBootargs(extraBootargs, extraBootargsLen, " sn=", idl4kS.sn, sizeof(idl4kS.sn));
  addNumberParamToBootargs(extraBootargs, extraBootargsLen, " uuid=", idl4kS.uuid, sizeof(idl4kS.uuid));

  STPIO_SET_PIN(PIO_PORT(3), 2, 0);
  STPIO_SET_PIN(PIO_PORT(3), 3, 0);
  STPIO_SET_PIN(PIO_PORT(3), 4, 0);
  STPIO_SET_PIN(PIO_PORT(3), 5, 0);
  STPIO_SET_PIN(PIO_PORT(3), 6, 0);

  if (!STPIO_GET_PIN(PIO_PORT(26), 7))
  {
    int defaultReset = 1;
    int delayLoopCnt = 0;
    while (delayLoopCnt++<100)
    {
      udelay(20000);
      if (STPIO_GET_PIN(PIO_PORT(26), 7))
      {
        defaultReset = 0;
        break;
      }
    }
    if (defaultReset)
    {
      if (strlen(CONFIG_IDENT_STRING)+strlen(" fdef=1") < extraBootargsLen-strlen(extraBootargs))
      {
        strcat(extraBootargs, " fdef=1");
      }
    }
  }

  if (strlen(CONFIG_IDENT_STRING)+strlen(" revBoot=") < extraBootargsLen-strlen(extraBootargs))
  {
    strcat(extraBootargs, " revBoot=");
    strcat(extraBootargs, CONFIG_IDENT_STRING);
  }

  // mac address must start from 00:0F:6F or 00:24:A6
  if (!(idl4kS.mac[0] == 0x00 && idl4kS.mac[1] == 0x0F && idl4kS.mac[2] == 0x6F) &&
      !(idl4kS.mac[0] == 0x00 && idl4kS.mac[1] == 0x24 && idl4kS.mac[2] == 0xA6))
  {
    int ledOn=0;
    while (1)
    {
      udelay(100000);
      STPIO_SET_PIN(PIO_PORT(3), 0, ledOn);
      ledOn = !ledOn;
    }
  }
}

#endif

#ifdef CUSTOM_SHOW_USB_PROGRESS
void custom_usb_show_progress(void)
{
  static int ledOn = 2;
  static int callCnt=0;
  static int dirForward=1;
  if (callCnt++ > 10)
  {
    STPIO_SET_PIN(PIO_PORT(3), ledOn, 0);
    if (dirForward)
    {
      ledOn++;
      if (ledOn>6) { ledOn=5; dirForward=0; }
    }
    else
    {
      ledOn--;
      if (ledOn<2) { ledOn=3; dirForward=1; }
    }

    STPIO_SET_PIN(PIO_PORT(3), ledOn, 1);
    callCnt = 0;
  }
}
#endif

#ifdef CUSTOM_SHOW_FATLOAD_PROGRESS
void custom_fatload_show_start(void)
{
  STPIO_SET_PIN(PIO_PORT(3), 2, 0);
  STPIO_SET_PIN(PIO_PORT(3), 3, 0);
  STPIO_SET_PIN(PIO_PORT(3), 4, 0);
  STPIO_SET_PIN(PIO_PORT(3), 5, 0);
  STPIO_SET_PIN(PIO_PORT(3), 6, 0);
}

void custom_fatload_show_end(void)
{
	STPIO_SET_PIN(PIO_PORT(3), 2, 1);
	STPIO_SET_PIN(PIO_PORT(3), 3, 1);
	STPIO_SET_PIN(PIO_PORT(3), 4, 1);
	STPIO_SET_PIN(PIO_PORT(3), 5, 1);
	STPIO_SET_PIN(PIO_PORT(3), 6, 1);
}
#endif

