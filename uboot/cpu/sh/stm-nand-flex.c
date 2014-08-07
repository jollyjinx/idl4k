/*
 * (C) Copyright 2008-2011 STMicroelectronics, Sean McGoogan <Sean.McGoogan@st.com>
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
#include <malloc.h>

#if defined(CONFIG_CMD_NAND) && defined(CFG_NAND_FLEX_MODE)

#include <nand.h>
#include <asm/stm-nand.h>
#include <asm/ecc.h>
#include <asm/errno.h>
#include <asm/st40reg.h>
#include <asm/io.h>
#include <asm/socregs.h>
#include <asm/cache.h>


#define DEBUG_FLEX		0	/* Enable additional debugging of the FLEX controller */


/* Flex-Mode Data {Read,Write} Config Registers & Flex-Mode {Command,Address} Registers */
#define FLEX_WAIT_RBn			( 1u << 27 )	/* wait for RBn to be asserted (i.e. ready) */
#define FLEX_BEAT_COUNT_1		( 1u << 28 )	/* One Beat */
#define FLEX_BEAT_COUNT_2		( 2u << 28 )	/* Two Beats */
#define FLEX_BEAT_COUNT_3		( 3u << 28 )	/* Three Beats */
#define FLEX_BEAT_COUNT_4		( 0u << 28 )	/* Four Beats */
#define FLEX_CSn_STATUS			( 1u << 31 )	/* Deasserts CSn after current operation completes */

/* Flex-Mode Data-{Read,Write} Config Registers */
#define FLEX_1_BYTE_PER_BEAT		( 0u << 30 )	/* One Byte per Beat */
#define FLEX_2_BYTES_PER_BEAT		( 1u << 30 )	/* Two Bytes per Beat */

/* Flex-Mode Configuration Register */
#define FLEX_CFG_ENABLE_FLEX_MODE	( 1u <<  0 )	/* Enable Flex-Mode operations */
#define FLEX_CFG_ENABLE_AFM		( 2u <<  0 )	/* Enable Advanced-Flex-Mode operations */
#define FLEX_CFG_SW_RESET		( 1u <<  3 )	/* Enable Software Reset */
#define FLEX_CFG_CSn_STATUS		( 1u <<  4 )	/* Deasserts CSn in current Flex bank */


enum stm_nand_flex_mode {
	flex_quiecent,		/* next byte_write is *UNEXPECTED* */
	flex_command,		/* next byte_write is a COMMAND */
	flex_address		/* next byte_write is a ADDRESS */
};


/*
 * NAND device connected to STM NAND Controller operating in FLEX mode.
 * There may be several NAND device connected to the NAND controller.
 */
struct stm_nand_flex_device {
	int			csn;
	struct nand_chip	*chip;
	struct mtd_info		*mtd;
	struct nand_timing_data *timing_data;
};


/*
 * STM NAND Controller operating in FLEX mode.
 * There is only a single one of these.
 */
static struct stm_nand_flex_controller {
	int			current_csn;	/* Currently Selected Device (CSn) */
	int			next_csn;	/* First free NAND Device (CSn) */
	enum stm_nand_flex_mode mode;
	struct stm_nand_flex_device device[CFG_MAX_NAND_DEVICE];
	uint8_t			*buf;		/* Bounce buffer for non-aligned xfers */
} flex;


/*
 * In FLEX-mode, we can either read from the Flex-mode data
 * register (ST40_EMI_NAND_FLEX_DATA), over the STBus with either
 * a 4-byte read (LD4) or a 32-byte read (LD32) bus opcode.
 * Using the LD32 bus opcode amortises the cost of the bus
 * latency over 32-bytes, instead of only 4 bytes, so one can
 * achieve a higher throughput, using the LD32 bus opcode.
 *
 * However, in order to realize this potential improvement, one
 * needs to access the ST40_EMI_NAND_FLEX_DATA via the CPU's operand
 * (data) caches. This in turn requires a TLB (or PMB) address
 * translation to be configured and enabled.
 *
 * The following macros are used to configure the TLB to map
 * the ST40_EMI_NAND_FLEX_DATA into a P3 (cachable + translatable)
 * virtual address, so we can utilize the LD32 opcode.
 *
 * The implementation will create a single (read-only) 1KiB TLB
 * mapping, including ST40_EMI_NAND_FLEX_DATA to 0xC0000000.
 */
