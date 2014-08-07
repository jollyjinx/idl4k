 /*
  * include/asm-sh/io_stb1eval.h
  *
  * Copyright 2001 Stuart Menefy (stuart.menefy@st.com)
  *
  * May be copied or modified under the terms of the GNU General Public
  * License.  See linux/COPYING for more information.
  *
  * IO functions for the ST40STB1 Eval board
  */

#ifndef _ASM_SH_IO_STB1EVAL_H
#define _ASM_SH_IO_STB1EVAL_H

#include <asm/io_generic.h>

extern unsigned long stb1eval_isa_port2addr (unsigned long offset);

#ifdef __WANT_IO_DEF
# define __inb			generic_inb
# define __inw			generic_inw
# define __inl			generic_inl
# define __outb			generic_outb
# define __outw			generic_outw
# define __outl			generic_outl

# define __inb_p		generic_inb_p
# define __inw_p		generic_inw
# define __inl_p		generic_inl
# define __outb_p		generic_outb_p
# define __outw_p		generic_outw
# define __outl_p		generic_outl

# define __insb			generic_insb
# define __insw			generic_insw
# define __insl			generic_insl
# define __outsb		generic_outsb
# define __outsw		generic_outsw
# define __outsl		generic_outsl

# define __readb		generic_readb
# define __readw		generic_readw
# define __readl		generic_readl
# define __writeb		generic_writeb
# define __writew		generic_writew
# define __writel		generic_writel

# define __isa_port2addr	stb1eval_isa_port2addr
# define __ioremap		generic_ioremap
# define __iounmap		generic_iounmap
#endif

static __inline__ unsigned long p4_inb (unsigned long addr)
{
	return *(volatile unsigned char *) addr;
}

static __inline__ unsigned long p4_inw (unsigned long addr)
{
	return *(volatile unsigned short *) addr;
}

static __inline__ unsigned long p4_inl (unsigned long addr)
{
	return *(volatile unsigned long *) addr;
}

static __inline__ void p4_outb (unsigned long addr, unsigned short b)
{
	*(volatile unsigned char *) addr = b;
}

static __inline__ void p4_outw (unsigned long addr, unsigned short b)
{
	*(volatile unsigned short *) addr = b;
}

static __inline__ void p4_outl (unsigned long addr, unsigned int b)
{
	*(volatile unsigned long *) addr = b;
}

#define p4_in(addr)	*(addr)
#define p4_out(addr,data) *(addr) = (data)

#define p2_inl	p4_inl
#define p2_outl	p4_outl

#endif /* _ASM_SH_IO_STB1EVAL_H */
