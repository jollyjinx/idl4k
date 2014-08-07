/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2009 STMicroelectronics.
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

#ifndef _SCONSOLE_H_
#define _SCONSOLE_H_

#include <config.h>


/*
 * set the address and size of the SCONSOLE_BUFFER,
 * if not explicitly defined in the config.h file.
 */
#ifndef CFG_SCONSOLE_ADDR
#define CFG_SCONSOLE_ADDR		CFG_SDRAM_BASE
#endif	/* CFG_SCONSOLE_ADDR */
#ifndef CFG_SCONSOLE_SIZE
#define CFG_SCONSOLE_SIZE		0x2000	/* 8 KiB */
#endif	/* CFG_SCONSOLE_SIZE */


typedef struct sconsole_buffer_s
{
	unsigned long size;
	unsigned long max_size;
	unsigned long pos;
	char data[1];
} sconsole_buffer_t;

#define SCONSOLE_BUFFER		((sconsole_buffer_t *) CFG_SCONSOLE_ADDR)

extern void (*sconsole_putc) (char);
extern void (*sconsole_puts) (const char *);
extern int (*sconsole_getc) (void);
extern int (*sconsole_tstc) (void);
extern void (*sconsole_setbrg) (void);

extern void sconsole_flush (void);

#endif