#if defined(CONFIG_SH_NAND_USES_CACHED_READS)
#define ST40_MMU_PTEH	0xFF000000	/* Page Table Entry High register */
#define ST40_MMU_PTEL	0xFF000004	/* Page Table Entry Low register */
#define ST40_MMU_MMUCR	0xFF000010	/* MMU Control Register */

#define PTEH_ASID	0		/* ASID to use (any 8-bit number) */

#define PTEL_WT		(1ul<<0)	/* Write-through bit */
#define PTEL_SH		(1ul<<1)	/* Private page or Shared page */
#define PTEL_D		(1ul<<2)	/* Dirty bit */
#define PTEL_C		(1ul<<3)	/* Cacheability bit */
#define PTEL_SZ_1K	(0ul<<4)	/* Page size of 1 KiB */
#define PTEL_SZ_4K	(1ul<<4)	/* Page size of 4 KiB */
#define PTEL_SZ_64K	(8ul<<4)	/* Page size of 64 KiB */
#define PTEL_SZ_1M	(9ul<<4)	/* Page size of 1 MiB */
#define PTEL_PR_READ	(0ul<<5)	/* Privileged-mode can READ */
#define PTEL_PR_WRITE	(1ul<<5)	/* Privileged-mode can READ+WRITE */
#define PTEL_V		(1ul<<8)	/* Validity Bit */
#define PTEL_UB		(1ul<<9)	/* Unbuffered write control bit */

#define MMUCR_AT	(1ul<<0)	/* Address Translation bit */
#define MMUCR_TI	(1ul<<2)	/* TLB Invalidate bit */

#define PAGE_MASK	(~0x3fful)	/* 1 KiB Page */
#define ASID_MASK	(0xfful)	/* 8 bit ASID */

static volatile u32 * const mmucr_p = (u32*)ST40_MMU_MMUCR;

static volatile u32 * const cache =
	(u32*)(0xC0000000ul | ((u32)ST40_EMI_NAND_FLEX_DATA & ~PAGE_MASK));
#endif	/* CONFIG_SH_NAND_USES_CACHED_READS */


/* Configure NAND controller timing registers */
/* QQQ: to write & use this function (for performance reasons!) */
#ifdef QQQ
static void flex_set_timings(struct nand_timing_data * const tm)
{
	uint32_t n;
	uint32_t reg;
	uint32_t emi_clk;
	uint32_t emi_t_ns;

	/* Timings are set in units of EMI clock cycles */
	emi_clk = clk_get_rate(clk_get(NULL, "emi_master"));
	emi_t_ns = 1000000000UL / emi_clk;

	/* CONTROL_TIMING */
	n = (tm->sig_setup + emi_t_ns - 1u)/emi_t_ns;
	reg = (n & 0xffu) << 0;

	n = (tm->sig_hold + emi_t_ns - 1u)/emi_t_ns;
	reg |= (n & 0xffu) << 8;

	n = (tm->CE_deassert + emi_t_ns - 1u)/emi_t_ns;
	reg |= (n & 0xffu) << 16;

	n = (tm->WE_to_RBn + emi_t_ns - 1u)/emi_t_ns;
	reg |= (n & 0xffu) << 24;

#if DEBUG_FLEX
	printf("info: CONTROL_TIMING = 0x%08x\n", reg);
#endif
	*ST40_EMI_NAND_CTL_TIMING = reg;

	/* WEN_TIMING */
	n = (tm->wr_on + emi_t_ns - 1u)/emi_t_ns;
	reg = (n & 0xffu) << 0;

	n = (tm->wr_off + emi_t_ns - 1u)/emi_t_ns;
	reg |= (n & 0xffu) << 8;

#if DEBUG_FLEX
	printf("info: WEN_TIMING = 0x%08x\n", reg);
#endif
	*ST40_EMI_NAND_WEN_TIMING = reg;

	/* REN_TIMING */
	n = (tm->rd_on + emi_t_ns - 1u)/emi_t_ns;
	reg = (n & 0xffu) << 0;

	n = (tm->rd_off + emi_t_ns - 1u)/emi_t_ns;
	reg |= (n & 0xffu) << 8;

#if DEBUG_FLEX
	printf("info: REN_TIMING = 0x%08x\n", reg);
#endif
	*ST40_EMI_NAND_REN_TIMING = reg;
}
#endif


