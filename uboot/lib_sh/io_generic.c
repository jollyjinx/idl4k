/*
 * lib_sh/io_generic.c
 *
 * Copyright (C) 2000  Niibe Yutaka
 *
 * Generic I/O routine. These can be used where a machine specific version
 * is not required.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 */

#include <asm/io.h>

void _insb (unsigned long port, void *buffer, unsigned long count)
{
	unsigned char *buf = buffer;
	while (count--)
		*buf++ = inb (port);
}

void _insw (unsigned long port, void *buffer, unsigned long count)
{
	unsigned short *buf = buffer;
	while (count--)
		*buf++ = inw (port);
}

void _insl (unsigned long port, void *buffer, unsigned long count)
{
	unsigned long *buf = buffer;
	while (count--)
		*buf++ = inl (port);
}

void _outsb (unsigned long port, const void *buffer,
		    unsigned long count)
{
	const unsigned char *buf = buffer;
	while (count--)
		outb (*buf++, port);
}

void _outsw (unsigned long port, const void *buffer,
		    unsigned long count)
{
	const unsigned short *buf = buffer;
	while (count--)
		outw (*buf++, port);
}

void _outsl (unsigned long port, const void *buffer,
		    unsigned long count)
{
	const unsigned long *buf = buffer;
	while (count--)
		outl (*buf++, port);
}

