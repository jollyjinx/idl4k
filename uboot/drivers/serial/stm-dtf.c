/*
 * drivers/serial/stm-dtf.c
 *
 * Support for emulated serial console I/O using STMicroelectronics'
 * DTF (Data Transfer Format) mechanism, over a JTAG link, to
 * a remote GDB debugger on a host machine.
 *
 *  Copyright (c) 2009  STMicroelectronics Limited
 *  Sean McGoogan <Sean.McGoogan@st.com>
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

#include "common.h"


#ifdef CONFIG_STM_DTF_SERIAL


#ifdef CONFIG_SH_SCIF_SERIAL
#error Can not define CONFIG_SH_SCIF_SERIAL and CONFIG_STM_DTF_SERIAL at same time!
#endif

#ifdef CONFIG_STM_ASC_SERIAL
#error Can not define CONFIG_STM_ASC_SERIAL and CONFIG_STM_DTF_SERIAL at same time!
#endif


static int dtf_transfer(
	volatile void * const data,
	volatile int * const numBytes,
	const size_t maxPacketSize)
{
	static unsigned char dtfPacket[8+sizeof(short)] __attribute__((aligned(4)));
	unsigned short pktSize = *numBytes;

	/* Ensure our DTF packet buffer is big enough */
	if (pktSize+sizeof(short) > sizeof(dtfPacket))
	{
		*numBytes = 0; /* Error */
		return -1;
	}

	/* pack the DTF request */
	*(unsigned short *)dtfPacket = pktSize;
	memcpy((char*)dtfPacket+sizeof(short), (void*)data, pktSize);

	/*
	 * Signal to the host that an I/O request is ready. When control returns
	 * the packet will have been updated with the result of the request.
	 */
	__asm__ __volatile__ (
		"	mov	%0, r8;"
		".short	0x003b;"
		".short	0x0000;"
		"1:	mov.w	@%0, r8;"
		"	tst	r8, r8;"
		"	bt	1b"
		: : "r" (dtfPacket) : "r8", "t");

	/*
	 * Unpack the DTF response packet.
	 * Check that the packet size is valid, if not then return immediately.
	 */
	pktSize = *(unsigned short *)dtfPacket;
	if (pktSize > maxPacketSize)
	{
		*numBytes = 0; /* Error */
		return -1;
	}

	/* Extract results */
	*numBytes = pktSize;
	memcpy((void*)data, (char*)dtfPacket+sizeof(short), pktSize);

	return 0;	/* Okay */
}


/*
 * The following are used to retain results whilst
 * polling for a key-stroke in tstc() and getc().
 */
static int CachedKeyCode;	/* Key-stroke that is cached */
static int CachedKey = 0;	/* is "CachedKeyCode" valid */


/* returns TRUE if a char is available, ready to be read */
extern int serial_tstc (void)
{
	volatile unsigned char buff[8] __attribute__((aligned(4))) = {24};
	int buffLen = 1;

	/* have we already got one key-stroke cached ? */
	if (CachedKey)
	{
		return 1;	/* cached key ready to be read */
	}

	/* Send request to and receive reply from host */
	if (dtf_transfer(buff, &buffLen, sizeof(buff)) == -1)
	{
		return 0;	/* Error */
	}

	/* Extract result */
	CachedKey     = *(volatile int*)&buff[0];
	CachedKeyCode = *(volatile unsigned int*)&buff[4];

	/* return TRUE if a char is available, else FALSE */
	return CachedKey;
}


/* blocking function, that returns next char */
extern int serial_getc (void)
{
	/* have we already got one key-stroke cached ? */
	if (CachedKey)
	{
		CachedKey = 0;		/* empty the cache */
		return CachedKeyCode;	/* cached key ready to be read */
	}

	/* polling wait: for a char to be read */
	while (!serial_tstc ());

	/* return the key just read */
	CachedKey = 0;			/* empty the cache */
	return CachedKeyCode;		/* return it */
}


/* write out a single char */
extern void serial_putc (const char ch)
{
	unsigned char buff[6] = { 7, 1, 0, 0, 0 };
	int buffLen = sizeof(buff);

	/* put the char to write in buffer */
	buff[5] = ch;

	/* Send DTF request to the host */
	dtf_transfer (buff, &buffLen, sizeof(buff));
}


/* write an entire (NUL-terminated) string */
extern void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

/* initialize the serial device */
extern int serial_init (void)
{
	/* do nothing */
	return 0;
}


/* called to adjust baud-rate */
extern void serial_setbrg (void)
{
	/* do nothing */
}


#endif	/* CONFIG_STM_DTF_SERIAL */