/*
 * hardware specific access to the Ready/not_Busy signal.
 * Signal is routed through the EMI NAND Controller block.
 */
static int stm_flex_device_ready(struct mtd_info * const mtd)
{
	/* Apply a small delay before sampling the RBn signal */
#if 1
	ndelay(500);	/* QQQ: do we really need this ??? */
#endif
	/* extract bit 2: status of RBn pin on the FLEX bank */
	return ((*ST40_EMI_NAND_RBN_STA) & (1ul<<2)) ? 1 : 0;
}


static void init_flex_mode(void)
{
	u_int32_t reg;

	/* Disable the BOOT-mode controller */
	*ST40_EMI_NAND_BOOTBANK_CFG = 0;

	/* Perform a S/W reset the FLEX-mode controller */
	/* need to assert it for at least one (EMI) clock cycle. */
	*ST40_EMI_NAND_FLEXMODE_CFG = FLEX_CFG_SW_RESET;
	udelay(1);	/* QQQ: can we do something shorter ??? */
	*ST40_EMI_NAND_FLEXMODE_CFG = 0;

	/* Disable all interrupts in FLEX mode */
	*ST40_EMI_NAND_INT_EN = 0;

	/* Set FLEX-mode controller to enable FLEX-mode */
	*ST40_EMI_NAND_FLEXMODE_CFG = FLEX_CFG_ENABLE_FLEX_MODE;

	/*
	 * Configure (pervading) FLEX_DATA to write 4-bytes at a time.
	 * DATA is only written by write_buf(), not write_byte().
	 * Hence, we only need to configure this once (ever)!
	 * As we may be copying directly from NOR flash to NAND flash,
	 * we need to deassert the CSn after *each* access, as we
	 * can not guarantee the buffer is in RAM (or not in the EMI).
	 * Note: we could run memcpy() in write_buf() instead.
	 */
	reg = FLEX_BEAT_COUNT_4 | FLEX_1_BYTE_PER_BEAT;
	reg |= FLEX_CSn_STATUS;		/* deassert CSn after each flex data write */
#if 0
	reg |= FLEX_WAIT_RBn;		/* QQQ: do we want this ??? */
#endif
	*ST40_EMI_NAND_FLEX_DATAWRT_CFG = reg;
}


/* FLEX mode chip select: For now we only support 1 chip per
 * 'stm_nand_flex_device' so chipnr will be 0 for select, -1 for deselect.
 *
 * So, if we change device:
 *   - Set bank in mux_control_reg to data->csn
 *   - Update read/write timings (to do)
 */
