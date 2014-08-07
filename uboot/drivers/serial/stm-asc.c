/*
 * drivers/serial/stm-asc.c
 *
 * Support for Serial I/O using STMicroelectronics' on-chip ASC.
 *
 *  Copyright (c) 2004,2008  STMicroelectronics Limited
 *  Sean McGoogan <Sean.McGoogan@st.com>
 *  Copyright (C) 1999  Takeshi Yaegachi & Niibe Yutaka
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file "COPYING.LIB" in the main
 * directory of this archive for more details.
 *
 */

#include "common.h"

#ifdef CONFIG_STM_ASC_SERIAL

#include "asm/termbits.h"
#include "asm/io.h"
#include "asm/pio.h"
#include "asm/socregs.h"
#include "asm/clk.h"

#define CS7		0000040
#define CS8		0000060
#define CSIZE		0000060
#define CSTOPB		0000100
#define CREAD		0000200
#define PARENB		0000400
#define PARODD		0001000
#define HUPCL		0002000
#define CLOCAL		0004000

#define BAUDMODE	0x00001000
#define CTSENABLE	0x00000800
#define RXENABLE	0x00000100
#define RUN		0x00000080
#define STOPBIT		0x00000008
#define MODE		0x00000001
#define MODE_7BIT_PAR	0x0003
#define MODE_8BIT_PAR	0x0007
#define MODE_8BIT	0x0001
#define STOP_1BIT	0x0008
#define PARITYODD	0x0020

#define STA_NKD		0x0400
#define STA_TF		0x0200
#define STA_RHF		0x0100
#define STA_TOI		0x0080
#define STA_TNE		0x0040
#define STA_OE		0x0020
#define STA_FE		0x0010
#define STA_PE		0x0008
#define	STA_THE		0x0004
#define STA_TE		0x0002
#define STA_RBF		0x0001


#define UART_BAUDRATE_OFFSET	0x00
#define UART_TXBUFFER_OFFSET	0x04
#define UART_RXBUFFER_OFFSET	0x08
#define UART_CONTROL_OFFSET	0x0C
#define UART_INTENABLE_OFFSET	0x10
#define UART_STATUS_OFFSET	0x14
#define UART_GUARDTIME_OFFSET	0x18
#define UART_TIMEOUT_OFFSET	0x1C
#define UART_TXRESET_OFFSET	0x20
#define UART_RXRESET_OFFSET	0x24
#define UART_RETRIES_OFFSET	0x28

#define UART_BAUDRATE_REG	(CFG_STM_ASC_BASE + UART_BAUDRATE_OFFSET)
#define UART_TXBUFFER_REG	(CFG_STM_ASC_BASE + UART_TXBUFFER_OFFSET)
#define UART_RXBUFFER_REG	(CFG_STM_ASC_BASE + UART_RXBUFFER_OFFSET)
#define UART_CONTROL_REG	(CFG_STM_ASC_BASE + UART_CONTROL_OFFSET)
#define UART_INTENABLE_REG	(CFG_STM_ASC_BASE + UART_INTENABLE_OFFSET)
#define UART_STATUS_REG		(CFG_STM_ASC_BASE + UART_STATUS_OFFSET)
#define UART_GUARDTIME_REG	(CFG_STM_ASC_BASE + UART_GUARDTIME_OFFSET)
#define UART_TIMEOUT_REG	(CFG_STM_ASC_BASE + UART_TIMEOUT_OFFSET)
#define UART_TXRESET_REG	(CFG_STM_ASC_BASE + UART_TXRESET_OFFSET)
#define UART_RXRESET_REG	(CFG_STM_ASC_BASE + UART_RXRESET_OFFSET)
#define UART_RETRIES_REG	(CFG_STM_ASC_BASE + UART_RETRIES_OFFSET)


/*---- Values for the BAUDRATE Register -----------------------*/

#if defined(__SH4__)
#define PCLK			(get_peripheral_clk_rate())
#define BAUDRATE_VAL_M0(bps)	(PCLK / (16 * (bps)))
#define BAUDRATE_VAL_M1(bps)	((((bps * (1 << 14))+ (1<<13)) / (PCLK/(1 << 6))))
#else	/* !defined(__SH4__) */
#define PCLK			B_CLOCK_RATE
#define BAUDRATE_VAL_M0(bps)	(PCLK / (16 * (bps)))
#define BAUDRATE_VAL_M1(bps)	(int)((((double)bps * (1 << 20))/ PCLK)+0.5)
#endif	/* defined(__SH4__) */

/*
 * MODE 0
 *                       ICCLK
 * ASCBaudRate =   ----------------
 *                   baudrate * 16
 *
 * MODE 1
 *                   baudrate * 16 * 2^16
 * ASCBaudRate =   ------------------------
 *                          ICCLK
 *
 * NOTE:
 * Mode 1 should be used for baudrates of 19200, and above, as it
 * has a lower deviation error than Mode 0 for higher frequencies.
 * Mode 0 should be used for all baudrates below 19200.
 */


