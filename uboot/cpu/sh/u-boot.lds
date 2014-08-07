/*
 * (C) Copyright 2007-2009 STMicroelectronics.
 *
 * Stuart Menefy <stuart.menefy@st.com>
 * Sean McGoogan <Sean.McGoogan@st.com>
 *
 * (C) Copyright 2004 STMicroelectronics.
 *
 * Andy Sturges <andy.sturges@st.com>
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

OUTPUT_ARCH(sh4)
ENTRY(_start)

SECTIONS
{
	.text ALIGN(0x4):
	{
		__text_start = .;
		cpu/sh/start.o (.text.init)
		*(.text.init)		/* pre-initialization code */
		. = ALIGN(32);		/* align to a cache-line */
		*(.data.init)		/* pre-initialization data */
		*(.text)		/* post-initialization code */
		__text_end = .;
	}

	.rodata ALIGN(0x4):
	{
		__rodata_start = .;
		*(.rodata)
		__rodata_end = .;
	}

	.data ALIGN(0x4):
	{
		__data_start = .;
		*(.data)
		__data_end = .;
	}

	.u_boot_cmd ALIGN(0x4):
	{
		__u_boot_cmd_start = .;
		*(.u_boot_cmd)
		__u_boot_cmd_end = .;
	}

	_uboot_end_data = .;

	.bss ALIGN(0x4):
	{
		__bss_start = .;
		*(.bss)
		*(COMMON)		/* ensure common data is initialized */
		__bss_end = .;
	}
	_uboot_end = .;
}