static void stm_flex_select_chip(
	struct mtd_info * const mtd,
	const int chipnr)
{
	struct nand_chip * const nand = mtd->priv;
	struct stm_nand_flex_device * const data = nand->priv;

#if DEBUG_FLEX
	printf("\t\t\t\t---- SELECT = %2d ----\n", chipnr);
#endif

	/* Deselect, do nothing */
	if (chipnr == -1) {
		return;

	} else if (chipnr == 0) {
		/* If same chip as last time, no need to change anything */
		if (data->csn == flex.current_csn)
			return;

		/* Set correct EMI Chip Select (CSn) on FLEX controller */
		flex.current_csn = data->csn;
		*ST40_EMI_NAND_FLEX_MUXCTRL = 1ul << data->csn;

	} else {
		printf("ERROR: In %s() attempted to select chipnr = %d\n",
			__FUNCTION__,
			chipnr);
	}
}


static void stm_flex_hwcontrol (
	struct mtd_info * const mtd,
	int control)
{
	switch(control) {

	case NAND_CTL_SETCLE:
#if DEBUG_FLEX
		printf("\t\t\t\t\t\t----START COMMAND----\n");
		if (flex.mode != flex_quiecent) BUG();
#endif
		flex.mode = flex_command;
		break;

#if DEBUG_FLEX
	case NAND_CTL_CLRCLE:
		printf("\t\t\t\t\t\t---- end  command----\n");
		if (flex.mode != flex_command) BUG();
		flex.mode = flex_quiecent;
		break;
#endif

	case NAND_CTL_SETALE:
#if DEBUG_FLEX
		printf("\t\t\t\t\t\t----START ADDRESS----\n");
		if (flex.mode != flex_quiecent) BUG();
#endif
		flex.mode = flex_address;
		break;

#if DEBUG_FLEX
	case NAND_CTL_CLRALE:
		printf("\t\t\t\t\t\t---- end  address----\n");
		if (flex.mode != flex_address) BUG();
		flex.mode = flex_quiecent;
		break;
#endif

#if DEBUG_FLEX
	default:
		printf("ERROR: Unexpected parameter (control=0x%x) in %s()\n",
			control,
			__FUNCTION__);
		BUG();
#endif
	}
}


/**
 * nand_read_byte - [DEFAULT] read one byte from the chip
 * @mtd:	MTD device structure
 */
static u_char stm_flex_read_byte(
	struct mtd_info * const mtd)
{
	u_char byte;
	u_int32_t reg;

	/* read 1-byte at a time */
	reg = FLEX_BEAT_COUNT_1 | FLEX_1_BYTE_PER_BEAT;
	reg |= FLEX_CSn_STATUS;		/* deassert CSn after each flex data read */
#if 0
	reg |= FLEX_WAIT_RBn;		/* QQQ: do we want this ??? */
#endif
	*ST40_EMI_NAND_FLEX_DATA_RD_CFG = reg;

	/* read it */
	byte = (u_char)*ST40_EMI_NAND_FLEX_DATA;

#if DEBUG_FLEX
	printf("\t\t\t\t\t\t\t\t\t READ = 0x%02x\n", byte);
#endif

	/* return it */
	return byte;
}


/**
 * nand_write_byte - [DEFAULT] write one byte to the chip
 * @mtd:	MTD device structure
 * @byte:	pointer to data byte to write
 */
static void stm_flex_write_byte(
	struct mtd_info * const mtd,
	u_char byte)
{
	u_int32_t reg;

#if DEBUG_FLEX
	printf("\t\t\t\t\t\t\t\t\tWRITE = 0x%02x\t%s\n", byte,
		(flex.mode==flex_command) ? "command" :
		((flex.mode==flex_address) ? "address" : "*UNKNOWN*"));
#endif

	switch (flex.mode)
	{
		case flex_command:
			reg = byte | FLEX_BEAT_COUNT_1;
			reg |= FLEX_CSn_STATUS;	/* deassert CSn after each flex command write */
#if 0
			reg |= FLEX_WAIT_RBn;		/* QQQ: do we want this ??? */
#endif
			*ST40_EMI_NAND_FLEX_CMD = reg;
			break;

		case flex_address:
			reg = byte | FLEX_BEAT_COUNT_1;
			reg |= FLEX_CSn_STATUS;	/* deassert CSn after each flex address write */
#if 0
			reg |= FLEX_WAIT_RBn;		/* QQQ: do we want this ??? */
#endif
			*ST40_EMI_NAND_FLEX_ADD_REG = reg;
#if 0			/* QQQ: do we need this - I think not! */
			while (!nand_device_ready()) ;	/* wait till NAND is ready */
#endif
			break;

		default:
			BUG();
	}
}


