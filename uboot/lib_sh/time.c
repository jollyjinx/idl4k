/*
 * (C) Copyright 2009
 * Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 *
 * (C) Copyright 2004,2009 STMicroelectronics.
 * Andy Sturges <andy.sturges@st.com>
 * Sean McGoogan <Sean.McGoogan@st.com>
 *
 * (C) Copyright 2007-2008
 * Nobobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <common.h>
#include <asm/socregs.h>
#include <div64.h>
#include <asm/processor.h>
#include <asm/clk.h>
#include <asm/io.h>

#define TMU_MAX_COUNTER (~0UL)

#if !defined(TMU_CLK_DIVIDER)
#define TMU_CLK_DIVIDER		4	/* Clock divided by 4 is FASTEST */
#endif	/* TMU_CLK_DIVIDER */

static ulong timer_freq;

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CFG_HZ;
	do_div(tick, timer_freq);

	return tick;
}

static inline unsigned long long usec_to_tick(unsigned long long usec)
{
	usec *= timer_freq;
	do_div(usec, 1000000);

	return usec;
}

static void tmu_timer_start (unsigned int timer)
{
	if (timer > 2)
		return;
	writeb(readb(TSTR) | (1 << timer), TSTR);
}

static void tmu_timer_stop (unsigned int timer)
{
	if (timer > 2)
		return;
	writeb(readb(TSTR) & ~(1 << timer), TSTR);
}

int timer_init (void)
{
	/* Divide clock by TMU_CLK_DIVIDER */
	u16 bit = 0;

	switch (TMU_CLK_DIVIDER) {
	case 1024:
		bit = 4;
		break;
	case 256:
		bit = 3;
		break;
	case 64:
		bit = 2;
		break;
	case 16:
		bit = 1;
		break;
	case 4:
	default:
		break;
	}
	writew(readw(TCR0) | bit, TCR0);

	/* Clock frequency calc */
	timer_freq = get_tmu0_clk_rate() >> ((bit + 1) * 2);

	tmu_timer_stop(0);
	tmu_timer_start(0);

	return 0;
}

unsigned long long get_ticks (void)
{
	return 0 - readl(TCNT0);
}

static inline void tick_delay (const unsigned long long delta)
{
	const u64 start = get_ticks();		/* get timestamp on entry */
	u64 end = start + delta;		/* calculate end timestamp */

	if (delta == 0)				/* zero delay ? */
		end++;				/* minimum of ONE tick */

	if (end > TMU_MAX_COUNTER)		/* overflows 32-bits ? */
	{
		while (get_ticks() >= start)	/* loop till overflowed */
			/*NOP*/;
		end &= TMU_MAX_COUNTER;		/* mask off upper 32-bits */
		while (get_ticks() < end)	/* loop till event */
			/*NOP*/;
	}
	else					/* no overflow */
	{
		while ( (get_ticks() >= start) && (get_ticks() < end) )
			/*NOP*/;
	}
}

void udelay (unsigned long usec)		/* delay in micro-seconds */
{
	u64 delta = usec_to_tick(usec);		/* time to wait */

	tick_delay (delta);			/* wait ... */
}

void ndelay (unsigned long nsec)		/* delay in nano-seconds */
{
	u64 delta = usec_to_tick(nsec);		/* time to wait */

	delta /= 1000ull;			/* remember: nsec not usec ! */

	tick_delay (delta);			/* wait ... */
}

unsigned long get_timer (unsigned long base)
{
	const unsigned long now = (unsigned long)tick_to_time(get_ticks());
	unsigned long result;

	if (now < base)		/* have we over-flowed ? */
	{
		result = (unsigned long)tick_to_time(TMU_MAX_COUNTER) - base;
		result += now + 1u;
	}
	else			/* we have not over-flowed */
	{
		result = now - base;
	}

	/* return delta (in msec) from 'now' w.r.t. 'base' */
	return result;
}

void set_timer (unsigned long t)
{
	/* Note: timer must be STOPPED to update it */
	tmu_timer_stop(0);
	writel((0 - t), TCNT0);
	tmu_timer_start(0);
}

void reset_timer (void)
{
	set_timer (0);
}

unsigned long get_tbclk (void)
{
	return timer_freq;
}
