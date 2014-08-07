/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2009,2010 STMicroelectronics Ltd.
 * Sean McGoogan <Sean.McGoogan@st.com>

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

/*
 * NOTE: For the ST40 series of SoCs, we use the "eeprom" set of
 * commands to access SPI serial flash memory devices.
 *
 * Hence we need to define CFG_ENV_IS_IN_EEPROM is we want the
 * environment stored in SPI serial flash.
 *
 * This file allows us to store U-boot's environment in SPI
 * serial flash, and boot from SPI using the SPIBOOT mode
 * controller in the STMicroelectronics' EMI.
 *
 * Sometimes (during initialization) we access the SPI via the
 * SPIBOOT mode controller, other times (post-initialization)
 * we access the SPI serial flash via the SSC (or PIOs).
 * This file tries to take care of dealing with the
 * duality aspects of all this.
 *
 * If CFG_BOOT_FROM_SPI is defined, then we assume we are booting
 * from SPI serial flash, using the SoC's SPIBOOT mode, to re-map
 * the serial flash to 0xA0000000 in the EMI space. Otherwise,
 * we assume we are *not* booting using the SoC's SPIBOOT mode,
 * in which case we need to use either the SSC or PIO to talk
 * to the SPI serial flash.
 * The main difference in usage, is the environment variables
 * will always be honoured if CFG_BOOT_FROM_SPI is defined,
 * whereas, if CFG_BOOT_FROM_SPI is not defined, then
 * the environment variables will only be honoured *after* the
 * SPI has been initialized (i.e. after serial initialization).
 * Hence, "baudrate" may not be honoured if the environment is
 * stored in SPI, unless CFG_BOOT_FROM_SPI is also defined.
 */

#if defined(CFG_ENV_IS_IN_EEPROM)

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>

DECLARE_GLOBAL_DATA_PTR;

env_t *env_ptr = NULL;

char * env_name_spec = "SPI Serial Flash";

extern uchar (*env_get_char)(int);
extern uchar env_get_char_memory (int index);


#if defined(CFG_BOOT_FROM_SPI)
	/*
	 * The following function will read a 32-bit value
	 * from the serial flash, via the SPIBOOT controller,
	 * whilst in SPI Boot mode.
	 */
static inline u32 spiboot_get_u32(const int index)
{
	return *(u32*)(CFG_EMI_SPI_BASE + index);
}


	/*
	 * The following function will read a byte
	 * from the serial flash, via the SPIBOOT
	 * controller, whilst in SPI boot mode.
	 * NOTE: we *need* to perform a 4-byte read, and only
	 * extract the one byte we are really interested in.
	 */
static inline uchar spiboot_get_byte(const int index)
{
	const u32 word = spiboot_get_u32(index & ~0x3u);
	return ( ( word >> (index%4)*8 ) & 0xffu );
}


/************************************************************************
 * Initialize Environment use
 *
 * Note: as we are using SPIBOOT mode to read the serial flash,
 * then we need to perform 32-bit accesses only. Hence, we loop
 * whilst reading 4-bytes into 'buf' and then update the CRC.
 */
extern int env_init(void)
{
	ulong crc, len, new;
	unsigned off;
	u32 *buf;		/* only do 32-bits per iteration */

	/* read old CRC (from flash) */
	crc = spiboot_get_u32(CFG_ENV_OFFSET + offsetof(env_t,crc));

	/* compute current CRC */
	new = 0;
	len = ENV_SIZE;
	off = CFG_ENV_OFFSET + offsetof(env_t,data);
	while (len > 0)
	{
		int n = (len > sizeof(buf)) ? sizeof(buf) : len;
		buf = (u32*)spiboot_get_u32(off);
		new = crc32 (new, (uchar*)&buf, n);
		len -= n;
		off += n;
	}

	/* is the flash environment good ? */
	if (crc == new)
	{
		gd->env_addr  = CFG_ENV_OFFSET + offsetof(env_t,data);
		gd->env_valid = 1;
	}
	else
	{
		gd->env_addr  = 0;
		gd->env_valid = 0;
	}

	return (0);
}
#endif	/* CFG_BOOT_FROM_SPI */


/************************************************************************
 * Initialize Environment use
 *
 * Note: for this case, we are storing the environment
 * in SPI serial flash - but we are *not* booting from SPI
 * serial flash, so we can not obtain the proper environment
 * until we have probed for the SPI serial flash - which we do later.
 * So, we initially just "fake" the environment by pretending
 * that there is not one!
 */
#if !defined(CFG_BOOT_FROM_SPI)
extern int env_init(void)
{
	gd->env_addr  = 0;
	gd->env_valid = 0;	/* no valid environment */

	return (0);
}

/*
 * The following function is performed to initialize
 * the environment *after* the SPI driver has been
 * initialized.
 */
extern int env_init_after_spi_done(void)
{
	ulong crc, len, new;
	unsigned off;
	uchar buf[64];

	/* read old CRC */
	eeprom_read (CFG_DEF_EEPROM_ADDR,
		     CFG_ENV_OFFSET+offsetof(env_t,crc),
		     (uchar *)&crc, sizeof(ulong));

	new = 0;
	len = ENV_SIZE;
	off = offsetof(env_t,data);
	while (len > 0) {
		int n = (len > sizeof(buf)) ? sizeof(buf) : len;

		eeprom_read (CFG_DEF_EEPROM_ADDR, CFG_ENV_OFFSET+off, buf, n);
		new = crc32 (new, buf, n);
		len -= n;
		off += n;
	}

	if (crc == new) {
		gd->env_addr  = offsetof(env_t,data);
		gd->env_valid = 1;
	} else {
		gd->env_addr  = 0;
		gd->env_valid = 0;
	}

	return (0);
}
#endif	/* !CFG_BOOT_FROM_SPI */


extern uchar env_get_char_spec (int index)
{
	uchar c;

#if defined(CFG_BOOT_FROM_SPI)
	if ( (env_ptr==NULL) ||
	     (gd->env_addr != (ulong)&(env_ptr->data)) )
	{
		/*
		 * Not relocated the environment yet.
		 * So the SPI is not yet initialized, so use the SPI-BOOT
		 * controller to read the environment directly via the EMI.
		 */
		c = spiboot_get_byte(CFG_ENV_OFFSET + index);
	}
	else
#endif	/* CFG_BOOT_FROM_SPI */
	{
		/*
		 * Serial flash accessible via SSC or PIO, so
		 * we can need to use the 'normal' SPI interfaces
		 * to talk to the SPI serial flash device.
		 */
		eeprom_read (CFG_DEF_EEPROM_ADDR,
			     CFG_ENV_OFFSET+index+offsetof(env_t,data),
			     &c, 1);
	}

	return (c);
}


extern void env_relocate_spec (void)
{
	eeprom_read (
		CFG_DEF_EEPROM_ADDR,
		CFG_ENV_OFFSET,
		(uchar *)env_ptr,
		CFG_ENV_SIZE);
}


extern int saveenv(void)
{
	return eeprom_write (CFG_DEF_EEPROM_ADDR,
			     CFG_ENV_OFFSET,
			     (uchar *)env_ptr,
			     CFG_ENV_SIZE);
}


#endif	/* CFG_ENV_IS_IN_EEPROM */