/**
 * nand_read_buf - [DEFAULT] read chip data into buffer
 * @mtd:	MTD device structure
 * @buf:	buffer to store data
 * @len:	number of bytes to read
 */
static void stm_flex_read_buf(
	struct mtd_info * const mtd,
	u_char * const buf,
	const int len)
{
	int i;
	uint32_t *p;
	u_int32_t reg;

	/* our buffer needs to be 4-byte aligned, for the FLEX controller */
	p = ((uint32_t)buf & 0x3) ? (void*)flex.buf : (void*)buf;

#if DEBUG_FLEX
	printf("info: stm_flex_read_buf( buf=%p, len=0x%x )\t\tp=%p%s\n",
		buf, len, p,
		((uint32_t)buf & 0x3) ? "\t\t**** UN-ALIGNED *****" : "");
#endif

	/* configure to read 4-bytes at a time */
	reg = FLEX_BEAT_COUNT_4 | FLEX_1_BYTE_PER_BEAT;
	reg |= FLEX_CSn_STATUS;		/* deassert CSn after each flex data read */
#if 0
	reg |= FLEX_WAIT_RBn;		/* QQQ: do we want this ??? */
#endif
	*ST40_EMI_NAND_FLEX_DATA_RD_CFG = reg;

#if defined(CONFIG_SH_NAND_USES_CACHED_READS)
	/*
	 * Note, we only use the optimized cached TLB mapping,
	 * if the amount of data to be copied is an exact
	 * multiple of the length of a cache line.
	 */
	if ((len % DCACHE_LINESZ) == 0)	/* whole multiples of cache lines ? */
	{
		*mmucr_p |= MMUCR_AT;	/* enable Address Translation */
		asm volatile ("nop");	/* wait a few cycles after enabling AT */
		asm volatile ("nop");
		asm volatile ("nop");
		asm volatile ("nop");

#if 0	/* QQQ - DELETE */
		/* copy the data (from NAND), one CACHE-LINE at a time ... */
		if (((u32)p) % DCACHE_LINESZ == 0ul)	/* cache aligned ? */
		{
			register u32 src32;
			register u32 dst32 = (u32)p;
			register double temp0 asm("dr0");
			register double temp1 asm("dr2");
			register double temp2 asm("dr4");
			register double temp3 asm("dr6");

#define PUT_FPSCR(F)	asm volatile ("lds %0, fpscr"	: : "r"(F))
#define OCBP(LINE)	asm volatile ("ocbp @%0"	: : "r"(LINE))
#define PREF(LINE)	asm volatile ("pref @%0"	: : "r"(LINE))
#define ALLOC(LINE)	asm volatile ("movca.l r0, @%0"	: : "r"(LINE))

			OCBP(cache);
			PREF(cache);
			PUT_FPSCR(1<<20);	/* FPSCR.SZ=1 */

			for(i=0; i<len; )
			{
				src32 = (u32)cache;
				asm volatile ("fmov @%0+,%1"	: "+r"(src32),"=d"(temp0));
				asm volatile ("fmov @%0+,%1"	: "+r"(src32),"=d"(temp1));
				asm volatile ("fmov @%0+,%1"	: "+r"(src32),"=d"(temp2));
				asm volatile ("fmov @%0+,%1"	: "+r"(src32),"=d"(temp3));

				OCBP(cache);

				i += DCACHE_LINESZ;

				if (i<len)
				{
//QQQ					PREF(cache);
				}

				ALLOC(dst32);
				asm volatile ("fmov %1,@%0"	: : "r"(dst32),"d"(temp0) : "memory" );
				dst32+=8;
				asm volatile ("fmov %1,@%0"	: : "r"(dst32),"d"(temp1) : "memory" );
				dst32+=8;
				asm volatile ("fmov %1,@%0"	: : "r"(dst32),"d"(temp2) : "memory" );
				dst32+=8;
				asm volatile ("fmov %1,@%0"	: : "r"(dst32),"d"(temp3) : "memory" );
				dst32+=8;
			}
		}
		else	/* destination is *not* cache-aligned */
#endif	/* QQQ - DELETE */
		{
			for(i=0; i<len/4; i+=DCACHE_LINESZ/4)
			{
				asm volatile ("ocbp @%0" : : "r"(cache));
				p[i+0] = cache[0];
				p[i+1] = cache[1];
				p[i+2] = cache[2];
				p[i+3] = cache[3];
				p[i+4] = cache[4];
				p[i+5] = cache[5];
				p[i+6] = cache[6];
				p[i+7] = cache[7];
			}
		}
#if 0					/* QQQ - DELETE */
		if (++done_init < 8)
		{
			int j;
			printf("READ BUF\tlen=%u,\tpass=%u\n",len, done_init);
			for (i=0; i<len; i+=16)
			{
				for (j=0; j<16; j++)
					printf("%02x ", buf[i+j]);
				printf("\n");
			}
			printf("\n");
		}
#endif					/* QQQ - DELETE */
		/* finally, disable Address Translation */
		*mmucr_p &= ~MMUCR_AT;
	}
	else
	{
		/* let the user know we are *not* using the TLB */
		printf("warning: Not using cached copy in %s() for len=%u\n",
			__FUNCTION__,
			len);
		/* copy the data (from NAND) as 4-byte words ... */
		for(i=0; i<len/4; i++)
		{
			p[i] = *ST40_EMI_NAND_FLEX_DATA;
		}
	}
#else	/* CONFIG_SH_NAND_USES_CACHED_READS */
	/* copy the data (from NAND) as 4-byte words ... */
	for(i=0; i<len/4; i++)
	{
		p[i] = *ST40_EMI_NAND_FLEX_DATA;
	}
#endif	/* CONFIG_SH_NAND_USES_CACHED_READS */

	/* copy back into user-supplied buffer, if it was unaligned */
	if ((void*)p != (void*)buf)
		memcpy(buf, p, len);

#if DEBUG_FLEX
	printf("READ BUF\t\t\t\t");
	for (i=0; i<16; i++)
		printf("%02x ", buf[i]);
	printf("...\n");
#endif
}