#if defined(CONFIG_STM_ST231)
#define p2_outl(addr,b) writel(b,addr)
#define p2_inl(addr)    readl(addr)
#endif


#ifdef CONFIG_HWFLOW
static int hwflow = 0;		/* turned off by default */
#endif	/* CONFIG_HWFLOW */


/* busy wait until it is safe to send a char */
static inline void TxCharReady (void)
{
	unsigned long status;

	do {
		status = p2_inl (UART_STATUS_REG);
	} while (status & STA_TF);
}

/* initialize the ASC */
extern int serial_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	const int cflag = CREAD | HUPCL | CLOCAL | CSTOPB | CS8 | PARODD;
	unsigned long val;
	int baud = gd->baudrate;
	int t, mode=1;

	switch (baud) {
#if 0
	case 0:
		t = -1;
		break;
	case 2400:
		t = BAUDRATE_VAL_M0(2400);
		mode = 0;
		break;
	case 4800:
		t = BAUDRATE_VAL_M0(4800);
		mode = 0;
		break;
#endif
	case 9600:
		t = BAUDRATE_VAL_M0(9600);
		mode = 0;
		break;
	case 19200:
		t = BAUDRATE_VAL_M1(19200);
		break;
	case 38400:
		t = BAUDRATE_VAL_M1(38400);
		break;
	case 57600:
		t = BAUDRATE_VAL_M1(57600);
		break;
	default:
		printf ("ASC: unsupported baud rate: %d, using 115200 instead.\n", baud);
	case 115200:
		t = BAUDRATE_VAL_M1(115200);
		break;
	}

	/* wait for end of current transmission */
	TxCharReady ();

	/* disable the baudrate generator */
	val = p2_inl (UART_CONTROL_REG);
	p2_outl (UART_CONTROL_REG, (val & ~RUN));

	/* set baud generator reload value */
	p2_outl (UART_BAUDRATE_REG, t);

	/* reset the RX & TX buffers */
	p2_outl (UART_TXRESET_REG, 1);
	p2_outl (UART_RXRESET_REG, 1);

	/* build up the value to be written to CONTROL */
	val = RXENABLE | RUN;

	/* set character length */
	if ((cflag & CSIZE) == CS7)
		val |= MODE_7BIT_PAR;
	else {
		if (cflag & PARENB)
			val |= MODE_8BIT_PAR;
		else
			val |= MODE_8BIT;
	}

	/* set stop bit */
	/* it seems no '0 stop bits' option is available: by default
	 * we get 0.5 stop bits */
	if (cflag & CSTOPB)
		val |= STOP_1BIT;

	/* odd parity */
	if (cflag & PARODD)
		val |= PARITYODD;

#ifdef CONFIG_HWFLOW
	/*  set flow control */
	if (hwflow)
		val |= CTSENABLE;
#endif	/* CONFIG_HWFLOW */

	/* set baud generator mode */
	if (mode)
		val |= BAUDMODE;

	/* finally, write value and enable ASC */
	p2_outl (UART_CONTROL_REG, val);
	return 0;
}

/* returns TRUE if a char is available, ready to be read */
extern int serial_tstc (void)
{
	unsigned long status;

	status = p2_inl (UART_STATUS_REG);
	return (status & STA_RBF);
}

/* blocking function, that returns next char */
extern int serial_getc (void)
{
	char ch;

	/* polling wait: for a char to be read */
	while (!serial_tstc ());

	/* read char, now that we know we have one */
	ch = p2_inl (UART_RXBUFFER_REG);

	/* return consumed char to the caller */
	return ch;
}

/* write write out a single char */
extern void serial_putc (char ch)
{
	/* Stream-LF to CR+LF conversion */
	if (ch == 10)
		serial_putc ('\r');

	/* wait till safe to write next char */
	TxCharReady ();

	/* finally, write next char */
	p2_outl (UART_TXBUFFER_REG, ch);
}

/* write an entire (NUL-terminated) string */
extern void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

/* called to adjust baud-rate */
extern void serial_setbrg (void)
{
	/* just re-initialize ASC */
	serial_init ();
}

#ifdef CONFIG_HWFLOW
extern int hwflow_onoff (int on)
{
	switch (on) {
	case 0:
	default:
		break;		/* return current */
	case 1:
		hwflow = 1;	/* turn on */
		serial_init ();
		break;
	case -1:
		hwflow = 0;	/* turn off */
		serial_init ();
		break;
	}
	return hwflow;
}
#endif	/* CONFIG_HWFLOW */

#endif	/* CONFIG_STM_ASC_SERIAL */
