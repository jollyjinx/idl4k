/*
 *  drivers/sh-sci.c
 *
 *  SuperH on-chip serial module support.  (SCI with no FIFO / with FIFO)
 *  Copyright (C) 1999, 2000  Niibe Yutaka
 *  Copyright (C) 2000  Sugioka Toshinobu
 *  Modified to support multiple serial ports. Stuart Menefy (May 2000).
 *  Modified to support SecureEdge. David McCullough (2002)
 *  Modified to support SH7300 SCIF. Takashi Kusuda (Jun 2003).
 *  Modified for u-boot Andy Sturges (Nov 2004)
 *
 * TTY code is based on sx.c (Specialix SX driver) by:
 *
 *   (C) 1998 R.E.Wolff@BitWizard.nl
 *
 */

#include "common.h"

#ifdef CONFIG_SH_SCIF_SERIAL

#include "asm/termbits.h"
#include "asm/io.h"
#include "sh-sci.h"

static void sci_init_pins_scif (struct sci_port *port, unsigned int cflag);
static struct sci_port sci_ports[SCI_NPORTS] = SCI_INIT;

static void put_char (struct sci_port *port, char c)
{
	unsigned short status;

	do
		status = sci_in (port, SCxSR);
	while (!(status & SCxSR_TDxE (port)));

	sci_out (port, SCxTDR, c);
	sci_in (port, SCxSR);	/* Dummy read */
	sci_out (port, SCxSR, SCxSR_TDxE_CLEAR (port));

}

static void handle_error (struct sci_port *port)
{				/* Clear error flags */
	sci_out (port, SCxSR, SCxSR_ERROR_CLEAR (port));
}

static int get_char (struct sci_port *port)
{
	unsigned short status;
	int c;

	do {
		status = sci_in (port, SCxSR);
		if (status & SCxSR_ERRORS (port)) {
			handle_error (port);
			continue;
		}
	} while (!(status & SCxSR_RDxF (port)));
	c = sci_in (port, SCxRDR);
	sci_in (port, SCxSR);	/* Dummy read */
	sci_out (port, SCxSR, SCxSR_RDxF_CLEAR (port));

	return c;
}

#if 0
static void put_string (struct sci_port *port, const char *buffer, int count)
{
	int i;
	const unsigned char *p = buffer;

	for (i = 0; i < count; i++) {
		if (*p == 10)
			put_char (port, '\r');
		put_char (port, *p++);
	}
}
#endif

static int is_char_ready (struct sci_port *port)
{
	unsigned short status = sci_in (port, SCxSR);

	if (status & (SCxSR_ERRORS (port) | SCxSR_BRK (port)))
		handle_error (port);

	return (status & SCxSR_RDxF (port));
}

static void sci_init_pins_scif (struct sci_port *port, unsigned int cflag)
{
	unsigned int fcr_val = 0;

	if (cflag & CRTSCTS) {
		fcr_val |= SCFCR_MCE;
	} else {
		sci_out (port, SCSPTR, 0x0080);	/* Set RTS = 1 */
	}
	sci_out (port, SCFCR, fcr_val);
}

static void sci_setsignals (struct sci_port *port, int dtr, int rts)
{
	/* This routine is used for seting signals of: DTR, DCD, CTS/RTS */
	/* We use SCIF's hardware for CTS/RTS, so don't need any for that. */
	/* If you have signals for DTR and DCD, please implement here. */
}

static void sci_set_baud (struct sci_port *port, int baud)
{
	int t;
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	switch (baud) {
	case 0:
		t = -1;
		break;
	case 2400:
		t = BPS_2400;
		break;
	case 4800:
		t = BPS_4800;
		break;
	case 9600:
		t = BPS_9600;
		break;
	case 19200:
		t = BPS_19200;
		break;
	case 38400:
		t = BPS_38400;
		break;
	case 57600:
		t = BPS_57600;
		break;
	case 230400:
		if (BPS_230400 != BPS_115200) {
			t = BPS_230400;
			break;
		}
	default:
		printf ("sci: unsupported baud rate: %d, using 115200 instead.\n", baud);
	case 115200:
		t = BPS_115200;
		break;
	}

	if (t > 0) {
		sci_setsignals (port, 1, -1);
		if (t >= 256) {
			sci_out (port, SCSMR,
				 (sci_in (port, SCSMR) & ~3) | 1);
			t >>= 2;
		} else {
			sci_out (port, SCSMR, sci_in (port, SCSMR) & ~3);
		}
		sci_out (port, SCBRR, t);
		udelay ((1000000 + (baud - 1)) / baud);	/* Wait one bit interval */
	} else {
		sci_setsignals (port, 0, -1);
	}
}

static void sci_set_termios_cflag (struct sci_port *port, int cflag, int baud)
{
	unsigned int status;
	unsigned int smr_val;

	do
		status = sci_in (port, SCxSR);
	while (!(status & SCxSR_TEND (port)));

	sci_out (port, SCSCR, 0x00);	/* TE=0, RE=0, CKE1=0 */

	if (port->type == PORT_SCIF) {
		sci_out (port, SCFCR, SCFCR_RFRST | SCFCR_TFRST);
	}

	smr_val = sci_in (port, SCSMR) & 3;
	if ((cflag & CSIZE) == CS7)
		smr_val |= 0x40;
	if (cflag & PARENB)
		smr_val |= 0x20;
	if (cflag & PARODD)
		smr_val |= 0x30;
	if (cflag & CSTOPB)
		smr_val |= 0x08;
	sci_out (port, SCSMR, smr_val);
	sci_set_baud (port, baud);

	port->init_pins (port, cflag);
	sci_out (port, SCSCR, SCSCR_INIT (port));
}

int serial_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	int cflags = CS8 | CREAD | HUPCL | CLOCAL | CRTSCTS;
	sci_set_termios_cflag (&sci_ports[CONFIG_CONS_INDEX], cflags,
			       gd->baudrate);
	return (0);
}

void serial_putc (const char c)
{
	if (c == 10)
		put_char (&sci_ports[CONFIG_CONS_INDEX], '\r');
	put_char (&sci_ports[CONFIG_CONS_INDEX], c);
}

void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int serial_getc (void)
{
	return get_char (&sci_ports[CONFIG_CONS_INDEX]);
}

int serial_tstc (void)
{
	return is_char_ready (&sci_ports[CONFIG_CONS_INDEX]);
}

void serial_setbrg (void)
{
	/* */
}

#endif /* CONFIG_SH_SCIF_SERIAL */
