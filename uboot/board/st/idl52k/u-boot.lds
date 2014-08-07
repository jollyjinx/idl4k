/*
 * (C) Copyright 2007 STMicroelectronics.
 *
 * Stuart Menefy <stuart.menefy@st.com>
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
        FILL(0xFFFFFFFF)
        __text_start = .;
        cpu/sh/start.o (.text.init)
		*(.text.init)		/* pre-initialization code */
		. = ALIGN(32);		/* align to a cache-line */
		*(.data.init)		/* pre-initialization data */
        board/st/idl52k/libidl52k.a (.text)
        cpu/sh/libsh.a (.text)
        lib_sh/libsh.a (.text)
        net/libnet.a (.text)
        lib_generic/libgeneric.a (.text)
        cpu/sh/stx7105/libstx7105.a (.text)
        drivers/mtd/libmtd.a (.text)
        drivers/mtd/nand/libnand.a (.text)
        drivers/net/libnet.a (.text)
        drivers/serial/libserial.a (.text)
        lib_fta/libfta.a (.text)
        fs/cramfs/libcramfs.a (.text)
        fs/fat/libfat.a (.text)
        fs/jffs2/libjffs2.a (.text)
        fs/ext2/libext2fs.a (.text)
        disk/libdisk.a (.text)
        drivers/usb/libusb.a (.text)
        /opt/STM/STLinux-2.3/devkit/sh4/lib/gcc/sh4-linux/4.2.4/libgcc.a (.text)
        . = __text_start + 0x1FC00;
        *(.keys)
        . = __text_start + 0x20000;
        *(.text)
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