/**
 * nand_write_buf - [DEFAULT] write buffer to chip
 * @mtd:	MTD device structure
 * @buf:	data buffer
 * @len:	number of bytes to write
 */
static void stm_flex_write_buf(
	struct mtd_info * const mtd,
	const u_char * const buf,
	const int len)
{
	int i;
	uint32_t *p;

#if DEBUG_FLEX
	printf("WRITE BUF (%u)\t\t", len);
	for (i=0; i<16; i++)
		printf("%02x ", buf[i]);
	printf("...\n");
#endif

	/* we want only to write multiples of 4-bytes at a time! */
	if (len % 4 != 0)
		printf("ERROR: in %s(), length (%u) not a multiple of 4!\n", __FUNCTION__, len);

	/* our buffer needs to be 4-byte aligned, for the FLEX controller */
	p = ((uint32_t)buf & 0x3) ? (void*)flex.buf : (void*)buf;

	/* copy from user-supplied buffer, if it is unaligned */
	if ((void*)p != (void*)buf)
		memcpy(p, buf, len);

	/* configured to write 4-bytes at a time */
	/* copy the data (to NAND) as 32-bit words ... */
	for(i=0; i<len/4; i++)
	{
		*ST40_EMI_NAND_FLEX_DATA = p[i];
	}
}


extern void stm_flex_init_nand(
	struct mtd_info * const mtd,
	struct nand_chip * const nand)
{
	struct stm_nand_flex_device * data = nand->priv;
	int csn;
#ifdef CFG_NAND_FLEX_CSn_MAP
	const int csn_map[CFG_MAX_NAND_DEVICE] = CFG_NAND_FLEX_CSn_MAP;
#endif	/* CFG_NAND_FLEX_CSn_MAP */

	BUG_ON(data!=NULL);		/* just in case ... */
	BUG_ON(flex.next_csn!=0);	/* just in case ... */

#if defined(CONFIG_SH_NAND_USES_CACHED_READS)
	volatile u32 * const pteh_p  = (u32*)ST40_MMU_PTEH;
	volatile u32 * const ptel_p  = (u32*)ST40_MMU_PTEL;
	const u32 pteh =
		((u32)cache & PAGE_MASK)			|
		(PTEH_ASID & ASID_MASK);
	const u32 ptel =
		((u32)ST40_EMI_NAND_FLEX_DATA & PAGE_MASK)	|
		PTEL_V						|
#if 0
		PTEL_PR_WRITE | PTEL_D	/* if we use OCBI */	|
#endif
		PTEL_SZ_1K					|
		PTEL_C						|
		PTEL_SH;
#endif	/* CONFIG_SH_NAND_USES_CACHED_READS */

	/* initialize the FLEX mode controller H/W */
	init_flex_mode();
	/* initialize the "flex" software structure */
	flex.mode          = flex_quiecent;	/* nothing pending */
	flex.current_csn   = -1;		/* no NAND device selected */
						/* allocate a bounce buffer */
	flex.buf = malloc(NAND_MAX_PAGESIZE + NAND_MAX_OOBSIZE);
	if (flex.buf==NULL)
	{
		printf("ERROR: Unable to allocate memory for a bounce buffer\n");
		BUG();
	}
	/* initialize the TLB mapping if configured */
#if defined(CONFIG_SH_NAND_USES_CACHED_READS)
	*mmucr_p |= MMUCR_TI;	/* invalidate the TLBs */
	*pteh_p = pteh;
	*ptel_p = ptel;
	asm volatile ("ldtlb");	/* define 1 TLB mapping */
#endif	/* CONFIG_SH_NAND_USES_CACHED_READS */

	csn = flex.next_csn++;		/* first free CSn */
	nand->priv = data = &(flex.device[csn]);	/* first free "private" structure */
	if (csn >= CFG_MAX_NAND_DEVICE) BUG();
#ifdef CFG_NAND_FLEX_CSn_MAP
	csn = csn_map[csn];				/* Re-map to different CSn if needed */
#endif	/* CFG_NAND_FLEX_CSn_MAP */
#if DEBUG_FLEX
	printf("info: stm_nand_flex_device.csn = %u\n", csn);
#endif
	data->csn         = csn;			/* fill in the private structure ... */
	data->mtd         = mtd;
	data->chip        = nand;
	data->timing_data = NULL;			/* QQQ: to do */

	/* Set up timing parameters */
#if 0
	/* The default times will work for 200MHz (or slower) */
	flex_set_timings(data->timing_data); /* QQQ: to do */
#endif

	/*
	 * Finally, over-write the function pointers in the 'nand_chip'
	 * structure, to use our FLEX H/W-specific ones instead.
	 */
	nand->select_chip = stm_flex_select_chip;
	nand->dev_ready   = stm_flex_device_ready;
	nand->hwcontrol   = stm_flex_hwcontrol;
	nand->read_byte   = stm_flex_read_byte;
	nand->write_byte  = stm_flex_write_byte;
	nand->read_buf    = stm_flex_read_buf;
	nand->write_buf   = stm_flex_write_buf;
}


#endif	/* CONFIG_CMD_NAND && CFG_NAND_FLEX_MODE */
